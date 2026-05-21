#include "UI/DuckCursorWidget.h"

void UDuckCursorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 에디터에서 설정한 피벗 값을 렌더 변형 피벗으로 적용
	SetRenderTransformPivot(ScalingPivot);
}

void UDuckCursorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 마우스 위치 업데이트를 여기서 수행하여 UI 렌더링 지연 최소화
	if (APlayerController* PC = GetOwningPlayer())
	{
		float MouseX, MouseY;
		if (PC->GetMousePosition(MouseX, MouseY))
		{
			SetPositionInViewport(FVector2D(MouseX, MouseY));
		}
	}

	// 현재 스케일을 1.0으로 부드럽게 복구
	if (CurrentScale > 1.0f)
	{
		CurrentScale = FMath::FInterpTo(CurrentScale, 1.0f, InDeltaTime, ReturnSpeed);
		SetRenderScale(FVector2D(CurrentScale));
	}
}

void UDuckCursorWidget::NotifyFire()
{
	// 발사 시 최대 크기로 설정
	CurrentScale = MaxScale;
	SetRenderScale(FVector2D(CurrentScale));
}
