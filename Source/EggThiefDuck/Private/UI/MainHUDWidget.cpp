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

	if (bIsAnimatingNotification && Text_DayNotification)
	{
		NotificationTimer += InDeltaTime;

		// 0.0 ~ 1.0 사이의 진행도 계산
		float Progress = FMath::Clamp(NotificationTimer / NotificationDuration, 0.0f, 1.0f);

		// 페이드 인/아웃 곡선 계산 (Sin 함수 활용: 0 -> 1 -> 0)
		// 0~PI 구간의 Sin 값은 0에서 시작해 1을 찍고 다시 0으로 돌아옴
		float Opacity = FMath::Sin(Progress * PI);

		Text_DayNotification->SetRenderOpacity(Opacity);

		// 애니메이션 종료 체크
		if (NotificationTimer >= NotificationDuration)
		{
			bIsAnimatingNotification = false;
			NotificationTimer = 0.0f;
			Text_DayNotification->SetRenderOpacity(0.0f);
			Text_DayNotification->SetVisibility(ESlateVisibility::Hidden);
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
