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

	/** 업그레이드 적용 */
	void ApplyUpgrade(class UUpgradeDataAsset* Upgrade);

private:
	/** 발사 시도 (애니메이션 및 타이머 시작) */
	void Fire();

	/** 실제 발사체 생성 (타이머 호출용) */
	void FireProjectile();

	/** 달걀 게이지 업데이트 */
	void UpdateEggGauge(float DeltaTime);

private:
	/** 발사 지연 시간 비율 (0.0 ~ 1.0)
	  * 공격 애니메이션 재생 시간 중 어느 시점에 실제 발사체가 생성될지 결정합니다. 
	  * 예: 0.5면 애니메이션의 50% 지점, 0.3이면 30% 지점. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float FireDelayRatio = 0.5f;

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

	/** 업그레이드 메커니즘 상태 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Mech", meta = (AllowPrivateAccess = "true"))
	int32 MultiShotCount = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Mech", meta = (AllowPrivateAccess = "true"))
	bool bPiercingEnabled = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Mech", meta = (AllowPrivateAccess = "true"))
	bool bExplosiveEnabled = false;

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
