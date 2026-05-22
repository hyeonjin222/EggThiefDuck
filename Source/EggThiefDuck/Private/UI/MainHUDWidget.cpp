#include "UI/MainHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UMainHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 시작 시 알림 텍스트 숨김 처리
	if (Text_DayNotification)
	{
		Text_DayNotification->SetRenderOpacity(0.0f);
		Text_DayNotification->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 1. 날짜 알림 애니메이션 처리
	if (bIsAnimatingNotification && Text_DayNotification)
	{
		NotificationTimer += InDeltaTime;
		float Progress = FMath::Clamp(NotificationTimer / NotificationDuration, 0.0f, 1.0f);
		float Opacity = FMath::Sin(Progress * PI);
		Text_DayNotification->SetRenderOpacity(Opacity);

		if (NotificationTimer >= NotificationDuration)
		{
			bIsAnimatingNotification = false;
			NotificationTimer = 0.0f;
			Text_DayNotification->SetRenderOpacity(0.0f);
			Text_DayNotification->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// 2. HUD 전체 페이드 아웃 처리 (사망 시)
	if (bIsFadingOut)
	{
		FadeTimer += InDeltaTime;
		float Progress = FMath::Clamp(FadeTimer / FadeDuration, 0.0f, 1.0f);
		
		// 1.0(보임) -> 0.0(안보임)으로 부드럽게 감소
		SetRenderOpacity(1.0f - Progress);

		if (FadeTimer >= FadeDuration)
		{
			bIsFadingOut = false;
			SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UMainHUDWidget::ShowDayNotification(int32 Day)
{
	if (Text_DayNotification)
	{
		Text_DayNotification->SetText(FText::Format(FText::FromString("Day {0}"), FText::AsNumber(Day)));
		
		// 애니메이션 초기화 및 시작
		bIsAnimatingNotification = true;
		NotificationTimer = 0.0f;
		Text_DayNotification->SetRenderOpacity(0.0f);
		Text_DayNotification->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UMainHUDWidget::StartFadeOut(float Duration)
{
	bIsFadingOut = true;
	FadeDuration = Duration;
	FadeTimer = 0.0f;
}
