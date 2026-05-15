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
	if (OwnerCharacter && ProjectileClass)
	{
		// 캐릭터가 조준 방향과 충분히 정렬되었는지 확인
		if (!OwnerCharacter->IsAlignedWithCursor())
		{
			// 정렬되지 않았다면 이번 사격은 스킵
			return;
		}

		FVector SpawnLocation = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * 100.f;
		FRotator SpawnRotation = OwnerCharacter->GetActorRotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerCharacter;
		SpawnParams.Instigator = OwnerCharacter;

		AEggProjectile* Projectile = GetWorld()->SpawnActor<AEggProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (Projectile)
		{
			Projectile->FireInDirection(OwnerCharacter->GetActorForwardVector());
			
			// 게이지 소모 및 시간 기록
			CurrentEggGauge -= GaugeCostPerShot;
			LastFireTime = GetWorld()->GetTimeSeconds();

			// [버그 해결 핵심] 한 발이라도 나갔다면 클릭에 의한 '최소 보장 의사'는 즉시 소모합니다.
			// 이제 마우스를 계속 누르고 있다면 bIsFiring에 의해 연사가 유지되고, 
			// 마우스를 뗐다면 여기서 bWantsToFire가 꺼지므로 정확히 한 발만 쏘고 멈춥니다.
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
