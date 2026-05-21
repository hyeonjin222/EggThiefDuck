// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DamageTextActor.h"
#include "DropItemBase.h"
#include "ExpItem.h"
#include "GoldItem.h"
#include "DuckCharacter.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. 물리 루트 설정
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = BoxComp;
	BoxComp->InitBoxExtent(FVector(40.0f, 40.0f, 50.0f));
	BoxComp->SetSimulatePhysics(true);
	BoxComp->SetNotifyRigidBodyCollision(true);
	BoxComp->SetCollisionProfileName(TEXT("Pawn"));
	BoxComp->BodyInstance.bUseCCD = true;
	BoxComp->BodyInstance.bLockXRotation = true;
	BoxComp->BodyInstance.bLockYRotation = true;
	BoxComp->SetLinearDamping(1.5f);
	BoxComp->SetAngularDamping(1.0f);

	// 2. 공격 감지 박스 설정
	AttackBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackBox"));
	AttackBox->SetupAttachment(RootComponent);
	AttackBox->InitBoxExtent(FVector(50.0f, 50.0f, 60.0f));
	AttackBox->SetCollisionProfileName(TEXT("Trigger"));
	
	// 이벤트 바인딩
	AttackBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnAttackOverlap);

	// 3. 메시 설정
	EnemyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyMesh"));
	EnemyMesh->SetupAttachment(RootComponent);
	EnemyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemyMesh->SetAbsolute(false, false, true);

	EnemySkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EnemySkeletalMesh"));
	EnemySkeletalMesh->SetupAttachment(RootComponent);
	EnemySkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemySkeletalMesh->SetAbsolute(false, false, true);

	// 4. UI 설정
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidget->SetDrawAtDesiredSize(true);

	// 5. 기본 드롭 세팅
	FItemDropRecord ExpDrop;
	ExpDrop.ItemClass = AExpItem::StaticClass();
	ExpDrop.DropChance = 0.8f;
	DropTable.Add(ExpDrop);

	FItemDropRecord GoldDrop;
	GoldDrop.ItemClass = AGoldItem::StaticClass();
	GoldDrop.DropChance = 0.3f;
	DropTable.Add(GoldDrop);
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;

	// 스태틱 메시가 있으면 사용, 없으면 스켈레탈 메시 사용 (둘 다 없으면 기본값)
	if (EnemyMesh && EnemyMesh->GetStaticMesh())
	{
		ActiveMeshPtr = EnemyMesh;
		BaseMeshScale = EnemyMesh->GetRelativeScale3D();
		if (EnemySkeletalMesh) EnemySkeletalMesh->SetHiddenInGame(true);
	}
	else if (EnemySkeletalMesh && EnemySkeletalMesh->GetSkeletalMeshAsset())
	{
		ActiveMeshPtr = EnemySkeletalMesh;
		BaseMeshScale = EnemySkeletalMesh->GetRelativeScale3D();
		if (EnemyMesh) EnemyMesh->SetHiddenInGame(true);
	}
	else
	{
		ActiveMeshPtr = nullptr;
		BaseMeshScale = FVector(1.0f);
	}
}

void AEnemyBase::OnAttackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(ADuckCharacter::StaticClass()))
	{
		UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
	}
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 지속 데미지 판정
	if (CurrentState == EEnemyState::Chasing && AttackBox)
	{
		TArray<AActor*> OverlappingActors;
		AttackBox->GetOverlappingActors(OverlappingActors, ADuckCharacter::StaticClass());
		for (AActor* Actor : OverlappingActors)
		{
			UGameplayStatics::ApplyDamage(Actor, AttackDamage, GetController(), this, UDamageType::StaticClass());
		}
	}

	// 무브먼트 및 호핑 로직
	if (BoxComp)
	{
		FVector Velocity = BoxComp->GetPhysicsLinearVelocity();
		if (Velocity.SizeSquared() > FMath::Square(2000.0f))
		{
			BoxComp->SetPhysicsLinearVelocity(Velocity.GetSafeNormal() * 2000.0f);
		}
	}

	if (!bIsFleeingPaused)
	{
		HopTimer += DeltaTime;
		if (IsGrounded() && HopTimer >= HopInterval)
		{
			PhysicalHop();
			HopTimer = 0.0f;
		}
	}

	// Squash & Stretch 연출 (ActiveMeshPtr가 유효할 때만 실행)
	if (ActiveMeshPtr)
	{
		float VelocityZ = GetVelocity().Z;
		if (FMath::Abs(VelocityZ) < 10.0f) VelocityZ = 0.0f;
		
		// 1. 과장된 스쿼시 앤 스트레치 계수 (기존 0.0005f -> 0.0015f로 3배 강화)
		// 점프 시 위로 길쭉해지고, 착지/낙하 시 납작해지는 폭을 키웁니다.
		float TargetStretch = 1.0f + (FMath::Clamp(VelocityZ, -1200.f, 1200.f) * 0.0015f);

		// 2. 바닥에 닿았을 때(착지 직후) 짧게 과장되게 찌그러지도록 추가 보정
		if (IsGrounded() && HopTimer < 0.1f)
		{
			// 점프 직전 혹은 착지 직후에 아주 납작(0.6배)해짐
			TargetStretch = 0.6f; 
		}

		FVector CurrentScale = ActiveMeshPtr->GetRelativeScale3D();
		float CurrentStretch = CurrentScale.Z / BaseMeshScale.Z;
		
		// 3. 복원 속도를 조절하여 젤리 같은 탄성 부여 (기존 15.0f -> 18.0f로 약간 더 빠르고 찰지게)
		float SmoothedStretch = FMath::FInterpTo(CurrentStretch, TargetStretch, DeltaTime, 18.0f);
		
		// 체적 유지를 위해 Z축이 늘어난 만큼 X, Y축을 얇게(또는 두껍게) 만듦
		ActiveMeshPtr->SetRelativeScale3D(FVector(BaseMeshScale.X / FMath::Sqrt(SmoothedStretch), BaseMeshScale.Y / FMath::Sqrt(SmoothedStretch), BaseMeshScale.Z * SmoothedStretch));
	}

	// 플레이어 추격 회전
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (Player)
	{
		FVector Dir = (CurrentState == EEnemyState::Chasing || bIsFleeingPaused) ? (Player->GetActorLocation() - GetActorLocation()) : (GetActorLocation() - Player->GetActorLocation());
		Dir.Z = 0.0f;
		if (!Dir.IsNearlyZero())
		{
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), Dir.Rotation(), DeltaTime, 10.0f));
		}
	}
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth -= ActualDamage;
	SpawnDamageText(FMath::FloorToInt(ActualDamage));
	if (CurrentHealth <= 0.0f) Die();
	return ActualDamage;
}

void AEnemyBase::Die()
{
	for (const FItemDropRecord& Record : DropTable)
	{
		if (Record.ItemClass && FMath::FRand() <= Record.DropChance)
		{
			for (int32 i = 0; i < Record.DropCount; i++)
			{
				FVector Loc = GetActorLocation() + FVector(FMath::RandRange(-20.f, 20.f), FMath::RandRange(-20.f, 20.f), 30.f);
				ADropItemBase* Item = GetWorld()->SpawnActor<ADropItemBase>(Record.ItemClass, Loc, FRotator::ZeroRotator);
				if (Item)
				{
					FVector Launch = (FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 0.f).GetSafeNormal() * FMath::RandRange(Record.MinLaunchStrength, Record.MaxLaunchStrength)) + (FVector::UpVector * FMath::RandRange(Record.MinUpwardForce, Record.MaxUpwardForce));
					Item->InitVelocity(Launch);
				}
			}
		}
	}
	Destroy();
}

bool AEnemyBase::IsGrounded()
{
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	return GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), GetActorLocation() - FVector(0, 0, 65.0f), ECC_Visibility, Params);
}

void AEnemyBase::PhysicalHop()
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player || !BoxComp) return;
	FVector Dir = (CurrentState == EEnemyState::Chasing) ? (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal() : (GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();
	Dir.Z = 0.0f;
	BoxComp->AddImpulse((FVector::UpVector * JumpImpulse) + (Dir * ForwardImpulse), NAME_None, true);
}

void AEnemyBase::SetState(EEnemyState NewState)
{
	if (CurrentState == NewState) return;
	CurrentState = NewState;
	if (CurrentState == EEnemyState::Fleeing)
	{
		if (HealthBarWidget) HealthBarWidget->SetVisibility(false);
		bIsFleeingPaused = true;
		GetWorldTimerManager().SetTimer(FleeDelayTimerHandle, this, &AEnemyBase::ResumeFleeing, 4.0f, false);
	}
}

void AEnemyBase::ResumeFleeing() { bIsFleeingPaused = false; }

void AEnemyBase::ApplyKnockback(FVector ImpactImpulse)
{
	if (BoxComp)
	{
		BoxComp->SetPhysicsLinearVelocity(BoxComp->GetPhysicsLinearVelocity() * 0.5f);
		BoxComp->AddImpulse(ImpactImpulse, NAME_None, true);
	}
}
