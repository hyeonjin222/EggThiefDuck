// Fill out your copyright notice in the Description page of Project Settings.

#include "DuckPlayerController.h"
#include "UI/MainHUDWidget.h"
#include "Blueprint/UserWidget.h"

void ADuckPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 메인 HUD 생성
	if (MainHUDClass)
	{
		MainHUDWidget = CreateWidget<UMainHUDWidget>(this, MainHUDClass);
		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport();
		}
	}
}

void ADuckPlayerController::UpdateHUDHealth(float CurrentHP, float MaxHP)
{
	if (MainHUDWidget)
	{
		MainHUDWidget->UpdateHealth(CurrentHP, MaxHP);
	}
}

void ADuckPlayerController::UpdateHUDXP(int32 Level, float CurrentXP, float MaxXP)
{
	if (MainHUDWidget)
	{
		MainHUDWidget->UpdateXP(Level, CurrentXP, MaxXP);
	}
}

void ADuckPlayerController::UpdateHUDGold(int32 Amount)
{
	if (MainHUDWidget)
	{
		MainHUDWidget->UpdateGold(Amount);
	}
}

void ADuckPlayerController::UpdateHUDTime(int32 Day, float Hour)
{
	if (MainHUDWidget)
	{
		MainHUDWidget->UpdateTime(Day, Hour);
	}
}
