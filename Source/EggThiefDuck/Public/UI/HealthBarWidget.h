// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

/**
 * 캐릭터나 몹 머리 위에 뜨는 작은 체력바 위젯의 베이스 클래스
 */
UCLASS()
class EGGTHIEFDUCK_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 체력 비율 업데이트 (0.0 ~ 1.0) */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateHealthPercent(float Percent);
};
