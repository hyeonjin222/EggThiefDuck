// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
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
	
	// 회전 고정 및 저항 설정
	BoxComp->BodyInstance.bLockXRotation = true;
	BoxComp->BodyInstance.bLockYRotation = true;
	
	// 선형 감쇄(Linear Damping)를 조금 높여 미끄러짐 방지
	BoxComp->SetLinearDamping(1.5f);
	BoxComp->SetAngularDamping(1.0f);

	// 2. 메시 설정
	EnemyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyMesh"));
	EnemyMesh->SetupAttachment(RootComponent);
	EnemyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemyMesh->SetAbsolute(false, false, true);
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// --- 1. 속도 제한 (Velocity Clamping) ---
	if (BoxComp)
	{
		FVector CurrentVelocity = BoxComp->GetPhysicsLinearVelocity();
		float SpeedSq = CurrentVelocity.SizeSquared();
		float MaxSpeed = 2000.0f; // 최대 속도 제한 (상황에 맞게 조절 가능)

		if (SpeedSq > FMath::Square(MaxSpeed))
		{
			FVector ClampedVelocity = CurrentVelocity.GetSafeNormal() * MaxSpeed;
			BoxComp->SetPhysicsLinearVelocity(ClampedVelocity);
		}
	}

	// --- 2. 호핑 로직 ---
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
		// --- 3. 넉백 전 속도 상쇄 ---
		// 현재 속도의 50%를 제거하여 넉백 힘이 무식하게 중첩되는 것을 방지
		FVector CurrentVel = BoxComp->GetPhysicsLinearVelocity();
		BoxComp->SetPhysicsLinearVelocity(CurrentVel * 0.5f);

		BoxComp->AddImpulse(ImpactImpulse, NAME_None, true);
	}
}
