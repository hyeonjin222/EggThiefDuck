// Fill out your copyright notice in the Description page of Project Settings.

#include "EggProjectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnemyBase.h"
#include "Kismet/GameplayStatics.h"

AEggProjectile::AEggProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = BoxComp;
	BoxComp->InitBoxExtent(FVector(15.0f, 15.0f, 15.0f));

	// 1. 물리 및 콜리전 설정
	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxComp->SetCollisionObjectType(ECC_WorldDynamic);
	BoxComp->BodyInstance.bUseCCD = true;
	BoxComp->SetNotifyRigidBodyCollision(true);

	// 2. 반응 설정: 바닥(Static/Dynamic)과 적은 막고, 아이템은 통과
	BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	BoxComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block); // 환경 바닥 대응
	BoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	BoxComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap); // 아이템 통과

	BoxComp->OnComponentHit.AddDynamic(this, &AEggProjectile::OnHit);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = BoxComp;
	ProjectileMovement->InitialSpeed = 3500.f;
	ProjectileMovement->MaxSpeed = 3500.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.3f;

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
	// 이미 무언가에 부딪혔다면 중복 처리 방지
	if (bHit) return;

	if ((OtherActor != nullptr) && (OtherActor != this))
	{
		bHit = true;

		AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
		if (Enemy)
		{
			FVector ImpactImpulse = ProjectileMovement->Velocity * 0.4f;
			ImpactImpulse.Z = 200.0f;
			Enemy->ApplyKnockback(ImpactImpulse);

			UGameplayStatics::ApplyDamage(Enemy, 20.0f, nullptr, this, UDamageType::StaticClass());
		}

		// --- 충돌 후 즉시 파괴하지 않고 "가짜 파괴" 처리 (VFX 재생용) ---
		
		// 1. 메시 숨기기
		if (ProjectileMesh)
		{
			ProjectileMesh->SetVisibility(false);
		}

		// 2. 콜리전 비활성화 (더 이상 다른 것에 부딪히지 않음)
		if (BoxComp)
		{
			BoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		// 3. 움직임 정지
		if (ProjectileMovement)
		{
			ProjectileMovement->StopMovementImmediately();
		}

		// 4. 수명 연장 (약 1초 뒤에 실제로 메모리에서 삭제)
		SetLifeSpan(1.0f);
	}
}
