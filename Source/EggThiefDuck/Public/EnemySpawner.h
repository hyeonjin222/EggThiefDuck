// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

/** 시간대별 웨이브 설정을 담는 구조체 */
USTRUCT(BlueprintType)
struct FWaveSetting
{
	GENERATED_BODY()

	/** 해당 세팅이 시작되는 시간 (0.0 ~ 24.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartHour = 19.0f;

	/** 스폰 주기 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnInterval = 2.0f;

	/** 한 번에 스폰할 마리 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnCountPerInterval = 1;

	/** 필드 최대 적 마리 수 (구간별 점프용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxActiveEnemies = 50;

	/** 스폰할 적 클래스와 등장 확률 (가중치) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<class AEnemyBase>, float> EnemyPool;
};

UCLASS()
class EGGTHIEFDUCK_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemySpawner();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

protected:
	/** 시간대별 스케줄 설정 (에디터에서 19시, 21시, 0시, 3시 순서대로 추가 권장) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
	TArray<FWaveSetting> WaveSchedule;

	/** 스폰 반경 설정 (도넛 모양) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
	float MinSpawnRadius = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
	float MaxSpawnRadius = 1800.0f;

private:
	/** 현재 시간에 맞는 웨이브 세팅 업데이트 */
	void UpdateCurrentWaveSetting();

	/** 적 스폰 실행 */
	void SpawnEnemies();

	/** 현재 필드에 활성화된 적의 수 계산 */
	int32 GetActiveEnemyCount();

	/** 랜덤 스폰 위치 계산 (도넛 모양) */
	FVector GetRandomSpawnLocation();

	/** 가중치 기반 적 클래스 선택 */
	TSubclassOf<class AEnemyBase> GetRandomEnemyClass();

	/** 현재 적용 중인 세팅 포인터 */
	FWaveSetting* CurrentSetting = nullptr;

	/** 스폰 타이머 */
	float SpawnTimer = 0.0f;

	/** 플레이어 참조 캐싱 */
	UPROPERTY()
	TObjectPtr<AActor> PlayerActor;
};
