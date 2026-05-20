// Fill out your copyright notice in the Description page of Project Settings.

#include "DuckCombatComponent.h"
#include "DuckCharacter.h"
#include "EggProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Data/UpgradeDataAsset.h"

UDuckCombatComponent::UDuckCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	CurrentEggGauge = MaxEggGauge;
}

void UDuckCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// 최초 설정된 연사력을 기준값으로 저장
	BaseFireRate = FireRate;
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
		// --- 변이 시너지 파라미터 계산 ---
		float FinalSpeed = DefaultProjectileSpeed * (1.0f + ProjectileSpeedBonus);
		float FinalLifeSpan = DefaultLifeSpan * (1.0f + RangeBonus); // [수정] 사거리 보너스
		float FinalSpreadAngle = 10.0f + SpreadAngleBonus;
		FVector FinalScale = FVector(1.0f + ProjectileSizeBonus); // [수정] 크기 보너스

		if (bSniperEnabled)
		{
			FinalSpeed *= 2.0f;
			FinalLifeSpan = SniperLifeSpan;
			FinalSpreadAngle = 0.5f;
			FinalScale *= FVector(3.0f, 0.2f, 0.2f);
		}
		else if (bShotgunEnabled)
		{
			FinalLifeSpan *= 0.15f; // 샷건은 기본 사거리의 15% 수준 (짧음)
			FinalSpreadAngle = FMath::Max(30.0f, FinalSpreadAngle);
			FinalScale *= 1.5f;
		}
		else if (bFlamethrowerEnabled)
		{
			FinalSpeed = FlamethrowerProjectileSpeed;
			FinalLifeSpan = FlamethrowerLifeSpan;
			FinalSpreadAngle = 40.0f;
			FinalScale *= 2.5f;
		}

		float StartYaw = -(MultiShotCount - 1) * FinalSpreadAngle * 0.5f;

		for (int32 i = 0; i < MultiShotCount; ++i)
		{
			FVector SpawnLocation = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * 100.f;
			FRotator SpawnRotation = OwnerCharacter->GetActorRotation();
			SpawnRotation.Yaw += StartYaw + (i * FinalSpreadAngle);

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = OwnerCharacter;
			SpawnParams.Instigator = OwnerCharacter;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AEggProjectile* Projectile = GetWorld()->SpawnActor<AEggProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
			if (Projectile)
			{
				// 변이된 스탯 적용
				Projectile->SetSpeed(FinalSpeed);
				Projectile->SetLifeSpan(FinalLifeSpan);
				Projectile->SetActorScale3D(FinalScale);

				// [수정] 시너지 효과 및 세부 보너스 전달
				Projectile->SetDamage(OwnerCharacter->GetCurrentAttackDamage());
				Projectile->SetKnockbackBonus(KnockbackBonus);
				Projectile->SetExplosionRadiusBonus(ExplosionRadiusBonus);
				
				Projectile->SetPiercing(bPiercingEnabled);
				Projectile->SetExplosive(bExplosiveEnabled);
				
				// 등록된 모든 트레일 VFX 적용
				for (UNiagaraSystem* TrailVFX : ProjectileTrailVFXs)
				{
					Projectile->AddTrailVFX(TrailVFX);
				}

				Projectile->FireInDirection(SpawnRotation.Vector());
			}
		}

		// 게이지 소모
		CurrentEggGauge -= GaugeCostPerShot;
		bWantsToFire = false;
	}
}

void UDuckCombatComponent::ApplyUpgrade(UUpgradeDataAsset* Upgrade)
{
	if (!Upgrade) return;

	// 발사체 트레일 VFX 추가
	if (Upgrade->ProjectileTrailVFX)
	{
		AddProjectileTrailVFX(Upgrade->ProjectileTrailVFX);
	}

	// [신규] 모든 효과 순회하며 적용
	for (const FUpgradeEffect& Effect : Upgrade->Effects)
	{
		switch (Effect.Type)
		{
		case EUpgradeType::Stat_FireRate:
			AttackSpeedBonus += Effect.Value;
			FireRate = FMath::Max(0.05f, BaseFireRate / (1.0f + AttackSpeedBonus));
			break;

		case EUpgradeType::Stat_ProjectileSpeed:
			ProjectileSpeedBonus += Effect.Value;
			break;

		case EUpgradeType::Stat_SpreadAngle:
			SpreadAngleBonus += Effect.Value;
			break;

		case EUpgradeType::Stat_Knockback:
			KnockbackBonus += Effect.Value;
			break;

		case EUpgradeType::Stat_Range:
			RangeBonus += Effect.Value;
			break;

		case EUpgradeType::Stat_ProjectileSize:
			ProjectileSizeBonus += Effect.Value;
			break;

		case EUpgradeType::Stat_ExplosionRadius:
			ExplosionRadiusBonus += Effect.Value;
			break;

		case EUpgradeType::Stat_GaugeMax:
			MaxEggGauge += Effect.Value;
			CurrentEggGauge = FMath::Min(MaxEggGauge, CurrentEggGauge + Effect.Value);
			break;

		case EUpgradeType::Stat_GaugeRecovery:
			GaugeRecoveryRate += Effect.Value;
			break;

		case EUpgradeType::Weapon_Mod_MachineGun:
			bMachineGunEnabled = true;
			// 기관총은 기본 연사 속도를 높여줌 (예: 보너스 50% 즉시 추가)
			AttackSpeedBonus += 0.5f;
			FireRate = FMath::Max(0.05f, BaseFireRate / (1.0f + AttackSpeedBonus));
			break;

		case EUpgradeType::Weapon_Mod_Shotgun:
			bShotgunEnabled = true;
			// 샷건 획득 시 발사 수 즉시 3발로 증가 (기본값)
			MultiShotCount = FMath::Max(MultiShotCount, 3);
			break;

		case EUpgradeType::Weapon_Mod_Piercing:
			bPiercingEnabled = true;
			break;

		case EUpgradeType::Weapon_Mod_Explosive:
			bExplosiveEnabled = true;
			break;

		case EUpgradeType::Weapon_Mod_Sniper:
			bSniperEnabled = true;
			break;

		case EUpgradeType::Weapon_Mod_Flamethrower:
			bFlamethrowerEnabled = true;
			break;

		default:
			break;
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

void UDuckCombatComponent::AddProjectileTrailVFX(UNiagaraSystem* VFX)
{
	if (VFX && !ProjectileTrailVFXs.Contains(VFX))
	{
		ProjectileTrailVFXs.Add(VFX);
	}
}

void UDuckCombatComponent::EndOverheat()
{
	bIsOverheated = false;
	CurrentEggGauge = MaxEggGauge; // 과열 회복 시 게이지 풀 충전 (기획에 따라 조절 가능)
	UE_LOG(LogTemp, Log, TEXT("Overheat Ended. Ready to Fire!"));
}
