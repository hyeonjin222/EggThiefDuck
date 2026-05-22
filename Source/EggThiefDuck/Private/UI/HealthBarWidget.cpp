// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HealthBarWidget.h"
#include "Components/ProgressBar.h"

void UHealthBarWidget::UpdateHealthPercent(float Percent)
{
	if (ProgressBar_Health)
	{
		ProgressBar_Health->SetPercent(Percent);
	}
}

void UHealthBarWidget::UpdateAmmoPercent(float Percent)
{
	if (ProgressBar_Ammo)
	{
		ProgressBar_Ammo->SetPercent(Percent);
	}
}

void UHealthBarWidget::SetAmmoBarColor(FLinearColor Color)
{
	if (ProgressBar_Ammo)
	{
		ProgressBar_Ammo->SetFillColorAndOpacity(Color);
	}
}

void UHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsFadingOut)
	{
		FadeTimer += InDeltaTime;
		float Progress = FMath::Clamp(FadeTimer / FadeDuration, 0.0f, 1.0f);
		
		// 부드럽게 투명도 감소
		SetRenderOpacity(1.0f - Progress);

		if (FadeTimer >= FadeDuration)
		{
			bIsFadingOut = false;
			SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UHealthBarWidget::StartFadeOut(float Duration)
{
	bIsFadingOut = true;
	FadeDuration = Duration;
	FadeTimer = 0.0f;
}
