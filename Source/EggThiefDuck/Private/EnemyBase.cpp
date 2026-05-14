// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DamageTextActor.h"
#include "DropItemBase.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. 박스 컴포넌트 설정 (루트)
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = BoxComp;
	BoxComp->InitBoxExtent(FVector(40.0f, 40.0f, 50.0f));
	
	BoxComp->SetSimulatePhysics(true);
	BoxComp->SetNotifyRigidBodyCollision(true);
	BoxComp->SetCollisionProfileName(TEXT("Pawn"));
	
	BoxComp->BodyInstance.bUseCCD = true;
	
	BoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	BoxComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	BoxComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	
	BoxComp->BodyInstance.bLockXRotation = true;
	BoxComp->BodyInstance.bLockYRotation = true;
	
	BoxComp->SetLinearDamping(1.5f);
	BoxComp->SetAngularDamping(1.0f);

	// 2. 메시 설정
	EnemyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyMesh"));
	EnemyMesh->SetupAttachment(RootComponent);
	EnemyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemyMesh->SetAbsolute(false, false, true);

	// 3. 체력바 UI 설정
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // 머리 위
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen); // 항상 카메라 정면
	HealthBarWidget->SetDrawAtDesiredSize(true);
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;

	// 초기 스케일 저장
	if (EnemyMesh)
	{
		BaseMeshScale = EnemyMesh->GetRelativeScale3D();
	}
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 속도 제한
	if (BoxComp)
	{
		FVector CurrentVelocity = BoxComp->GetPhysicsLinearVelocity();
		float SpeedSq = CurrentVelocity.SizeSquared();
		float MaxSpeed = 2000.0f;

		if (SpeedSq > FMath::Square(MaxSpeed))
		{
			FVector ClampedVelocity = CurrentVelocity.GetSafeNormal() * MaxSpeed;
			BoxComp->SetPhysicsLinearVelocity(ClampedVelocity);
		}
	}

	// 호핑 로직 (도망 일시정지 중이면 뛰지 않음)
	if (!bIsFleeingPaused)
	{
		HopTimer += DeltaTime;
		if (IsGrounded() && HopTimer >= HopInterval)
		{
			PhysicalHop();
			HopTimer = 0.0f;
		}
	}

	// 시각 효과 (Squash & Stretch) - 부드러운 보간 적용
	float VelocityZ = GetVelocity().Z;
	
	// 아주 작은 속도는 무시 (정지 상태 떨림 방지)
	if (FMath::Abs(VelocityZ) < 10.0f) VelocityZ = 0.0f;

	float TargetStretch = 1.0f + (FMath::Clamp(VelocityZ, -1000.f, 1000.f) * 0.0005f);
	
	// 현재 스케일에서 목표 스케일로 부드럽게 이동 (Interp)
	FVector CurrentScale = EnemyMesh->GetRelativeScale3D();
	float CurrentStretch = CurrentScale.Z / BaseMeshScale.Z; // 현재의 늘어남 정도 역계산
	float SmoothedStretch = FMath::FInterpTo(CurrentStretch, TargetStretch, DeltaTime, 15.0f);

	// 기본 스케일에 늘어남 정도를 곱해줌
	EnemyMesh->SetRelativeScale3D(FVector(
		BaseMeshScale.X / FMath::Sqrt(SmoothedStretch), 
		BaseMeshScale.Y / FMath::Sqrt(SmoothedStretch), 
		BaseMeshScale.Z * SmoothedStretch
	));

	// --- 회전 조절 로직 (더 부드럽고 안정적으로) ---
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		FVector TargetDirection;
		
		if (CurrentState == EEnemyState::Chasing || bIsFleeingPaused)
		{
			// 추격 중이거나 도망 대기 중일 때는 플레이어를 바라봄
			TargetDirection = PlayerPawn->GetActorLocation() - GetActorLocation();
		}
		else
		{
			// 도망 중일 때는 '플레이어의 반대 방향'을 명확히 고정해서 바라봄 (물리 속도 대신 계산된 방향 사용)
			TargetDirection = GetActorLocation() - PlayerPawn->GetActorLocation();
		}

		TargetDirection.Z = 0.0f;
		if (!TargetDirection.IsNearlyZero())
		{
			FRotator TargetRotation = TargetDirection.Rotation();
			FRotator CurrentRotation = GetActorRotation();
			
			// RInterpTo를 사용해 부드럽게 회전 (지터 방지)
			FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 10.0f);
			SetActorRotation(NewRotation);
		}
	}
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	CurrentHealth -= ActualDamage;

	// 데미지 숫자 스폰 이벤트 호출
	SpawnDamageText(ActualDamage);
	
	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
	
	return ActualDamage;
}

void AEnemyBase::Die()
{
	// 드롭 테이블에 정의된 모든 아이템 처리
	for (const FItemDropRecord& DropRecord : DropTable)
	{
		if (DropRecord.ItemClass && FMath::FRand() <= DropRecord.DropChance)
		{
			for (int32 i = 0; i < DropRecord.DropCount; i++)
			{
				// 소환 위치에 약간의 랜덤 오프셋 추가 (겹침 방지)
				FVector RandomOffset = FVector(FMath::RandRange(-20.f, 20.f), FMath::RandRange(-20.f, 20.f), FMath::RandRange(0.f, 20.f));
				FVector SpawnLoc = GetActorLocation() + FVector(0, 0, 30.f) + RandomOffset;
				
				ADropItemBase* Item = GetWorld()->SpawnActor<ADropItemBase>(DropRecord.ItemClass, SpawnLoc, FRotator::ZeroRotator);
				if (Item)
				{
					// 수평 방향 랜덤 벡터
					FVector LaunchDir = FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 0.f).GetSafeNormal();
					float HorizontalStrength = FMath::RandRange(DropRecord.MinLaunchStrength, DropRecord.MaxLaunchStrength);
					
					// 수직 방향 힘 개별 계산
					float UpwardForce = FMath::RandRange(DropRecord.MinUpwardForce, DropRecord.MaxUpwardForce);
					
					FVector FinalImpulse = (LaunchDir * HorizontalStrength) + (FVector::UpVector * UpwardForce);
					Item->InitVelocity(FinalImpulse);
				}
			}
		}
	}

	Destroy();
}

bool AEnemyBase::IsGrounded()
{
	if (!BoxComp) return false;

	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 65.0f);
	
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
}

void AEnemyBase::PhysicalHop()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn || !BoxComp) return;

	FVector MoveDir;
	if (CurrentState == EEnemyState::Chasing)
	{
		// 플레이어 방향으로
		MoveDir = (PlayerPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	}
	else
	{
		// 플레이어 반대 방향으로 (도망)
		MoveDir = (GetActorLocation() - PlayerPawn->GetActorLocation()).GetSafeNormal();
	}
	
	MoveDir.Z = 0.0f;

	FVector Impulse = (FVector::UpVector * JumpImpulse) + (MoveDir * ForwardImpulse);
	BoxComp->AddImpulse(Impulse, NAME_None, true);
}

void AEnemyBase::SetState(EEnemyState NewState)
{
	if (CurrentState == NewState) return;
	CurrentState = NewState;
	
	if (CurrentState == EEnemyState::Fleeing)
	{
		// 1. 체력바 숨기기
		if (HealthBarWidget) HealthBarWidget->SetVisibility(false);

		// 2. 도망 전 4초간 정지 및 대기 (플레이어를 쳐다봄)
		bIsFleeingPaused = true;
		GetWorld()->GetTimerManager().SetTimer(FleeDelayTimerHandle, this, &AEnemyBase::ResumeFleeing, 4.0f, false);
	}
}

void AEnemyBase::ResumeFleeing()
{
	bIsFleeingPaused = false;
}

void AEnemyBase::ApplyKnockback(FVector ImpactImpulse)
{
	if (BoxComp)
	{
		FVector CurrentVel = BoxComp->GetPhysicsLinearVelocity();
		BoxComp->SetPhysicsLinearVelocity(CurrentVel * 0.5f);
		BoxComp->AddImpulse(ImpactImpulse, NAME_None, true);
	}
}
