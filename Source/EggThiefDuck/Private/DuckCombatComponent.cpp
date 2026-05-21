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

	MaxEggGauge = BaseMaxEggGauge;
	GaugeRecoveryRate = BaseGaugeRecoveryRate;
	CurrentEggGauge = MaxEggGauge;
}

void UDuckCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// 최초 설정된 연사력을 기준값으로 저장
	BaseFireRate = FireRate;

	// 최종 스탯 초기화
	MaxEggGauge = BaseMaxEggGauge;
	GaugeRecoveryRate = BaseGaugeRecoveryRate;
	CurrentEggGauge = MaxEggGauge;
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

void UDuckCombatComponent::UpdateFinalStats()
{
	ADuckCharacter* OwnerCharacter = Cast<ADuckCharacter>(GetOwner());

	// 공속 계산: (기본 * 무기배율) / (1 + 강화수치)
	FireRate = FMath::Max(0.05f, (BaseFireRate * WeaponFireRateMultiplier) / (1.0f + AttackSpeedBonus));

	// 게이지 계산: (기본 * 무기배율) * (1 + 강화수치)
	float OldMax = MaxEggGauge;
	MaxEggGauge = (BaseMaxEggGauge * WeaponMaxEggGaugeMultiplier) * (1.0f + MaxEggGaugeBonus);
	GaugeRecoveryRate = (BaseGaugeRecoveryRate * WeaponGaugeRecoveryMultiplier) * (1.0f + GaugeRecoveryBonus);

	// 관통 횟수 계산: (기본 + 보너스) * 무기배율
	MaxPiercingCount = FMath::RoundToInt((BaseMaxPiercingCount + PiercingCountBonus) * WeaponPiercingCountMultiplier);
	// 관통 모드가 활성화되어 있는데 횟수가 0이면 최소 1회 보장
	if ((bPiercingEnabled || bSniperEnabled) && MaxPiercingCount <= 0)
	{
		MaxPiercingCount = 1;
	}

	// 최대치가 바뀌면 현재 게이지도 비율에 맞춰 보정 (또는 차이만큼 더해줌)
	if (OldMax > 0.f)
	{
		CurrentEggGauge += (MaxEggGauge - OldMax);
		CurrentEggGauge = FMath::Clamp(CurrentEggGauge, 0.f, MaxEggGauge);
	}

	// 캐릭터 무기 데미지/이속 배율 전달
	if (OwnerCharacter)
	{
		OwnerCharacter->UpdateMoveSpeed();
	}
	
	UE_LOG(LogTemp, Log, TEXT("Combat Stats Recalculated - FireRate: %.3f, MaxGauge: %.1f, Piercing: %d"), 
		FireRate, MaxEggGauge, MaxPiercingCount);
}

void UDuckCombatComponent::FireProjectile()
{
	ADuckCharacter* OwnerCharacter = Cast<ADuckCharacter>(GetOwner());
	if (OwnerCharacter && ProjectileClass)
	{
		// --- 변이 시너지 파라미터 최종 계산 ---
		// 공식: (기본 * 무기배율) * (1 + 강화수치)
		float FinalSpeed = (BaseProjectileSpeed * WeaponProjectileSpeedMultiplier) * (1.0f + ProjectileSpeedBonus);
		float FinalLifeSpan = (BaseLifeSpan * WeaponRangeMultiplier) * (1.0f + RangeBonus);
		FVector FinalScale = FVector(1.0f * WeaponProjectileSizeMultiplier) * (1.0f + ProjectileSizeBonus);
		
		// 확산 각도: 기본 무기 베이스에 보너스 더하기
		float FinalSpreadAngle = WeaponSpreadAngleBase + SpreadAngleBonus;

		// 저격총 특수 스케일 비율
		if (bSniperEnabled)
		{
			FinalScale *= FVector(3.0f, 0.2f, 0.2f);
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
				Projectile->SetSpeed(FinalSpeed);
				Projectile->SetLifeSpan(FinalLifeSpan);
				Projectile->SetActorScale3D(FinalScale);

				Projectile->SetDamage(OwnerCharacter->GetCurrentAttackDamage());
				Projectile->SetKnockbackBonus(KnockbackBonus);
				Projectile->SetExplosionRadiusBonus(ExplosionRadiusBonus);
				
				// [수정] 계산된 관통 횟수 적용
				Projectile->SetPiercing(MaxPiercingCount > 0);
				Projectile->SetMaxPiercingCount(MaxPiercingCount);
				
				Projectile->SetExplosive(bExplosiveEnabled);
				
				for (UNiagaraSystem* TrailVFX : ProjectileTrailVFXs)
				{
					Projectile->AddTrailVFX(TrailVFX);
				}

				Projectile->FireInDirection(SpawnRotation.Vector());
			}
		}

		CurrentEggGauge -= GaugeCostPerShot;
		bWantsToFire = false;
	}
}

void UDuckCombatComponent::ApplyUpgrade(UUpgradeDataAsset* Upgrade, bool bIsFirstTime)
{
	if (!Upgrade) return;

	// 처음 획득할 때만 발사체 트레일 VFX 추가
	if (bIsFirstTime)
	{
		for (UNiagaraSystem* TrailVFX : Upgrade->ProjectileTrailVFXs)
		{
			AddProjectileTrailVFX(TrailVFX);
		}
	}

	ADuckCharacter* OwnerCharacter = Cast<ADuckCharacter>(GetOwner());

	for (const FUpgradeEffect& Effect : Upgrade->Effects)
	{
		switch (Effect.Type)
		{
		case EUpgradeType::Stat_FireRate:
			AttackSpeedBonus += Effect.Value;
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
			MaxEggGaugeBonus += Effect.Value;
			break;

		case EUpgradeType::Stat_GaugeRecovery:
			GaugeRecoveryBonus += Effect.Value;
			break;

		case EUpgradeType::Stat_PiercingCount:
			PiercingCountBonus += FMath::RoundToInt(Effect.Value);
			break;

		case EUpgradeType::Weapon_Mod_MachineGun:
			bMachineGunEnabled = true;
			WeaponFireRateMultiplier = 0.5f; // 공속 2배
			WeaponMaxEggGaugeMultiplier = 2.0f; // 최대 탄약 2배
			WeaponGaugeRecoveryMultiplier = 2.0f; // 탄약 회복속도 2배
			if (OwnerCharacter) OwnerCharacter->WeaponDamageMultiplier = 0.7f; // 데미지 배율 0.7
			WeaponSpreadAngleBase = 15.0f;
			break;

		case EUpgradeType::Weapon_Mod_Shotgun:
			bShotgunEnabled = true;
			// 샷건: 산탄 4개 발사
			MultiShotCount = FMath::Max(MultiShotCount, 4);
			// 공속 50% 감소 (간격 2배)
			WeaponFireRateMultiplier = 2.0f;
			// 데미지 30% 감소 (배율 0.7)
			if (OwnerCharacter) OwnerCharacter->WeaponDamageMultiplier = 0.7f;
			
			WeaponRangeMultiplier = 0.15f; 
			WeaponSpreadAngleBase = 30.0f; 
			WeaponProjectileSizeMultiplier = 1.5f;
			break;
		case EUpgradeType::Weapon_Mod_Sniper:
			bSniperEnabled = true;
			bPiercingEnabled = false; // 관통 롤백
			// 저격총: 공속 50% 감소 (간격 2.0배)
			WeaponFireRateMultiplier = 2.0f;
			// 데미지 2배 증가
			if (OwnerCharacter) OwnerCharacter->WeaponDamageMultiplier = 2.0f;
			// 투사체 속도 3배 증가
			WeaponProjectileSpeedMultiplier = 3.0f;
			// 관통 배율 초기화
			WeaponPiercingCountMultiplier = 0.0f;

			WeaponSpreadAngleBase = 0.0f;
			break;

		case EUpgradeType::Weapon_Mod_Flamethrower:
			bFlamethrowerEnabled = true;
			WeaponFireRateMultiplier = 0.2f;
			WeaponProjectileSpeedMultiplier = 0.6f;
			WeaponRangeMultiplier = 0.2f;
			WeaponProjectileSizeMultiplier = 2.5f;
			WeaponSpreadAngleBase = 40.0f;
			if (OwnerCharacter) OwnerCharacter->WeaponDamageMultiplier = 0.3f;
			break;

		case EUpgradeType::Weapon_Mod_Piercing:
			bPiercingEnabled = true;
			break;

		case EUpgradeType::Weapon_Mod_Explosive:
			bExplosiveEnabled = true;
			break;

		default:
			break;
		}
	}

	UpdateFinalStats();
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
