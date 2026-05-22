// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 게임의 메인 HUD 위젯 베이스 클래스
 */
UCLASS()
class EGGTHIEFDUCK_API UMainHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 체력 업데이트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void UpdateHealth(float CurrentHP, float MaxHP);

	/** 경험치 및 레벨 업데이트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void UpdateXP(int32 Level, float CurrentXP, float MaxXP);

	/** 골드 업데이트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void UpdateGold(int32 Amount);

	/** 시간 업데이트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void UpdateTime(int32 Day, float Hour);

	/** 날짜 변경 알림 연출 (C++에서 직접 처리) */
	void ShowDayNotification(int32 Day);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** --- UI 컴포넌트 바인딩 --- */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UProgressBar> ProgressBar_XP;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UTextBlock> Text_Level;

	/** 날짜 알림 텍스트 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UTextBlock> Text_DayNotification;

private:
	/** 알림 애니메이션 제어 변수 */
	bool bIsAnimatingNotification = false;
	float NotificationTimer = 0.0f;
	const float NotificationDuration = 3.0f; // 총 3초 동안 연출
};
