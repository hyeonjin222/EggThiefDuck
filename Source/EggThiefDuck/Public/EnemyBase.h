// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnemyBase.generated.h"

class UBoxComponent;
class UWidgetComponent;

UCLASS()
class EGGTHIEFDUCK_API AEnemyBase : public APawn
{
	GENERATED_BODY()

public:
	AEnemyBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/** 외부에서 물리적 충격을 줄 때 사용 */
	void ApplyKnockback(FVector ImpactImpulse);

	/** 데미지 처리 함수 (언리얼 표준 API) */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	/** 박스 형태의 물리 루트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Collision")
	TObjectPtr<UBoxComponent> BoxComp;

	/** 비주얼을 위한 스태틱 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	TObjectPtr<UStaticMeshComponent> EnemyMesh;

	/** 체력바 UI 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|UI")
	TObjectPtr<UWidgetComponent> HealthBarWidget;

	/** --- 물리 기반 Hopping 로직 --- */
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement")
	float JumpImpulse = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement")
	float ForwardImpulse = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement")
	float HopInterval = 0.5f;

	float HopTimer = 0.0f;

	bool IsGrounded();
	void PhysicalHop();

	/** 사망 처리 */
	void Die();

	/** 스탯 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Stats")
	float CurrentHealth;
};
