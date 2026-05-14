// Fill out your copyright notice in the Description page of Project Settings.

#include "DuckGameMode.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"

ADuckGameMode::ADuckGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	// 기본 시간 설정: 밤 12시부터 시작
	CurrentHour = 0.0f;
	TimeScale = 0.5f; // 기본값: 현실 1초당 게임 시간 0.5분 (조절 가능)
}

void ADuckGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 월드에서 DirectionalLight 자동 탐색 (지정되지 않았을 경우)
	if (!MainLight)
	{
		AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ADirectionalLight::StaticClass());
		MainLight = Cast<ADirectionalLight>(FoundActor);
	}

	SetPhase(EGamePhase::Night);
}

void ADuckGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTime(DeltaTime);
	UpdateLightRotation();
	CheckPhaseTransition();
}

void ADuckGameMode::UpdateTime(float DeltaTime)
{
	// 시간에 따라 시간 증가
	CurrentHour += (DeltaTime * TimeScale);

	if (CurrentHour >= 24.0f)
	{
		CurrentHour -= 24.0f;
		CurrentDay++;
	}
}

void ADuckGameMode::UpdateLightRotation()
{
	if (!MainLight) return;

	// 1. 4개의 핵심 시간대 로테이션 정의 (사용자 수치 기반)
	// Yaw는 -90 -> -180 -> -270 -> -360(0) 으로 흐름
	FQuat Q_6AM  = FRotator(0.f, -90.f, 0.f).Quaternion();
	FQuat Q_12PM = FRotator(-90.f, -180.f, 0.f).Quaternion();
	FQuat Q_6PM  = FRotator(0.f, -270.f, 0.f).Quaternion();
	FQuat Q_12AM = FRotator(85.f, -360.f, 0.f).Quaternion();

	FQuat TargetQuat;
	float Alpha = 0.0f;

	// 2. 현재 시간에 따른 구간 선택 및 보간 (Slerp)
	if (CurrentHour >= 6.0f && CurrentHour < 12.0f)
	{
		Alpha = (CurrentHour - 6.0f) / 6.0f;
		TargetQuat = FQuat::Slerp(Q_6AM, Q_12PM, Alpha);
	}
	else if (CurrentHour >= 12.0f && CurrentHour < 18.0f)
	{
		Alpha = (CurrentHour - 12.0f) / 6.0f;
		TargetQuat = FQuat::Slerp(Q_12PM, Q_6PM, Alpha);
	}
	else if (CurrentHour >= 18.0f && CurrentHour < 24.0f)
	{
		Alpha = (CurrentHour - 18.0f) / 6.0f;
		TargetQuat = FQuat::Slerp(Q_6PM, Q_12AM, Alpha);
	}
	else // 0 AM ~ 6 AM
	{
		Alpha = CurrentHour / 6.0f;
		TargetQuat = FQuat::Slerp(Q_12AM, Q_6AM, Alpha);
	}

	// 3. 최종 회전 적용 (쿼터니언을 사용하여 짐벌 락과 이상한 휘청거림 방지)
	MainLight->SetActorRotation(TargetQuat);
}

void ADuckGameMode::CheckPhaseTransition()
{
	// 시간대별 페이즈 전환 로직
	if (CurrentHour >= NightStartTime && CurrentHour < MorningStartTime)
	{
		if (CurrentPhase != EGamePhase::Night) SetPhase(EGamePhase::Night);
	}
	else if (CurrentHour >= MorningStartTime && CurrentHour < DayStartTime)
	{
		if (CurrentPhase != EGamePhase::Morning) SetPhase(EGamePhase::Morning);
	}
	else // 8 AM ~ 12 AM
	{
		if (CurrentPhase != EGamePhase::Day) SetPhase(EGamePhase::Day);
	}
}

#include "DuckGameMode.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyBase.h"

void ADuckGameMode::SetPhase(EGamePhase NewPhase)
{
	if (CurrentPhase == NewPhase) return;

	CurrentPhase = NewPhase;
	OnPhaseChanged(CurrentPhase);

	// 아침이 되면 모든 적 도망 상태로 변경
	if (NewPhase == EGamePhase::Morning)
	{
		TArray<AActor*> FoundEnemies;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), FoundEnemies);
		for (AActor* EnemyActor : FoundEnemies)
		{
			if (AEnemyBase* Enemy = Cast<AEnemyBase>(EnemyActor))
			{
				Enemy->SetState(EEnemyState::Fleeing);
			}
		}
	}

	FString PhaseName = (NewPhase == EGamePhase::Night) ? TEXT("Night") : 
	                   (NewPhase == EGamePhase::Morning) ? TEXT("Morning") : TEXT("Day");
	
	UE_LOG(LogTemp, Log, TEXT("Game Phase Changed: %s"), *PhaseName);
}
