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
	void PlayAttackMontage();

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

	/** 재화 (Gold) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 Gold = 0;

	/** 사망 처리 */
	void Die();

	/** UI 동기화 함수 */
	void RefreshHUD();

protected:
	/** 애니메이션 에셋들 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

public:
	/** 골드 추가 함수 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddGold(int32 Amount);

	/** 골드 양 반환 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetGold() const { return Gold; }

	/** 현재 체력 비율 반환 (0.0 ~ 1.0) */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetHealthPercent() const { return CurrentHealth / MaxHealth; }

	/** 애니메이션 에셋 Getter */
	UAnimMontage* GetAttackMontage() const { return AttackMontage; }

	/** 전투 컴포넌트 Getter */
	UDuckCombatComponent* GetCombatComponent() const { return CombatComp; }
};
