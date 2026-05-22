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

/** 사운드 타입 정의 */
UENUM(BlueprintType)
enum class EDuckSoundType : uint8
{
	StartGame,
	LevelUp,
	UpgradeSelected,
	ItemPickup,
	PlayerDeath,
	ProjectileThrow
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

	/** 전역 사운드 재생 함수 */
	UFUNCTION(BlueprintCallable, Category = "Game|Sound")
	void PlayGlobalSound(EDuckSoundType SoundType, FVector Location = FVector::ZeroVector);

	/** 배경음악 제어 */
	void UpdateBGM(bool bIsIngame);

	/** 현재 시간 (0.0 ~ 24.0) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game|Time")
	float CurrentHour = 0.0f;

	/** 게임 시간 흐름 속도 (현실 1초당 몇 분이 흐를지) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Time")
	float TimeScale = 1.0f;

	/** 낮 시간(스킵 구간) 가속 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Time")
	float DayTimeSpeedMultiplier = 50.0f;

	/** 현재 게임 일차 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game|Time")
	int32 CurrentDay = 1;

	/** 현재 게임 페이즈 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game|Time")
	EGamePhase CurrentPhase = EGamePhase::Night;

	/** 게임 시작 여부 (인트로 완료 여부) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	bool bIsGameStarted = false;

	/** 게임을 시작하고 인트로 연출을 종료합니다. */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartGame();

	/** 게임을 종료합니다. (승리/패배) */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void EndGame(bool bIsVictory);

protected:
	/** 태양/달 역할을 할 조명 (에디터에서 지정하거나 자동 탐색) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Visual")
	TObjectPtr<class ADirectionalLight> MainLight;

	/** 사격 효과음 목록 (에디터에서 지정) */
	UPROPERTY(EditAnywhere, Category = "Game|Sound")
	TMap<EDuckSoundType, TObjectPtr<class USoundBase>> GlobalSoundMap;

	/** 배경음악 (인트로) */
	UPROPERTY(EditAnywhere, Category = "Game|Sound")
	TObjectPtr<class USoundBase> BGM_Intro;

	/** 배경음악 (인게임) */
	UPROPERTY(EditAnywhere, Category = "Game|Sound")
	TObjectPtr<class USoundBase> BGM_Ingame;

	/** 실제 배경음악 재생 컴포넌트 */
	UPROPERTY()
	TObjectPtr<class UAudioComponent> BGMPlayer;

	/** 게임 종료 시 호출되는 이벤트 (UI 표시용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game|Event")
	void OnGameEnded(bool bIsVictory);

	/** 페이즈 변경 시 호출되는 이벤트 (UI 업데이트용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game|Event")
	void OnPhaseChanged(EGamePhase NewPhase);

private:
	void UpdateTime(float DeltaTime);
	void UpdateLightRotation();
	void CheckPhaseTransition();
	void SetPhase(EGamePhase NewPhase);

	/** 전투 시작 시간 (6 PM) */
	const float NightStartTime = 18.0f;
	/** 아침(퇴각) 시작 시간 (6 AM) */
	const float MorningStartTime = 6.0f;
	/** 정비(낮) 시작 시간 (7 AM) */
	const float DayStartTime = 7.0f;

	/** 마지막으로 날짜 알림을 보낸 일차 */
	int32 LastNotifiedDay = 0;
};
