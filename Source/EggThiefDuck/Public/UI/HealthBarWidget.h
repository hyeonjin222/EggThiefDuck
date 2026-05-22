// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

class UProgressBar;

/**
 * 캐릭터나 몹 머리 위에 뜨는 작은 체력/탄약바 위젯의 베이스 클래스
 */
UCLASS()
class EGGTHIEFDUCK_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 체력 비율 업데이트 (0.0 ~ 1.0) */
	void UpdateHealthPercent(float Percent);

	/** 탄약 게이지 비율 업데이트 (0.0 ~ 1.0) */
	void UpdateAmmoPercent(float Percent);

	/** 탄약 게이지 색상 변경 */
	void SetAmmoBarColor(FLinearColor Color);

	/** 페이드 아웃 시작 */
	void StartFadeOut(float Duration);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 체력바 UI 바인딩 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UProgressBar> ProgressBar_Health;

	/** 탄약바 UI 바인딩 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UProgressBar> ProgressBar_Ammo;

private:
	/** 페이드 아웃 제어 변수 */
	bool bIsFadingOut = false;
	float FadeTimer = 0.0f;
	float FadeDuration = 2.0f;
};
