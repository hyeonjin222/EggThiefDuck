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
};
