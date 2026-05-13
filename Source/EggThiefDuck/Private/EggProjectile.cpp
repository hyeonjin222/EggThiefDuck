// Fill out your copyright notice in the Description page of Project Settings.

#include "EggProjectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnemyBase.h"

AEggProjectile::AEggProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// 1. 박스 충돌체 설정 (이름을 BoxComp로 통일)
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = BoxComp; // 박스를 루트로 설정
	BoxComp->InitBoxExtent(FVector(15.0f, 15.0f, 15.0f));

	BoxComp->SetCollisionProfileName(TEXT("Custom"));
	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxComp->SetCollisionObjectType(ECC_WorldDynamic);
	BoxComp->BodyInstance.bUseCCD = true;

	BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	BoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	BoxComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);

	BoxComp->SetNotifyRigidBodyCollision(true);
	BoxComp->OnComponentHit.AddDynamic(this, &AEggProjectile::OnHit);

	// 2. 메시 설정
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetAbsolute(false, false, true);

	// 3. 발사체 이동 설정
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = BoxComp;
	ProjectileMovement->InitialSpeed = 3500.f;
	ProjectileMovement->MaxSpeed = 3500.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.2f;
	ProjectileMovement->ProjectileGravityScale = 0.1f;

	InitialLifeSpan = 3.0f;
}

void AEggProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AEggProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEggProjectile::FireInDirection(const FVector& ShootDirection)
{
	ProjectileMovement->Velocity = ShootDirection * ProjectileMovement->InitialSpeed;
}

void AEggProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
		if (Enemy)
		{
			FVector ImpactImpulse = ProjectileMovement->Velocity * 0.4f;
			ImpactImpulse.Z = 200.0f;
			
			Enemy->ApplyKnockback(ImpactImpulse);
		}

		Destroy();
	}
}
