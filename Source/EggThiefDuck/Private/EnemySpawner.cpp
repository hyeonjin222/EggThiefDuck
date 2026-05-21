// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemySpawner.h"
#include "DuckGameMode.h"
#include "EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 현재 시간에 맞는 세팅 업데이트
	UpdateCurrentWaveSetting();

	// 2. 밤 페이즈일 때만 스폰 타이머 작동
	ADuckGameMode* GM = Cast<ADuckGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM && GM->CurrentPhase == EGamePhase::Night && CurrentSetting)
	{
		SpawnTimer += DeltaTime;
		if (SpawnTimer >= CurrentSetting->SpawnInterval)
		{
			SpawnTimer = 0.0f;
			SpawnEnemies();
		}
	}
}

void AEnemySpawner::UpdateCurrentWaveSetting()
{
	ADuckGameMode* GM = Cast<ADuckGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM) return;

	float Hour = GM->CurrentHour;
	
	// 시간을 19시 기준으로 정규화하여 선형적으로 비교 (19시=0, 06시=11)
	auto GetRelativeHour = [](float H) -> float {
		if (H >= 19.0f) return H - 19.0f;
		return H + 5.0f;
	};

	float RelativeCurrentHour = GetRelativeHour(Hour);
	FWaveSetting* BestSetting = nullptr;
	float BestRelativeStart = -1.0f;

	for (int32 i = 0; i < WaveSchedule.Num(); ++i)
	{
		float RelativeStart = GetRelativeHour(WaveSchedule[i].StartHour);
		
		// 현재 시간보다 이전(또는 같은) 시간에 시작하는 세팅 중 가장 늦은 것 선택
		if (RelativeStart <= RelativeCurrentHour)
		{
			if (RelativeStart > BestRelativeStart)
			{
				BestRelativeStart = RelativeStart;
				BestSetting = &WaveSchedule[i];
			}
		}
	}

	CurrentSetting = BestSetting;
}

void AEnemySpawner::SpawnEnemies()
{
	if (!CurrentSetting || !PlayerActor) return;

	// 현재 활성화된 적 수 체크
	int32 CurrentCount = GetActiveEnemyCount();
	if (CurrentCount >= CurrentSetting->MaxActiveEnemies) return;

	// 한 번에 소환할 양만큼 루프
	for (int32 i = 0; i < CurrentSetting->SpawnCountPerInterval; ++i)
	{
		// 루프 중간에도 최대치 체크
		if (GetActiveEnemyCount() >= CurrentSetting->MaxActiveEnemies) break;

		TSubclassOf<AEnemyBase> EnemyClass = GetRandomEnemyClass();
		if (!EnemyClass) continue;

		FVector SpawnLoc = GetRandomSpawnLocation();
		FRotator SpawnRot = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, SpawnLoc, SpawnRot, SpawnParams);
	}
}

int32 AEnemySpawner::GetActiveEnemyCount()
{
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), FoundEnemies);
	return FoundEnemies.Num();
}

FVector AEnemySpawner::GetRandomSpawnLocation()
{
	if (!PlayerActor) return GetActorLocation();

	FVector PlayerLoc = PlayerActor->GetActorLocation();
	FVector Velocity = PlayerActor->GetVelocity();
	
	float FinalAngle = FMath::FRandRange(0.0f, 360.0f);

	// 플레이어가 이동 중일 때 (속도가 일정 이상일 때)
	if (Velocity.SizeSquared() > FMath::Square(100.0f))
	{
		// 70% 확률로 플레이어 진행 방향 앞쪽 120도 내에 생성
		if (FMath::FRand() < 0.7f)
		{
			float MoveAngle = Velocity.Rotation().Yaw;
			FinalAngle = MoveAngle + FMath::RandRange(-60.0f, 60.0f);
		}
	}

	// 도넛 반경 내 랜덤 위치 계산
	float RandomDistance = FMath::FRandRange(MinSpawnRadius, MaxSpawnRadius);

	// Yaw만 적용하여 평면상의 오프셋 생성
	FVector Offset = UKismetMathLibrary::CreateVectorFromYawPitch(FinalAngle, 0.0f, RandomDistance);
	FVector SpawnLoc = PlayerLoc + Offset;
	
	// 바닥 높이 보정
	SpawnLoc.Z = PlayerLoc.Z;

	return SpawnLoc;
}

TSubclassOf<AEnemyBase> AEnemySpawner::GetRandomEnemyClass()
{
	if (!CurrentSetting || CurrentSetting->EnemyPool.Num() == 0) return nullptr;

	float TotalWeight = 0.0f;
	for (auto& Elem : CurrentSetting->EnemyPool)
	{
		TotalWeight += Elem.Value;
	}

	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;

	for (auto& Elem : CurrentSetting->EnemyPool)
	{
		CurrentWeight += Elem.Value;
		if (RandomValue <= CurrentWeight)
		{
			return Elem.Key;
		}
	}

	return nullptr;
}
