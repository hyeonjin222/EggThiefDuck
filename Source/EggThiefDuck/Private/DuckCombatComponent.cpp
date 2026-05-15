// Fill out your copyright notice in the Description page of Project Settings.

#include "DuckCombatComponent.h"
#include "DuckCharacter.h"
#include "EggProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

UDuckCombatComponent::UDuckCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	CurrentEggGauge = MaxEggGauge;
}

void UDuckCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDuckCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateEggGauge(DeltaTime);

	// 사격 조건 확인: 마우스를 누르고 있거나(bIsFiring), 최소 한 발 보장이 필요한 상태(bWantsToFire)
	if ((bIsFiring || bWantsToFire) && !bIsOverheated)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastFireTime >= FireRate)
		{
			Fire();
		}
	}
}

void UDuckCombatComponent::StartFire()
{
	if (bIsOverheated) return;

	bIsFiring = true;
	bWantsToFire = true; // 사격 의사 표시 (최소 1발 보장 시작)
}

void UDuckCombatComponent::StopFire()
{
	bIsFiring = false;
	// bWantsToFire는 건드리지 않습니다. Fire() 함수에서 1발을 쏘고 나서 스스로 꺼질 것입니다.
}

void UDuckCombatComponent::Fire()
{
	if (CurrentEggGauge < GaugeCostPerShot)
	{
		// 탄약 부족 시 과열 처리
		bIsOverheated = true;
		bIsFiring = false;
		bWantsToFire = false;
		GetWorld()->GetTimerManager().SetTimer(OverheatTimerHandle, this, &UDuckCombatComponent::EndOverheat, OverheatPenaltyTime, false);
		
		UE_LOG(LogTemp, Warning, TEXT("Egg Gauge Empty! Overheated!"));
		return;
	}

	ADuckCharacter* OwnerCharacter = Cast<ADuckCharacter>(GetOwner());
	if (OwnerCharacter && OwnerCharacter->GetAttackMontage())
	{
		// 1. 조준 확인
		if (!OwnerCharacter->IsAlignedWithCursor())
		{
			return;
		}

		// 2. 애니메이션 재생 속도 계산
		// FireRate(연사 간격) 안에 애니메이션이 딱 맞춰 끝나도록 PlayRate 결정
		float MontageLength = OwnerCharacter->GetAttackMontage()->GetPlayLength();
		float PlayRate = MontageLength / FireRate;

		// 3. 공격 애니메이션 재생 (계산된 배율 적용)
		// 블렌드 타임 계산 공식: (FireRate - 0.1) / 2
		float DynamicBlendTime = FMath::Max(0.01f, (FireRate - 0.1f) / 2.0f);
		OwnerCharacter->PlayAttackMontage(PlayRate, DynamicBlendTime); 

		// 4. 지연 발사 타이머 설정
		// 사용자 설정 비율(FireDelayRatio)에 따라 발사 시점 결정
		// 애니메이션이 FireRate 시간에 맞춰 재생되므로, 그 비율만큼의 시간 후에 발사합니다.
		float ScaledFireDelay = FireRate * FireDelayRatio;
		
		FTimerHandle FireTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &UDuckCombatComponent::FireProjectile, ScaledFireDelay, false);
		
		// 쿨타임 계산용 시간 기록
		LastFireTime = GetWorld()->GetTimeSeconds();
	}
}

void UDuckCombatComponent::FireProjectile()
{
	ADuckCharacter* OwnerCharacter = Cast<ADuckCharacter>(GetOwner());
	if (OwnerCharacter && ProjectileClass)
	{
		FVector SpawnLocation = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * 100.f;
		FRotator SpawnRotation = OwnerCharacter->GetActorRotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerCharacter;
		SpawnParams.Instigator = OwnerCharacter;

		AEggProjectile* Projectile = GetWorld()->SpawnActor<AEggProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (Projectile)
		{
			Projectile->FireInDirection(OwnerCharacter->GetActorForwardVector());
			
			// 게이지 소모
			CurrentEggGauge -= GaugeCostPerShot;

			// 클릭 보장 의사 소모
			bWantsToFire = false;
		}
	}
}

void UDuckCombatComponent::UpdateEggGauge(float DeltaTime)
{
	// 사격 중이 아닐 때만 게이지 회복
	if (!bIsFiring && CurrentEggGauge < MaxEggGauge)
	{
		CurrentEggGauge = FMath::Min(MaxEggGauge, CurrentEggGauge + (GaugeRecoveryRate * DeltaTime));
	}
}

void UDuckCombatComponent::RefillGauge(float Amount)
{
	CurrentEggGauge = FMath::Min(MaxEggGauge, CurrentEggGauge + Amount);
	
	// 과열 상태 해제 (아이템 획득 시 즉시 사용 가능하게)
	if (bIsOverheated)
	{
		GetWorld()->GetTimerManager().ClearTimer(OverheatTimerHandle);
		EndOverheat();
	}
}

void UDuckCombatComponent::EndOverheat()
{
	bIsOverheated = false;
	CurrentEggGauge = MaxEggGauge; // 과열 회복 시 게이지 풀 충전 (기획에 따라 조절 가능)
	UE_LOG(LogTemp, Log, TEXT("Overheat Ended. Ready to Fire!"));
}
