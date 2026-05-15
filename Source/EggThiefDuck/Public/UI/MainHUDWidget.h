// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainHUDWidget.generated.h"

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

	/** 골드 업데이트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void UpdateGold(int32 Amount);

	/** 시간 업데이트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void UpdateTime(int32 Day, float Hour);
};
