// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnemyBase.generated.h"

class UBoxComponent;
class UWidgetComponent;
class UStaticMeshComponent;

/** --- 아이템 드롭 시스템 구조체 --- */
USTRUCT(BlueprintType)
struct FItemDropRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ADropItemBase> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DropCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0, UIMax = 1))
	float DropChance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	float MinLaunchStrength = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	float MaxLaunchStrength = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	float MinUpwardForce = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	float MaxUpwardForce = 600.0f;
};

/** 적 상태 정의 */
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Chasing,
	Fleeing
};

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

	/** 공격 감지 충돌 이벤트 */
	UFUNCTION()
	void OnAttackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 데미지 처리 함수 */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	/** 상태 전환 */
	UFUNCTION(BlueprintCallable, Category = "Enemy|AI")
	void SetState(EEnemyState NewState);

protected:
	/** 현재 상태 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
	EEnemyState CurrentState = EEnemyState::Chasing;

	bool bIsFleeingPaused = false;
	FTimerHandle FleeDelayTimerHandle;

	void ResumeFleeing();

	/** 물리 루트 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Collision")
	TObjectPtr<UBoxComponent> BoxComp;

	/** 공격 범위 감지 박스 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Collision")
	TObjectPtr<UBoxComponent> AttackBox;

	/** 비주얼 메시 (Static) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	TObjectPtr<UStaticMeshComponent> EnemyMesh;

	/** 비주얼 메시 (Skeletal) - 스켈레탈 메시 에셋을 쓸 경우 여기에 할당 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	TObjectPtr<class USkeletalMeshComponent> EnemySkeletalMesh;

	/** 현재 활성화된 메시 (Squash & Stretch 적용 대상) */
	UPROPERTY()
	TObjectPtr<USceneComponent> ActiveMeshPtr;

	/** 머리 위 체력바 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|UI")
	TObjectPtr<UWidgetComponent> HealthBarWidget;

	/** 무브먼트 수치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement")
	float JumpImpulse = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement")
	float ForwardImpulse = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement")
	float HopInterval = 0.5f;

	float HopTimer = 0.0f;
	FVector BaseMeshScale;

	bool IsGrounded();
	void PhysicalHop();

	/** 사망 및 드롭 */
	void Die();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Drop")
	TArray<FItemDropRecord> DropTable;

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|UI")
	void SpawnDamageText(int32 DamageAmount);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|UI")
	TSubclassOf<class ADamageTextActor> DamageTextClass;

	/** --- 전투 스탯 (블루프린트에서 수정 가능) --- */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float AttackDamage = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Stats")
	float CurrentHealth;
};
