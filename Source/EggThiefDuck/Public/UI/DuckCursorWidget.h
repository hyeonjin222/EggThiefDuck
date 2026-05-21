#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DuckCursorWidget.generated.h"

/**
 * 발사 시 반응하는 커스텀 마우스 커서 위젯
 */
UCLASS()
class EGGTHIEFDUCK_API UDuckCursorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 발사 시 호출하여 애니메이션 트리거 */
	void NotifyFire();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	/** 현재 렌더링 스케일 */
	float CurrentScale = 1.0f;

	/** 확대/축소의 중심점 (이미지의 Alignment 값과 일치시켜야 마우스 위치에서 정중앙으로 커짐) */
	UPROPERTY(EditAnywhere, Category = "Cursor")
	FVector2D ScalingPivot = FVector2D(0.5f, 0.5f);

	/** 원래 크기로 돌아오는 속도 */
	UPROPERTY(EditAnywhere, Category = "Cursor")
	float ReturnSpeed = 15.0f;

	/** 발사 시 순간적으로 확대될 크기 */
	UPROPERTY(EditAnywhere, Category = "Cursor")
	float MaxScale = 1.4f;
};
