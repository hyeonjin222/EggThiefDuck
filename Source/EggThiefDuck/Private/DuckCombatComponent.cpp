// Fill out your copyright notice in the Description page of Project Settings.

#include "DuckCombatComponent.h"
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

	// 사격 중이고 쿨타임이 끝났으며 과열 상태가 아닐 때 발사
	if (bIsFiring && !bIsOverheated)
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
}

void UDuckCombatComponent::StopFire()
{
	bIsFiring = false;
}

void UDuckCombatComponent::Fire()
{
	if (CurrentEggGauge < GaugeCostPerShot)
	{
		// 탄약 부족 시 과열 처리
		bIsOverheated = true;
		bIsFiring = false;
		GetWorld()->GetTimerManager().SetTimer(OverheatTimerHandle, this, &UDuckCombatComponent::EndOverheat, OverheatPenaltyTime, false);
		
		UE_LOG(LogTemp, Warning, TEXT("Egg Gauge Empty! Overheated!"));
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
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
			LastFireTime = GetWorld()->GetTimeSeconds();
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

void UDuckCombatComponent::EndOverheat()
{
	bIsOverheated = false;
	CurrentEggGauge = MaxEggGauge; // 과열 회복 시 게이지 풀 충전 (기획에 따라 조절 가능)
	UE_LOG(LogTemp, Log, TEXT("Overheat Ended. Ready to Fire!"));
}
