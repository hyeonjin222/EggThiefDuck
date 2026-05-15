// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DuckCombatComponent.generated.h"

class AEggProjectile;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EGGTHIEFDUCK_API UDuckCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDuckCombatComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 사격 시작/중지 */
	void StartFire();
	void StopFire();

	/** 현재 사격 중인지 여부 반환 (실제 발사 루프 작동 여부) */
	bool IsFiring() const { return bIsFiring || bWantsToFire; }

	/** 달걀 게이지 즉시 회복 (아이템용) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RefillGauge(float Amount);

private:
	/** 실제 발사 로직 */
	void Fire();

	/** 달걀 게이지 업데이트 */
	void UpdateEggGauge(float DeltaTime);

private:
	/** 발사체 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AEggProjectile> ProjectileClass;

	/** 사격 주기 (연사력) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float FireRate = 0.2f;

	/** 달걀 게이지 관련 변수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Gauge", meta = (AllowPrivateAccess = "true"))
	float MaxEggGauge = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Gauge", meta = (AllowPrivateAccess = "true"))
	float CurrentEggGauge;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Gauge", meta = (AllowPrivateAccess = "true"))
	float GaugeCostPerShot = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Gauge", meta = (AllowPrivateAccess = "true"))
	float GaugeRecoveryRate = 15.f;

	/** 과열 상태 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Status", meta = (AllowPrivateAccess = "true"))
	bool bIsOverheated = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Status", meta = (AllowPrivateAccess = "true"))
	float OverheatPenaltyTime = 2.0f;

	/** 사격 제어 변수 */
	bool bIsFiring = false;    // 실제 연사 루프 중인지
	bool bWantsToFire = false; // 1발 발사 보장용 플래그

	/** 마지막 사격 시간 */
	float LastFireTime = 0.f;

	/** 과열 종료를 위한 타이머 핸들 */
	FTimerHandle OverheatTimerHandle;

	void EndOverheat();
};
