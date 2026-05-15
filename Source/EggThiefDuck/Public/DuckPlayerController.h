// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DuckPlayerController.generated.h"

class UMainHUDWidget;

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
	/** HUD 업데이트 함수들 */
	void UpdateHUDHealth(float CurrentHP, float MaxHP);
	void UpdateHUDGold(int32 Amount);
	void UpdateHUDTime(int32 Day, float Hour);

private:
	/** 위젯 클래스 (블루프린트에서 지정) */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UMainHUDWidget> MainHUDClass;

	/** 실제 생성된 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UMainHUDWidget> MainHUDWidget;
};
