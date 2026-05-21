// Fill out your copyright notice in the Description page of Project Settings.

#include "DuckCharacter.h"
#include "DuckPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DuckCombatComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/HealthBarWidget.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Data/UpgradeDataAsset.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Framework/Application/SlateApplication.h"

ADuckCharacter::ADuckCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. 카메라 시스템 설정 (탑뷰)
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 1000.f;
	SpringArmComp->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	SpringArmComp->bDoCollisionTest = false; // 장애물에 의한 카메라 줌 방지
	SpringArmComp->bInheritPitch = false;
	SpringArmComp->bInheritRoll = false;
	SpringArmComp->bInheritYaw = false;

	// 카메라 래그 (Camera Lag) 설정
	SpringArmComp->bEnableCameraLag = true;
	SpringArmComp->bEnableCameraRotationLag = true;
	SpringArmComp->CameraLagSpeed = 5.0f;
	SpringArmComp->CameraRotationLagSpeed = 5.0f;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);

	// 2. 전투 시스템 설정
	CombatComp = CreateDefaultSubobject<UDuckCombatComponent>(TEXT("CombatComponent"));

	// 3. 체력바 UI 설정
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);

	// 4. 캐릭터 회전 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;

	// 초기 체력 설정
	CurrentHealth = MaxHealth;
}

void ADuckCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		PlayerController->bShowMouseCursor = true;
	}

	RefreshHUD();
}

void ADuckCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return;

	bool bShouldLookAtMouse = (CombatComp && CombatComp->IsFiring()) || 
	                          (GetWorld()->GetTimeSeconds() - LastFireTime < 1.0f);

	if (bShouldLookAtMouse)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		LookAtMouseCursor();
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void ADuckCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADuckCharacter::Move);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ADuckCharacter::StartFire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ADuckCharacter::StopFire);
	}
}

float ADuckCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsInvincible || bIsDead || CurrentHealth <= 0.f) return 0.f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.f, MaxHealth);
	
	// 1. 무적 상태
	bIsInvincible = true;
	GetWorldTimerManager().SetTimer(InvincibleTimerHandle, this, &ADuckCharacter::ResetInvincibility, InvincibleDuration, false);

	// 2. 넉백
	if (DamageCauser)
	{
		FVector LaunchDir = GetActorLocation() - DamageCauser->GetActorLocation();
		LaunchDir.Z = 0.f;
		LaunchDir.Normalize();
		LaunchCharacter(LaunchDir * KnockbackStrength, true, true);
	}

	// 3. 카메라 흔들림 (Client 기반으로 수정)
	if (HitCameraShakeClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->ClientStartCameraShake(HitCameraShakeClass);
		}
	}

	// 4. 연출 이벤트
	OnPlayerHit();
	StartHitFlash();
	RefreshHUD();

	if (CurrentHealth <= 0.f)
	{
		Die();
	}
	else
	{
		PlayHitReactMontage();
	}

	return ActualDamage;
}

void ADuckCharacter::ResetInvincibility() { bIsInvincible = false; }

void ADuckCharacter::Move(const FInputActionValue& Value)
{
	if (bIsDead) return;

	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FVector ForwardDirection(1.f, 0.f, 0.f);
		const FVector RightDirection(0.f, 1.f, 0.f);
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ADuckCharacter::StartFire()
{
	if (bIsDead) return;
	if (CombatComp)
	{
		CombatComp->StartFire();
		LastFireTime = GetWorld()->GetTimeSeconds();
	}
}

void ADuckCharacter::StopFire()
{
	if (CombatComp) CombatComp->StopFire();
}

void ADuckCharacter::LookAtMouseCursor()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		FHitResult TraceHitResult;
		if (PC->GetHitResultUnderCursor(ECC_Visibility, false, TraceHitResult))
		{
			FVector LookTarget = TraceHitResult.ImpactPoint;
			FVector Direction = LookTarget - GetActorLocation();
			Direction.Z = 0.f;
			if (!Direction.IsNearlyZero())
			{
				FRotator TargetRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
				SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), 25.0f));
			}
		}
	}
}

bool ADuckCharacter::IsAlignedWithCursor() const
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return false;
	FHitResult TraceHitResult;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, TraceHitResult))
	{
		FVector LookTarget = TraceHitResult.ImpactPoint;
		FVector TargetDirection = (LookTarget - GetActorLocation());
		TargetDirection.Z = 0.f;
		TargetDirection.Normalize();
		float AngleDegree = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(GetActorForwardVector(), TargetDirection), -1.0f, 1.0f)));
		return AngleDegree < 10.0f;
	}
	return false;
}
void ADuckCharacter::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	// 1. 입력 및 이동 차단
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	// 2. 사격 중지
	if (CombatComp)
	{
		CombatComp->StopFire();
	}

	// 3. UI 및 충돌 처리
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
	
	// 캡슐 충돌 비활성화 (시체에 걸리지 않도록)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 4. 사망 애니메이션 재생 및 종료 대기
	if (DeathMontage)
	{
		float Duration = PlayAnimMontage(DeathMontage);
		if (Duration > 0.f)
		{
			// 애니메이션이 완전히 끝나는 시점에 포즈를 고정
			// (몽타주의 Blend Out 시간이 0일 때 가장 정확함)
			GetWorldTimerManager().SetTimer(DeathTimerHandle, this, &ADuckCharacter::OnDeathAnimationFinished, Duration, false);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Player is DEAD!"));
}

void ADuckCharacter::OnDeathAnimationFinished()
{
	if (GetMesh())
	{
		// 애니메이션 업데이트 중지 및 현재 포즈 고정
		GetMesh()->bPauseAnims = true;
	}
}

void ADuckCharacter::StartHitFlash()
{
	if (!HitOverlayMaterial || !GetMesh()) return;

	FlashCount = 0;
	// 에디터에서 설정한 간격으로 깜빡임 실행
	GetWorldTimerManager().SetTimer(FlashTimerHandle, this, &ADuckCharacter::ToggleFlash, FlashInterval, true);
}

void ADuckCharacter::ToggleFlash()
{
	FlashCount++;
	
	if (FlashCount > MaxFlashCount || bIsDead)
	{
		GetWorldTimerManager().ClearTimer(FlashTimerHandle);
		GetMesh()->SetOverlayMaterial(nullptr);
		return;
	}

	// 홀수 번째는 빨간색 적용, 짝수 번째는 해제
	if (FlashCount % 2 == 1)
	{
		GetMesh()->SetOverlayMaterial(HitOverlayMaterial);
	}
	else
	{
		GetMesh()->SetOverlayMaterial(nullptr);
	}
}


void ADuckCharacter::PlayHitReactMontage() { if (HitReactMontage) PlayAnimMontage(HitReactMontage); }
void ADuckCharacter::PlayDeathMontage() { if (DeathMontage) PlayAnimMontage(DeathMontage); }
void ADuckCharacter::PlayAttackMontage(float InPlayRate, float InBlendTime)
{
	if (AttackMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		FAlphaBlend BlendInSettings;
		BlendInSettings.SetBlendTime(InBlendTime);
		GetMesh()->GetAnimInstance()->Montage_PlayWithBlendIn(AttackMontage, BlendInSettings, InPlayRate);
	}
}

void ADuckCharacter::RefreshHUD()
{
	if (ADuckPlayerController* PC = Cast<ADuckPlayerController>(GetController()))
	{
		PC->UpdateHUDHealth(CurrentHealth, MaxHealth);
		PC->UpdateHUDXP(Level, CurrentExp, ExpToNextLevel);
		PC->UpdateHUDGold(Gold);
	}
	if (HealthBarWidget)
	{
		UHealthBarWidget* HPWidget = Cast<UHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
		if (HPWidget) HPWidget->UpdateHealthPercent(GetHealthPercent());
	}
}

void ADuckCharacter::AddExp(float Amount)
{
	CurrentExp += Amount;
	while (CurrentExp >= ExpToNextLevel) LevelUp();
	RefreshHUD();
}

void ADuckCharacter::LevelUp()
{
	CurrentExp -= ExpToNextLevel;
	Level++;
	ExpToNextLevel *= 1.2f;

	// HUD 즉시 갱신 (레벨 숫자 반영)
	RefreshHUD();

	// 1. 게임 일시 정지
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	// 2. 마우스 커서 활성화 및 입력 모드 변경
	if (ADuckPlayerController* PC = Cast<ADuckPlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		PC->SetInputMode(InputMode);
	}

	// 3. 업그레이드 후보군 랜덤 선택 (3개)
	TArray<UUpgradeDataAsset*> Options;
	TArray<UUpgradeDataAsset*> CandidatePool;

	// 매 5레벨 마다 무기 강제 등장 여부 확인 (2, 3, 4레벨은 스탯, 5레벨은 무기)
	bool bIsMilestoneLevel = (Level % 5 == 0);

	UE_LOG(LogTemp, Warning, TEXT("--- Level Up Debug Start (Level: %d, Milestone: %s) ---"), Level, bIsMilestoneLevel ? TEXT("TRUE") : TEXT("FALSE"));

	for (UUpgradeDataAsset* Asset : UpgradePool)
	{
		if (!Asset) continue;

		// ID가 비어있으면 에셋 이름으로 자동 할당 (중요: 중복 방지)
		FName FinalID = Asset->UpgradeID.IsNone() ? Asset->GetFName() : Asset->UpgradeID;
		bool bOwned = AppliedUpgradeLevels.Contains(FinalID);

		if (bIsMilestoneLevel)
		{
			// 마일스톤 레벨: 아직 '보유하지 않은' 새로운 무기만 후보군에 추가
			if (Asset->bIsWeapon && !bOwned)
			{
				CandidatePool.Add(Asset);
			}
		}
		else
		{
			// 일반 레벨: 
			// 1. 무기가 아닌 스탯 카드(bIsWeapon=false)
			// 2. 이미 보유하고 있는 무기의 강화 카드(bOwned=true)
			if (!Asset->bIsWeapon || bOwned)
			{
				// 최대 레벨 도달 확인
				const int32* CurrentLvlPtr = AppliedUpgradeLevels.Find(FinalID);
				int32 CurrentLvl = CurrentLvlPtr ? *CurrentLvlPtr : 0;
				if (CurrentLvl >= Asset->MaxLevel) continue;

				// 선행 요구 사항 확인
				if (!Asset->RequiredUpgradeID.IsNone())
				{
					const int32* ReqLvlPtr = AppliedUpgradeLevels.Find(Asset->RequiredUpgradeID);
					int32 ReqLvl = ReqLvlPtr ? *ReqLvlPtr : 0;
					if (ReqLvl < Asset->RequiredLevel) continue;
				}

				CandidatePool.Add(Asset);
			}
		}
	}

	// [Fallback] 마일스톤 레벨인데 뽑을 새로운 무기가 하나도 없다면, 일반 업그레이드라도 보여줌
	if (bIsMilestoneLevel && CandidatePool.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No new weapons available for Milestone! Falling back to normal upgrades."));
		for (UUpgradeDataAsset* Asset : UpgradePool)
		{
			if (!Asset || Asset->bIsWeapon) continue;
			
			FName FinalID = Asset->UpgradeID.IsNone() ? Asset->GetFName() : Asset->UpgradeID;
			const int32* CurrentLvlPtr = AppliedUpgradeLevels.Find(FinalID);
			int32 CurrentLvl = CurrentLvlPtr ? *CurrentLvlPtr : 0;
			if (CurrentLvl >= Asset->MaxLevel) continue;

			CandidatePool.AddUnique(Asset);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Candidate Pool Size: %d"), CandidatePool.Num());

	// 가중치 기반 랜덤 선택 (3개 추출)
	while (Options.Num() < 3 && CandidatePool.Num() > 0)
	{
		float TotalWeight = 0.0f;
		for (UUpgradeDataAsset* Asset : CandidatePool) TotalWeight += Asset->Weight;

		float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
		float WeightSum = 0.0f;

		for (int32 i = 0; i < CandidatePool.Num(); ++i)
		{
			WeightSum += CandidatePool[i]->Weight;
			if (RandomValue <= WeightSum)
			{
				Options.Add(CandidatePool[i]);
				CandidatePool.RemoveAt(i);
				break;
			}
		}
	}

	// 4. UI 이벤트 호출
	OnShowUpgradeScreen(Options);
	UE_LOG(LogTemp, Warning, TEXT("--- Level Up Debug End ---"));
}

void ADuckCharacter::SelectUpgrade(UUpgradeDataAsset* SelectedUpgrade)
{
	if (SelectedUpgrade)
	{
		ApplyUpgrade(SelectedUpgrade);
	}

	// 1. 게임 일시 정지 해제
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	// 2. 입력 모드 복구 및 마우스 설정 (트윈스틱 슈팅 최적화)
	if (ADuckPlayerController* PC = Cast<ADuckPlayerController>(GetController()))
	{
		FInputModeGameAndUI InputMode;
		// 마우스 캡처 시에도 커서를 숨기지 않음 (중요!)
		InputMode.SetHideCursorDuringCapture(false);
		// 마우스를 뷰포트에 가두지 않거나 게임 설정에 따름
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
		
		// 슬레이트(UI) 레벨에서도 포커스를 게임 뷰포트로 강제 이동
		FSlateApplication::Get().SetAllUserFocusToGameViewport();
	}
}

void ADuckCharacter::ApplyUpgrade(UUpgradeDataAsset* Upgrade)
{
	if (!Upgrade) return;

	// ID가 비어있으면 에셋 이름으로 자동 할당 (LevelUp 로직과 일치시킴)
	FName FinalID = Upgrade->UpgradeID.IsNone() ? Upgrade->GetFName() : Upgrade->UpgradeID;

	// 1. 업그레이드 단계 기록
	int32& CurrentLvl = AppliedUpgradeLevels.FindOrAdd(FinalID);
	CurrentLvl++;

	bool bIsFirstTime = (CurrentLvl == 1);

	UE_LOG(LogTemp, Warning, TEXT("Applying Upgrade: %s (Level %d, ID: %s)"), *Upgrade->UpgradeName.ToString(), CurrentLvl, *FinalID.ToString());

	// 2. 모든 효과 순회하며 적용
	for (const FUpgradeEffect& Effect : Upgrade->Effects)
	{
		switch (Effect.Type)
		{
		case EUpgradeType::Stat_MaxHealth:
			// 최대 체력 절대치 증가 및 현재 체력 동일 수치 회복
			MaxHealth += Effect.Value;
			CurrentHealth = FMath::Clamp(CurrentHealth + Effect.Value, 0.f, MaxHealth);
			UE_LOG(LogTemp, Warning, TEXT("Max Health Increased: +%.f (Total: %.f), Current Health recovered to: %.f"), Effect.Value, MaxHealth, CurrentHealth);
			break;

		case EUpgradeType::Stat_MoveSpeed:
			MoveSpeedBonus += Effect.Value;
			UpdateMoveSpeed();
			break;

		case EUpgradeType::Stat_AttackDamage:
			// 공격력 보너스 누적 (예: 0.2면 20% 증가)
			AttackDamageBonus += Effect.Value;
			UE_LOG(LogTemp, Warning, TEXT("Attack Damage Bonus: %.1f%%, Total Damage: %.2f"), AttackDamageBonus * 100.f, GetCurrentAttackDamage());
			break;

		// 전투 관련 모든 강화(주무기 변이 포함)는 CombatComp에서 처리
		case EUpgradeType::Stat_FireRate:
		case EUpgradeType::Stat_ProjectileSpeed:
		case EUpgradeType::Stat_SpreadAngle:
		case EUpgradeType::Stat_GaugeMax:
		case EUpgradeType::Stat_GaugeRecovery:
		case EUpgradeType::Weapon_Mod_MachineGun:
		case EUpgradeType::Weapon_Mod_Shotgun:
		case EUpgradeType::Weapon_Mod_Piercing:
		case EUpgradeType::Weapon_Mod_Explosive:
		case EUpgradeType::Weapon_Mod_Sniper:
		case EUpgradeType::Weapon_Mod_Flamethrower:
			if (CombatComp) CombatComp->ApplyUpgrade(Upgrade, bIsFirstTime);
			break;

		// 패시브 무기 설치
		case EUpgradeType::Weapon_Passive_Orbit:
		case EUpgradeType::Weapon_Passive_AutoBomb:
		case EUpgradeType::Weapon_Passive_Molotov:
			if (Upgrade->SpecialActorClass && bIsFirstTime) // 패시브 무기 액터도 처음 획득 시에만 생성
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				GetWorld()->SpawnActor<AActor>(Upgrade->SpecialActorClass, GetActorLocation(), GetActorRotation(), SpawnParams);
			}
			break;
		}
	}

	// 3. 연출 (VFX & Sound)
	
	// 일회성 획득 VFX (매 레벨업 시 발생)
	if (Upgrade->UpgradeVFX)
	{
		PlayUpgradeVFX(Upgrade->UpgradeVFX);
	}

	// 플레이어 지속 VFX (처음 획득 시에만 생성)
	if (bIsFirstTime)
	{
		for (UNiagaraSystem* PersistentVFX : Upgrade->PlayerPersistentVFXs)
		{
			if (!PersistentVFX) continue;

			UNiagaraComponent* NewVFXComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
				PersistentVFX, GetMesh(), NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
			
			if (NewVFXComp)
			{
				// 여러 개일 수 있으므로 ID에 인덱스를 붙여 저장하거나 그냥 관리 (중복 제거는 이미 bIsFirstTime으로 보장됨)
				PersistentVFXComponents.Add(FName(*FString::Printf(TEXT("%s_%d"), *FinalID.ToString(), PersistentVFXComponents.Num())), NewVFXComp);
			}
		}
	}

	// 발사체 트레일 VFX는 CombatComp에서 처리 (ApplyUpgrade 내부에서 이미 호출됨)
	
	if (Upgrade->UpgradeSound)
	{
		UGameplayStatics::PlaySound2D(this, Upgrade->UpgradeSound);
	}

	RefreshHUD();
}

void ADuckCharacter::PlayUpgradeVFX(UNiagaraSystem* VFX)
{
	if (VFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), VFX, GetActorLocation());
	}
}

void ADuckCharacter::AddGold(int32 Amount)
{
	Gold += Amount;
	RefreshHUD();
}

void ADuckCharacter::UpdateMoveSpeed()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = (BaseMoveSpeed * WeaponMoveSpeedMultiplier) * (1.0f + MoveSpeedBonus);
		UE_LOG(LogTemp, Warning, TEXT("Move Speed Recalculated: %.2f (WeaponMult: %.2f, Bonus: %.1f%%)"), 
			GetCharacterMovement()->MaxWalkSpeed, WeaponMoveSpeedMultiplier, MoveSpeedBonus * 100.f);
	}
}
