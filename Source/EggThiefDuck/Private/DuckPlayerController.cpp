// Fill out your copyright notice in the Description page of Project Settings.

#include "DuckPlayerController.h"
#include "UI/MainHUDWidget.h"
#include "UI/DuckCursorWidget.h"
#include "Blueprint/UserWidget.h"

void ADuckPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 커스텀 커서 생성 (인트로 중에도 필요)
	if (CursorWidgetClass)
	{
		CursorWidget = CreateWidget<UDuckCursorWidget>(this, CursorWidgetClass);
		if (CursorWidget)
		{
			CursorWidget->AddToViewport(999); // 최상단에 배치
			bShowMouseCursor = false; // 윈도우 기본 커서는 숨김
		}
	}
}

void ADuckPlayerController::InitializeHUD()
{
	// 이미 생성되어 있다면 무시
	if (MainHUDWidget) return;

	// 메인 HUD 생성
	if (MainHUDClass)
	{
		MainHUDWidget = CreateWidget<UMainHUDWidget>(this, MainHUDClass);
		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport(100);
		}
	}
}

void ADuckPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// 커서 위치 업데이트
	if (CursorWidget)
	{
		// 시스템 커서가 활성화되면(UI 상황 등) 조준점 숨김, 꺼지면(전투 상황) 보임
		CursorWidget->SetVisibility(bShowMouseCursor ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);

		float MouseX, MouseY;
		if (GetMousePosition(MouseX, MouseY))
		{
			CursorWidget->SetPositionInViewport(FVector2D(MouseX, MouseY));
		}
	}
}

void ADuckPlayerController::PlayCursorFireAnimation()
{
	if (CursorWidget)
	{
		CursorWidget->NotifyFire();
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

void ADuckPlayerController::ShowHUDDayNotification(int32 Day)
{
	if (MainHUDWidget)
	{
		MainHUDWidget->ShowDayNotification(Day);
	}
}
