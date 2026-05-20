// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EggProjectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS()
class EGGTHIEFDUCK_API AEggProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AEggProjectile();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void FireInDirection(const FVector& ShootDirection);

	/** 데미지 설정 */
	void SetDamage(float InDamage) { Damage = InDamage; }

	/** 넉백 보너스 설정 */
	void SetKnockbackBonus(float InBonus) { KnockbackBonus = InBonus; }

	/** 폭발 범위 보너스 설정 */
	void SetExplosionRadiusBonus(float InBonus) { ExplosionRadiusBonus = InBonus; }

	/** 관통 여부 설정 */
	void SetPiercing(bool bInPiercing) { bIsPiercing = bInPiercing; }

	/** 폭발 여부 설정 */
	void SetExplosive(bool bInExplosive) { bIsExplosive = bInExplosive; }

	/** 속도 설정 */
	void SetSpeed(float InSpeed);

	/** VFX 추가 */
	void AddTrailVFX(class UNiagaraSystem* VFX);

	/** --- 컴포넌트 (BoxComp로 이름 통일 및 노출) --- */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Collision")
	TObjectPtr<UBoxComponent> BoxComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Visual")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Movement")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

private:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** 이미 무언가에 부딪혔는지 여부 (중복 처리 방지) */
	bool bHit = false;

	/** 이미 부딪힌 액터 목록 (관통 시 중복 데미지 방지) */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> HitActors;

	/** 데미지 수치 */
	float Damage = 20.f;

	/** 관통 상태 */
	bool bIsPiercing = false;

	/** 폭발 상태 */
	bool bIsExplosive = false;

	/** 보너스 스탯들 */
	float KnockbackBonus = 0.0f;
	float ExplosionRadiusBonus = 0.0f;
};
