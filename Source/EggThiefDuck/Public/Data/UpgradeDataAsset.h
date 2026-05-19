// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UpgradeDataAsset.generated.h"

class UNiagaraSystem;

/**
 * 업그레이드 타입을 정의하는 열거형
 */
UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	Stat_MaxHealth          UMETA(DisplayName = "Stat: Max Health"),
	Stat_MoveSpeed          UMETA(DisplayName = "Stat: Move Speed"),
	Stat_AttackDamage       UMETA(DisplayName = "Stat: Attack Damage"),
	Stat_FireRate           UMETA(DisplayName = "Stat: Fire Rate"),
	Stat_GaugeMax           UMETA(DisplayName = "Stat: Max Egg Gauge"),
	Stat_GaugeRecovery      UMETA(DisplayName = "Stat: Gauge Recovery"),
	Mech_MultiShot          UMETA(DisplayName = "Mech: Multi Shot"),
	Mech_Piercing           UMETA(DisplayName = "Mech: Piercing"),
	Mech_Explosive          UMETA(DisplayName = "Mech: Explosive"),
	Special_OrbitingEgg     UMETA(DisplayName = "Special: Orbiting Egg"),
	Special_EggMine         UMETA(DisplayName = "Special: Egg Mine")
};

/**
 * 뱀파이어 서바이벌 스타일의 업그레이드 데이터를 정의하는 데이터 에셋 클래스
 */
UCLASS(BlueprintType)
class EGGTHIEFDUCK_API UUpgradeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 내부 관리를 위한 고유 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Info")
	FName UpgradeID;

	/** UI에 표시될 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Info")
	FText UpgradeName;

	/** UI에 표시될 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Info", meta = (MultiLine = true))
	FText UpgradeDescription;

	/** UI 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Info")
	TObjectPtr<UTexture2D> UpgradeIcon;

	/** 업그레이드 종류 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Logic")
	EUpgradeType UpgradeType;

	/** 강화 수치 (스탯 강화 시 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Logic")
	float Value;

	/** 최대 강화 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Logic")
	int32 MaxLevel = 5;

	/** 업그레이드 획득 시 재생할 나이아가라 이펙트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Effects")
	TObjectPtr<UNiagaraSystem> UpgradeVFX;

	/** 업그레이드 획득 시 재생할 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Effects")
	TObjectPtr<USoundBase> UpgradeSound;

	/** 특수 기술(지뢰, 회전 달걀 등)을 위한 액터 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special Logic")
	TSubclassOf<AActor> SpecialActorClass;
};
