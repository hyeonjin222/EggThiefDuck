// Fill out your copyright notice in the Description page of Project Settings.

#include "DuckGameMode.h"
#include "DuckCharacter.h"
#include "DuckPlayerController.h"
#include "EnemyBase.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"

ADuckGameMode::ADuckGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	// 기본 클래스 설정
	DefaultPawnClass = ADuckCharacter::StaticClass();
	PlayerControllerClass = ADuckPlayerController::StaticClass();

	// 기본 시간 설정: 오후 3시(15:00)부터 시작하여 18:00에 1일차 시작 연출
	CurrentHour = 15.0f;
	CurrentDay = 0;
	LastNotifiedDay = 0;
	TimeScale = 4.0f; // 밤: 1초당 4분 (현실 1분당 게임 4시간, 밤 전체 약 3분)
	DayTimeSpeedMultiplier = 15.0f; // 낮: 1초당 60분 (4.0 * 15.0 = 60분 = 1시간)
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
	// 게임이 시작되지 않았으면 시간을 멈춤
	if (!bIsGameStarted) return;

	// 1. 현재 배율 계산 (분 단위 기준)
	float CurrentScale = TimeScale;

	// 아침 8시(DayStartTime)부터 저녁 6시(18:00)까지는 광속 스킵
	if (CurrentHour >= DayStartTime && CurrentHour < 18.0f)
	{
		CurrentScale *= DayTimeSpeedMultiplier;
	}

	// 2. 시간에 따라 시간 증가 (분 단위를 시간 단위로 변환: / 60.0f)
	CurrentHour += (DeltaTime * CurrentScale) / 60.0f;

	// 3. 18시(오후 6시) 날짜 변경 및 알림 로직
	if (CurrentHour >= 18.0f && LastNotifiedDay == CurrentDay)
	{
		// 기획 의도: 18시가 되면 새로운 날이 시작되는 느낌이므로 여기서 Day를 올림
		CurrentDay++;

		if (ADuckPlayerController* PC = Cast<ADuckPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			PC->ShowHUDDayNotification(CurrentDay);
		}
	}

	// 4. 24시가 되면 시간만 리셋 및 다음 알림을 위한 상태 동기화
	if (CurrentHour >= 24.0f)
	{
		CurrentHour -= 24.0f;
		LastNotifiedDay = CurrentDay;
	}

	// HUD 시간 업데이트
	if (ADuckPlayerController* PC = Cast<ADuckPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->UpdateHUDTime(CurrentDay, CurrentHour);
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
	// 시간대별 페이즈 전환 로직 (밤 19:00 ~ 새벽 06:00 처리)
	bool bIsNightTime = (CurrentHour >= NightStartTime) || (CurrentHour < MorningStartTime);

	if (bIsNightTime)
	{
		if (CurrentPhase != EGamePhase::Night) SetPhase(EGamePhase::Night);
	}
	else if (CurrentHour >= MorningStartTime && CurrentHour < DayStartTime)
	{
		if (CurrentPhase != EGamePhase::Morning) SetPhase(EGamePhase::Morning);
	}
	else // 8 AM ~ 19 PM
	{
		if (CurrentPhase != EGamePhase::Day) SetPhase(EGamePhase::Day);
	}
}

void ADuckGameMode::SetPhase(EGamePhase NewPhase)
{
	if (CurrentPhase == NewPhase) return;

	CurrentPhase = NewPhase;
	OnPhaseChanged(CurrentPhase);

	// 1. 아침(퇴각) 페이즈 진입 시: 모든 적 도망 상태로 변경
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
	// 2. 낮(정비) 페이즈 진입 시: 아직 필드에 남은 모든 적 자연스럽게 소멸
	else if (NewPhase == EGamePhase::Day)
	{
		TArray<AActor*> FoundEnemies;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), FoundEnemies);
		for (AActor* EnemyActor : FoundEnemies)
		{
			if (AEnemyBase* Enemy = Cast<AEnemyBase>(EnemyActor))
			{
				Enemy->StartDespawning();
			}
		}
	}

	FString PhaseName = (NewPhase == EGamePhase::Night) ? TEXT("Night") : 
	                   (NewPhase == EGamePhase::Morning) ? TEXT("Morning") : TEXT("Day");

	UE_LOG(LogTemp, Log, TEXT("Game Phase Changed: %s"), *PhaseName);
}

void ADuckGameMode::StartGame()
{
	if (bIsGameStarted) return;

	bIsGameStarted = true;

	// 1. HUD 초기화 및 카메라 연출 시작
	if (ADuckPlayerController* PC = Cast<ADuckPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->InitializeHUD();

		if (ADuckCharacter* Player = Cast<ADuckCharacter>(PC->GetPawn()))
		{
			Player->StartCameraIntro();
		}
	}

	// 2. 울타리 제거 (IntroFence 태그를 가진 모든 액터 삭제)
	TArray<AActor*> FoundFences;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("IntroFence"), FoundFences);
	for (AActor* FenceActor : FoundFences)
	{
		FenceActor->Destroy();
	}

	UE_LOG(LogTemp, Log, TEXT("Game Started! Intro sequence finished."));
}
