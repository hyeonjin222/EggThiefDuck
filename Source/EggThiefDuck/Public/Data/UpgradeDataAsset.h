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
	// --- 순수 스탯 강화 (일반 레벨용) ---
	Stat_MaxHealth          UMETA(DisplayName = "Stat: Max Health"),
	Stat_MoveSpeed          UMETA(DisplayName = "Stat: Move Speed"),
	Stat_AttackDamage       UMETA(DisplayName = "Stat: Attack Damage"),
	Stat_FireRate           UMETA(DisplayName = "Stat: Fire Rate"),
	Stat_ProjectileSpeed    UMETA(DisplayName = "Stat: Projectile Speed"),
	Stat_SpreadAngle        UMETA(DisplayName = "Stat: Spread Angle"),
	Stat_Knockback          UMETA(DisplayName = "Stat: Knockback Power"),
	Stat_Range              UMETA(DisplayName = "Stat: Attack Range"),
	Stat_ProjectileSize     UMETA(DisplayName = "Stat: Projectile Size"),
	Stat_ExplosionRadius    UMETA(DisplayName = "Stat: Explosion Radius"),
	Stat_GaugeMax           UMETA(DisplayName = "Stat: Max Egg Gauge"),
	Stat_GaugeRecovery      UMETA(DisplayName = "Stat: Gauge Recovery"),
	Stat_PiercingCount      UMETA(DisplayName = "Stat: Piercing Count"),

	// --- 주무기 변이 (Option A 시너지 - 5레벨 무기 선택용) ---
	Weapon_Mod_MachineGun   UMETA(DisplayName = "Weapon Mod: Machine Gun"),
	Weapon_Mod_Shotgun      UMETA(DisplayName = "Weapon Mod: Shotgun"),
	Weapon_Mod_Piercing     UMETA(DisplayName = "Weapon Mod: Piercing"),
	Weapon_Mod_Explosive    UMETA(DisplayName = "Weapon Mod: Explosive"),
	Weapon_Mod_Sniper       UMETA(DisplayName = "Weapon Mod: Sniper Shot"),
	Weapon_Mod_Flamethrower UMETA(DisplayName = "Weapon Mod: Flamethrower"),

	// --- 패시브 무기 (독립 작동 - 5레벨 무기 선택용) ---
	Weapon_Passive_Orbit    UMETA(DisplayName = "Weapon Passive: Orbiting"),
	Weapon_Passive_AutoBomb UMETA(DisplayName = "Weapon Passive: Auto Bomb"),
	Weapon_Passive_Molotov  UMETA(DisplayName = "Weapon Passive: Molotov")
};

/**
 * 개별 업그레이드 효과를 정의하는 구조체
 */
USTRUCT(BlueprintType)
struct FUpgradeEffect
{
	GENERATED_BODY()

	/** 적용할 효과 종류 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EUpgradeType Type = EUpgradeType::Stat_AttackDamage;

	/** 적용할 수치 (체력/게이지는 절대치, 공격력/연사력 등은 퍼센트 비율) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Value = 0.0f;
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

	/** [신규] 적용될 효과 목록 (여러 개 동시 적용 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Logic")
	TArray<FUpgradeEffect> Effects;

	/** 최대 강화 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Logic")
	int32 MaxLevel = 5;

	/** 등장 확률 가중치 (높을수록 잘 나옴) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Logic")
	float Weight = 1.0f;

	/** 선행 요구 업그레이드 ID (비어있으면 무관) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Logic")
	FName RequiredUpgradeID;

	/** 선행 요구 업그레이드의 최소 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Logic")
	int32 RequiredLevel = 1;

	/** 새로운 무기 획득 여부 (5레벨 마다 확정 등장 후보군) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Logic")
	bool bIsWeapon = false;

	/** 업그레이드 획득 시 재생할 나이아가라 이펙트 (일회성) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Effects")
	TObjectPtr<UNiagaraSystem> UpgradeVFX;

	/** 플레이어에게 계속 붙어있을 나이아가라 이펙트 (오라, 버프 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Effects")
	TObjectPtr<UNiagaraSystem> PlayerPersistentVFX;

	/** 이 업그레이드 이후 발사되는 달걀에 붙을 나이아가라 이펙트 (잔상 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Effects")
	TObjectPtr<UNiagaraSystem> ProjectileTrailVFX;

	/** 업그레이드 획득 시 재생할 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Effects")
	TObjectPtr<USoundBase> UpgradeSound;

	/** 특수 기술(지뢰, 회전 달걀 등)을 위한 액터 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special Logic")
	TSubclassOf<AActor> SpecialActorClass;
};
