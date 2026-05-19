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
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

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

	// 1. 게임 일시 정지
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	// 2. 마우스 커서 활성화 및 입력 모드 변경
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		PC->SetInputMode(InputMode);
	}

	// 3. 업그레이드 후보군 랜덤 선택 (3개)
	TArray<UUpgradeDataAsset*> Options;
	TArray<TObjectPtr<UUpgradeDataAsset>> Pool = UpgradePool;

	// 최대 레벨에 도달하지 않은 업그레이드만 필터링
	Pool.RemoveAll([this](const TObjectPtr<UUpgradeDataAsset>& Asset) {
		if (!Asset) return true;
		const int32* CurrentLvl = AppliedUpgradeLevels.Find(Asset->UpgradeID);
		return CurrentLvl && *CurrentLvl >= Asset->MaxLevel;
	});

	// 랜덤하게 3개 추출
	while (Options.Num() < 3 && Pool.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, Pool.Num() - 1);
		Options.Add(Pool[Index]);
		Pool.RemoveAt(Index);
	}

	// 4. UI 이벤트 호출 (블루프린트에서 UI 생성 및 데이터 전달)
	OnShowUpgradeScreen(Options);

	UE_LOG(LogTemp, Warning, TEXT("Level Up! Current Level: %d"), Level);
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
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
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

	// 1. 업그레이드 단계 기록
	int32& CurrentLvl = AppliedUpgradeLevels.FindOrAdd(Upgrade->UpgradeID);
	CurrentLvl++;

	UE_LOG(LogTemp, Warning, TEXT("Applying Upgrade: %s (Level %d)"), *Upgrade->UpgradeName.ToString(), CurrentLvl);

	// 2. 스탯 및 기능 적용
	switch (Upgrade->UpgradeType)
	{
	case EUpgradeType::Stat_MaxHealth:
		MaxHealth += Upgrade->Value;
		CurrentHealth = FMath::Clamp(CurrentHealth + Upgrade->Value, 0.f, MaxHealth);
		break;
	case EUpgradeType::Stat_MoveSpeed:
		MoveSpeed += Upgrade->Value;
		GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
		break;
	case EUpgradeType::Stat_AttackDamage:
		BaseDamage += Upgrade->Value;
		break;
	case EUpgradeType::Stat_FireRate:
	case EUpgradeType::Stat_GaugeMax:
	case EUpgradeType::Stat_GaugeRecovery:
	case EUpgradeType::Mech_MultiShot:
	case EUpgradeType::Mech_Piercing:
	case EUpgradeType::Mech_Explosive:
		if (CombatComp) CombatComp->ApplyUpgrade(Upgrade);
		break;
	case EUpgradeType::Special_OrbitingEgg:
	case EUpgradeType::Special_EggMine:
		if (Upgrade->SpecialActorClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			GetWorld()->SpawnActor<AActor>(Upgrade->SpecialActorClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		}
		break;
	}

	// 3. 연출 (VFX & Sound)
	if (Upgrade->UpgradeVFX)
	{
		PlayUpgradeVFX(Upgrade->UpgradeVFX);
	}
	
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
