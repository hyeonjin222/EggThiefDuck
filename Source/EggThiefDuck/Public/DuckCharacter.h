// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DuckCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UDuckCombatComponent;
class UWidgetComponent;
class UUpgradeDataAsset;
class UNiagaraSystem;
struct FInputActionValue;

UCLASS()
class EGGTHIEFDUCK_API ADuckCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ADuckCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** 데미지 처리 함수 (언리얼 표준 API) */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	/** 조준 정렬 확인 (사격 가능 여부) */
	bool IsAlignedWithCursor() const;

	/** 애니메이션 몽타주 재생 함수들 */
	void PlayHitReactMontage();
	void PlayDeathMontage();
	void PlayAttackMontage(float InPlayRate = 1.0f, float InBlendTime = -1.0f);

	/** 업그레이드 선택 (UI에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	void SelectUpgrade(UUpgradeDataAsset* SelectedUpgrade);

	/** 실제 업그레이드 적용 로직 (외부 컴포넌트에서도 호출 가능하도록 public) */
	void ApplyUpgrade(UUpgradeDataAsset* Upgrade);

	/** 스탯 보너스 직접 추가 함수들 (C++ 로직용) */
	void AddAttackDamageBonus(float Amount) { AttackDamageBonus += Amount; }
	void AddAttackSpeedBonus(float Amount);

protected:
	/** 레벨업 시 업그레이드 화면 표시 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Upgrades")
	void OnShowUpgradeScreen(const TArray<UUpgradeDataAsset*>& Options);

private:
	/** 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDuckCombatComponent> CombatComp;

	/** 머리 위 체력바 UI 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> HealthBarWidget;

	/** 입력 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> UIInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FireAction;

	/** 이동 로직 */
	void Move(const FInputActionValue& Value);

	/** 사격 로직 */
	void StartFire();
	void StopFire();

	/** 마우스 조준 로직 */
	void LookAtMouseCursor();

	/** 캐릭터 이동 속도 */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveSpeed = 600.f;

	/** 마지막 사격 시간 (조준 유지용) */
	float LastFireTime = 0.0f;

	/** 스탯 (체력) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth;

	/** 공격력 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float BaseDamage = 20.0f;

	/** 누적 공격력 보너스 (0.1 = 10% 증가) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float AttackDamageBonus = 0.0f;

	/** 성장 시스템 (XP & Level) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	int32 Level = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float CurrentExp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float ExpToNextLevel = 100.0f;

	/** 업그레이드 풀 (에디터에서 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrades", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UUpgradeDataAsset>> UpgradePool;

	/** 현재 적용된 업그레이드 단계 저장 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Upgrades", meta = (AllowPrivateAccess = "true"))
	TMap<FName, int32> AppliedUpgradeLevels;

	/** 플레이어에게 붙어있는 지속성 VFX 컴포넌트들 */
	UPROPERTY()
	TMap<FName, TObjectPtr<class UNiagaraComponent>> PersistentVFXComponents;

	/** 재화 (Gold) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 Gold = 0;

	/** 사망 처리 */
	void Die();

	/** 사망 애니메이션 종료 시 호출될 콜백 */
	void OnDeathAnimationFinished();

	FTimerHandle DeathTimerHandle;

	/** 레벨업 로직 */
	void LevelUp();

	/** 업그레이드 VFX 재생 */
	void PlayUpgradeVFX(UNiagaraSystem* VFX);

	/** 무적 해제 */
	void ResetInvincibility();

	/** UI 동기화 함수 */
	void RefreshHUD();

protected:
	/** 피격 연출 관련 (에디터 조절용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|HitReaction")
	float InvincibleDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|HitReaction")
	float KnockbackStrength = 500.0f;

	/** 카메라 흔들림 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|HitReaction")
	TSubclassOf<class UCameraShakeBase> HitCameraShakeClass;

	/** 무적 상태 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|HitReaction")
	bool bIsInvincible = false;

	FTimerHandle InvincibleTimerHandle;

	/** 사망 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	bool bIsDead = false;

	/** 피격 연출 이벤트 (BP에서 메시 깜빡임, 붉은 테두리 등 구현) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Stats|HitReaction")
	void OnPlayerHit();

	/** C++ 기반 피격 깜빡임 구현 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|HitReaction")
	TObjectPtr<UMaterialInterface> HitOverlayMaterial;

	void StartHitFlash();
	void ToggleFlash();

	FTimerHandle FlashTimerHandle;
	int32 FlashCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|HitReaction")
	int32 MaxFlashCount = 4; // 깜빡일 횟수 (켜짐/꺼짐 합계)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|HitReaction")
	float FlashInterval = 0.08f; // 깜빡이는 간격 (속도)

	/** 애니메이션 에셋들 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

public:
	/** 경험치 추가 함수 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddExp(float Amount);

	/** 현재 레벨 반환 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	int32 GetCharacterLevel() const { return Level; }

	/** 경험치 비율 반환 (0.0 ~ 1.0) */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetExpPercent() const { return CurrentExp / ExpToNextLevel; }

	/** 골드 추가 함수 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddGold(int32 Amount);

	/** 현재 골드 반환 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	int32 GetGold() const { return Gold; }

	/** 현재 공격력 반환 (퍼센트 보너스 포함) */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetCurrentAttackDamage() const { return BaseDamage * (1.0f + AttackDamageBonus); }

	/** 기본 공격력 반환 (기준값) */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetBaseDamage() const { return BaseDamage; }

	/** 현재 체력 비율 반환 (0.0 ~ 1.0) */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetHealthPercent() const { return CurrentHealth / MaxHealth; }

	/** 사망 여부 반환 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	bool IsDead() const { return bIsDead; }

	/** 애니메이션 에셋 Getter */
	UAnimMontage* GetAttackMontage() const { return AttackMontage; }

	/** 전투 컴포넌트 Getter */
	UDuckCombatComponent* GetCombatComponent() const { return CombatComp; }
};
