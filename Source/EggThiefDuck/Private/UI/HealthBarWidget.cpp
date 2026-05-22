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
