// Fill out your copyright notice in the Description page of Project Settings.

#include "EggProjectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
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

	// 2. 반응 설정: 바닥(Static)과 적(Pawn)은 막고, 달걀끼리(WorldDynamic)는 통과
	BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	BoxComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore); // 달걀끼리 충돌 방지 (중요!)
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
		UNiagaraFunctionLibrary::SpawnSystemAttached(VFX, RootComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
	}
}

void AEggProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 이미 무언가에 부딪혔거나, 자기 자신 또는 다른 달걀 발사체와 부딪혔다면 무시
	if (bHit || !OtherActor || OtherActor == this || OtherActor->IsA<AEggProjectile>()) return;

	// 소유자(오리)와 부딪힌 경우도 무시
	if (OtherActor == GetOwner()) return;

	if (!HitActors.Contains(OtherActor))
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
		if (Enemy)
		{
			// 관통 중복 히트 방지를 위해 기록
			HitActors.Add(OtherActor);

			// [수정] 넉백 보너스 적용
			FVector ImpactImpulse = ProjectileMovement->Velocity * (0.4f + KnockbackBonus);
			ImpactImpulse.Z = 200.0f;
			Enemy->ApplyKnockback(ImpactImpulse);

			// 강화된 데미지 적용
			UGameplayStatics::ApplyDamage(Enemy, Damage, nullptr, this, UDamageType::StaticClass());

			// 관통이 아니면 히트 판정 (소멸 시작)
			if (!bIsPiercing)
			{
				bHit = true;
			}
		}
		else
		{
			// 벽이나 장애물에 부딪히면 무조건 소멸
			bHit = true;
		}

		// [수정] 폭발 범위 보너스 적용
		if (bIsExplosive && (bHit || Enemy))
		{
			float FinalRadius = 200.0f * (1.0f + ExplosionRadiusBonus);
			UGameplayStatics::ApplyRadialDamage(this, Damage * 0.5f, GetActorLocation(), FinalRadius, UDamageType::StaticClass(), TArray<AActor*>(), this);
			// TODO: 폭발 VFX/SFX 재생 로직 추가 가능
		}

		if (bHit)
		{
			// --- 충돌 후 즉시 파괴하지 않고 "가짜 파괴" 처리 (VFX 재생용) ---
			
			// 1. 메시 숨기기
			if (ProjectileMesh)
			{
				ProjectileMesh->SetVisibility(false);
			}

			// 2. 콜리전 비활성화
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
}
