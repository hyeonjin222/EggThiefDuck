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
	
	BoxComp->BodyInstance.bLockXRotation = true;
	BoxComp->BodyInstance.bLockYRotation = true;
	BoxComp->SetLinearDamping(0.8f);
	BoxComp->SetAngularDamping(1.0f);

	// 2. 메시 설정
	EnemyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyMesh"));
	EnemyMesh->SetupAttachment(RootComponent);
	EnemyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// [핵심] 메시가 부모(Box)의 스케일을 무시하고 자기 자신의 스케일만 쓰도록 설정
	// SetAbsolute(Location, Rotation, Scale)
	EnemyMesh->SetAbsolute(false, false, true);
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HopTimer += DeltaTime;
	
	if (IsGrounded() && HopTimer >= HopInterval)
	{
		PhysicalHop();
		HopTimer = 0.0f;
	}

	float VelocityZ = GetVelocity().Z;
	float Stretch = 1.0f + (FMath::Clamp(VelocityZ, -1000.f, 1000.f) * 0.0005f);
	// Absolute Scale 상태이므로 여기서 직접 스케일을 조절해줍니다.
	EnemyMesh->SetRelativeScale3D(FVector(1.0f / FMath::Sqrt(Stretch), 1.0f / FMath::Sqrt(Stretch), Stretch));

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
		BoxComp->AddImpulse(ImpactImpulse, NAME_None, true);
	}
}
