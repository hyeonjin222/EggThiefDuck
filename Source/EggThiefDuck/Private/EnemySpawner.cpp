// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemySpawner.h"
#include "DuckGameMode.h"
#include "EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"

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
	int32 Day = GM->CurrentDay;
	
	// 날짜와 시간을 합친 절대적인 선형 시간 계산 (단위: 시간)
	// 예: 1일차 18.1시 = 1*24 + 18.1 = 42.1
	auto GetAbsoluteTotalHour = [](int32 D, float H) -> float {
		return ((float)D * 24.0f) + H;
	};

	float TotalCurrentHour = GetAbsoluteTotalHour(Day, Hour);
	FWaveSetting* BestSetting = nullptr;
	float BestAbsoluteStart = -1.0f;

	for (int32 i = 0; i < WaveSchedule.Num(); ++i)
	{
		float AbsoluteStart = GetAbsoluteTotalHour(WaveSchedule[i].StartDay, WaveSchedule[i].StartHour);
		
		// 현재 시간보다 이전(또는 같은) 시간에 시작하는 세팅 중 가장 늦은 것 선택
		if (AbsoluteStart <= TotalCurrentHour)
		{
			if (AbsoluteStart > BestAbsoluteStart)
			{
				BestAbsoluteStart = AbsoluteStart;
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
		
		// 유효하지 않은 위치(FVector::ZeroVector 등)가 반환되면 스폰 건너뜀
		if (SpawnLoc.IsNearlyZero()) continue;

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

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys) return FVector::ZeroVector;

	FVector PlayerLoc = PlayerActor->GetActorLocation();
	FVector Velocity = PlayerActor->GetVelocity();
	
	const int32 MaxAttempts = 10; // 유효한 위치를 찾기 위한 최대 시도 횟수
	
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		float FinalAngle = FMath::FRandRange(0.0f, 360.0f);

		// 플레이어가 이동 중일 때 (속도가 일정 이상일 때)
		if (Velocity.SizeSquared() > FMath::Square(100.0f))
		{
			// 첫 5번의 시도 동안은 70% 확률로 플레이어 진행 방향 앞쪽 120도 내에 생성 시도
			if (Attempt < 5 && FMath::FRand() < 0.7f)
			{
				float MoveAngle = Velocity.Rotation().Yaw;
				FinalAngle = MoveAngle + FMath::RandRange(-60.0f, 60.0f);
			}
		}

		// 도넛 반경 내 랜덤 위치 계산
		float RandomDistance = FMath::FRandRange(MinSpawnRadius, MaxSpawnRadius);

		// Yaw만 적용하여 평면상의 오프셋 생성
		FVector Offset = UKismetMathLibrary::CreateVectorFromYawPitch(FinalAngle, 0.0f, RandomDistance);
		FVector CandidateLoc = PlayerLoc + Offset;
		
		// 바닥 높이 보정
		CandidateLoc.Z = PlayerLoc.Z;

		// --- 내비게이션 메쉬(NavMesh) 유효성 검사 ---
		FNavLocation ProjectedLoc;
		// 후보지가 실제 몹이 다닐 수 있는 길(NavMesh) 위인지 확인
		if (NavSys->ProjectPointToNavigation(CandidateLoc, ProjectedLoc, FVector(500.f, 500.f, 500.f)))
		{
			return ProjectedLoc.Location;
		}
	}

	// 모든 시도가 실패하면 유효하지 않은 위치 반환
	return FVector::ZeroVector;
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
