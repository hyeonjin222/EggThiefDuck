// Fill out your copyright notice in the Description page of Project Settings.

#include "EggProjectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

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

	// 2. 반응 설정: 바닥(Static)과 적(Pawn)은 막고, 달걀끼리(WorldDynamic)는 무시
	BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	BoxComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore); // 달걀끼리 충돌 방지
	BoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	BoxComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);

	// Hit 이벤트만 사용 (관통 롤백)
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

void AEggProjectile::SetSpeed(float InSpeed)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = InSpeed;
		ProjectileMovement->MaxSpeed = InSpeed;
	}
}

void AEggProjectile::AddTrailVFX(UNiagaraSystem* VFX)
{
	if (VFX)
	{
		TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(VFX, RootComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
	}
}

void AEggProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 이미 무언가에 부딪혔거나, 자기 자신 또는 다른 달걀 발사체와 부딪혔다면 무시
	if (bHit || !OtherActor || OtherActor == this || OtherActor->IsA<AEggProjectile>()) return;

	// 소유자(오리)와 부딪힌 경우도 무시
	if (OtherActor == GetOwner()) return;

	bHit = true;

	AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
	if (Enemy)
	{
		// 넉백 적용
		FVector ImpactImpulse = ProjectileMovement->Velocity * (0.4f + KnockbackBonus);
		ImpactImpulse.Z = 200.0f;
		Enemy->ApplyKnockback(ImpactImpulse);

		// 데미지 적용
		UGameplayStatics::ApplyDamage(Enemy, Damage, nullptr, this, UDamageType::StaticClass());
	}

	// 폭발 효과
	if (bIsExplosive)
	{
		float FinalRadius = 200.0f * (1.0f + ExplosionRadiusBonus);
		UGameplayStatics::ApplyRadialDamage(this, Damage * 0.5f, GetActorLocation(), FinalRadius, UDamageType::StaticClass(), TArray<AActor*>(), this);
	}

	// --- 시각적 소멸 및 궤적 유지 처리 ---
	if (ProjectileMesh) ProjectileMesh->SetVisibility(false);
	if (BoxComp) BoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 블루프린트에서 추가한 조명 끄기
	TArray<UPointLightComponent*> PointLights;
	GetComponents<UPointLightComponent>(PointLights);
	for (UPointLightComponent* Light : PointLights)
	{
		Light->SetVisibility(false);
	}
	
	// 이동 중지
	if (ProjectileMovement) ProjectileMovement->StopMovementImmediately();

	// 트레일 입자 방출 중단 (기존 입자는 수명만큼 남음)
	if (TrailComponent) TrailComponent->Deactivate();

	// 0.55초 후 액터 파괴 (VFX가 사라질 시간 확보)
	SetLifeSpan(0.55f);
}
