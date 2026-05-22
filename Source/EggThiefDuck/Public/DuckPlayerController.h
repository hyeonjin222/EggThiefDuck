// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DuckPlayerController.generated.h"

class UMainHUDWidget;
class UDuckCursorWidget;

/**
 * UI 관리를 전담하는 플레이어 컨트롤러
 */
UCLASS()
class EGGTHIEFDUCK_API ADuckPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	virtual void PlayerTick(float DeltaTime) override;

	/** HUD 생성 및 초기화 (인트로 종료 시 호출) */
	void InitializeHUD();

	/** HUD 업데이트 함수들 */
	void UpdateHUDHealth(float CurrentHP, float MaxHP);
	void UpdateHUDXP(int32 Level, float CurrentXP, float MaxXP);
	void UpdateHUDGold(int32 Amount);
	void UpdateHUDTime(int32 Day, float Hour);
	void ShowHUDDayNotification(int32 Day);

	/** 커서 애니메이션 재생 */
	void PlayCursorFireAnimation();

private:
	/** 위젯 클래스 (블루프린트에서 지정) */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UMainHUDWidget> MainHUDClass;

	/** 실제 생성된 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UMainHUDWidget> MainHUDWidget;

	/** 커서 위젯 클래스 */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UDuckCursorWidget> CursorWidgetClass;

	/** 커서 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UDuckCursorWidget> CursorWidget;
};
