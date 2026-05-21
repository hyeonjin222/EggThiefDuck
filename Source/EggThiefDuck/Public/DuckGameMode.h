// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DuckGameMode.generated.h"

/** 게임 페이즈 정의 */
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Night UMETA(DisplayName = "Night (Battle)"),
	Morning UMETA(DisplayName = "Morning (Flee)"),
	Day UMETA(DisplayName = "Day (Shop)")
};

UCLASS()
class EGGTHIEFDUCK_API ADuckGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADuckGameMode();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/** 현재 시간 (0.0 ~ 24.0) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game|Time")
	float CurrentHour = 0.0f;

	/** 게임 시간 흐름 속도 (현실 1초당 몇 분이 흐를지) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Time")
	float TimeScale = 0.1f;

	/** 현재 게임 일차 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game|Time")
	int32 CurrentDay = 1;

	/** 현재 게임 페이즈 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game|Time")
	EGamePhase CurrentPhase = EGamePhase::Night;

protected:
	/** 태양/달 역할을 할 조명 (에디터에서 지정하거나 자동 탐색) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Visual")
	TObjectPtr<class ADirectionalLight> MainLight;

	/** 페이즈 변경 시 호출되는 이벤트 (UI 업데이트용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game|Event")
	void OnPhaseChanged(EGamePhase NewPhase);

private:
	void UpdateTime(float DeltaTime);
	void UpdateLightRotation();
	void CheckPhaseTransition();
	void SetPhase(EGamePhase NewPhase);

	/** 전투 시작 시간 (7 PM) */
	const float NightStartTime = 19.0f;
	/** 아침(퇴각) 시작 시간 (6 AM) */
	const float MorningStartTime = 6.0f;
	/** 정비(낮) 시작 시간 (8 AM) */
	const float DayStartTime = 8.0f;
};
