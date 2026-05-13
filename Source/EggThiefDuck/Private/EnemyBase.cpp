// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

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

	// 호핑 로직
	HopTimer += DeltaTime;
	if (IsGrounded() && HopTimer >= HopInterval)
	{
		PhysicalHop();
		HopTimer = 0.0f;
	}

	// 시각 효과
	float VelocityZ = GetVelocity().Z;
	float Stretch = 1.0f + (FMath::Clamp(VelocityZ, -1000.f, 1000.f) * 0.0005f);
	EnemyMesh->SetRelativeScale3D(FVector(1.0f / FMath::Sqrt(Stretch), 1.0f / FMath::Sqrt(Stretch), Stretch));

	// 플레이어 조준
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		FVector Direction = PlayerPawn->GetActorLocation() - GetActorLocation();
		Direction.Z = 0.0f;
		if (!Direction.IsNearlyZero())
		{
			SetActorRotation(Direction.Rotation());
		}
	}
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	CurrentHealth -= ActualDamage;
	
	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
	
	return ActualDamage;
}

void AEnemyBase::Die()
{
	// TODO: 아이템 드롭 로직 추가 지점
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

	FVector ForwardDir = (PlayerPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	ForwardDir.Z = 0.0f;

	FVector Impulse = (FVector::UpVector * JumpImpulse) + (ForwardDir * ForwardImpulse);
	BoxComp->AddImpulse(Impulse, NAME_None, true);
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
