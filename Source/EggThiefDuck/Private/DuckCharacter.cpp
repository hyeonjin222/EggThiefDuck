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

	// 카메라 래그 (Camera Lag) 설정: 부드럽게 따라오게 함
	SpringArmComp->bEnableCameraLag = true;
	SpringArmComp->bEnableCameraRotationLag = true;
	SpringArmComp->CameraLagSpeed = 5.0f;         // 이동 지연 속도 (낮을수록 더 부드럽고 느림)
	SpringArmComp->CameraRotationLagSpeed = 5.0f; // 회전 지연 속도

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);

	// 2. 전투 시스템 설정
	CombatComp = CreateDefaultSubobject<UDuckCombatComponent>(TEXT("CombatComponent"));

	// 3. 체력바 UI 설정
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f)); // 캐릭터 머리 위
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);          // 화면 공간 UI

	// 4. 캐릭터 회전 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = false; // 이동 방향으로 자동 회전 끔 (조준을 위해)
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	// 초기 체력 설정
	CurrentHealth = MaxHealth;
}

void ADuckCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Enhanced Input Context 추가
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		
		// 마우스 커서 표시 및 감춤 설정
		PlayerController->bShowMouseCursor = true;
	}

	RefreshHUD();
}

void ADuckCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 조건부 회전 로직 (사격 중이거나 사격 후 1.0초 동안 유지)
	bool bShouldLookAtMouse = (CombatComp && CombatComp->IsFiring()) || 
	                          (GetWorld()->GetTimeSeconds() - LastFireTime < 1.0f);

	if (bShouldLookAtMouse)
	{
		// 마우스 방향으로 부드럽고 빠르게 조준
		GetCharacterMovement()->bOrientRotationToMovement = false;
		LookAtMouseCursor();
	}
	else
	{
		// 평소: 이동 방향 바라보기
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void ADuckCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADuckCharacter::Move);
		
		// 사격 바인딩
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ADuckCharacter::StartFire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ADuckCharacter::StopFire);
	}
}

float ADuckCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.f, MaxHealth);
	
	RefreshHUD();

	if (CurrentHealth <= 0.f)
	{
		Die();
	}
	else
	{
		// 살아있다면 피격 애니메이션 재생
		PlayHitReactMontage();
	}

	return ActualDamage;
}

void ADuckCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 카메라 기준이 아닌 월드 절대 좌표 기준으로 이동 (탑뷰 전형 방식)
		const FVector ForwardDirection(1.f, 0.f, 0.f);
		const FVector RightDirection(0.f, 1.f, 0.f);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ADuckCharacter::StartFire()
{
	if (CombatComp)
	{
		CombatComp->StartFire();
		// 사격 시작 시 타이머 초기화 (조준 유지 시작)
		LastFireTime = GetWorld()->GetTimeSeconds();
	}
}

void ADuckCharacter::StopFire()
{
	if (CombatComp)
	{
		CombatComp->StopFire();
	}
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
				FRotator CurrentRotation = GetActorRotation();

				// 즉시 회전 대신 RInterpTo를 사용하여 매우 빠르지만 부드럽게 회전
				FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 25.0f);
				SetActorRotation(NewRotation);
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
		FVector TargetDirection = LookTarget - GetActorLocation();
		TargetDirection.Z = 0.f;
		TargetDirection.Normalize();

		FVector CurrentForward = GetActorForwardVector();
		
		// 현재 정면과 조준 방향의 각도 차이 계산 (내적 활용)
		float DotProduct = FVector::DotProduct(CurrentForward, TargetDirection);
		// 아크코사인을 이용해 각도(라디안) 계산 후 도(Degree)로 변환
		float AngleDegree = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));

		// 10도 이내면 정렬된 것으로 간주 (정밀도 조절 가능)
		return AngleDegree < 10.0f;
	}

	return false;
}

void ADuckCharacter::Die()
{
	// 사망 애니메이션 재생
	PlayDeathMontage();

	UE_LOG(LogTemp, Warning, TEXT("Player is DEAD!"));
	
	// TODO: 사망 연출 후 레벨 재시작 (타이머 등 활용 권장)
	// UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}

void ADuckCharacter::PlayHitReactMontage()
{
	if (HitReactMontage)
	{
		PlayAnimMontage(HitReactMontage);
	}
}

void ADuckCharacter::PlayDeathMontage()
{
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
}

void ADuckCharacter::PlayAttackMontage(float InPlayRate, float InBlendTime)
{
	if (AttackMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
		
		// 1. 블렌드 설정 객체 생성 및 시간 설정
		FAlphaBlend BlendInSettings;
		BlendInSettings.SetBlendTime(InBlendTime);
		
		// 2. Montage_PlayWithBlendIn을 사용하여 재생 (블렌드 인 시간을 동적으로 덮어씌움)
		// 이 함수는 연사 시 이전 몽타주를 블렌딩하며 자연스럽게 교체해줍니다.
		AnimInst->Montage_PlayWithBlendIn(AttackMontage, BlendInSettings, InPlayRate);
	}
}

void ADuckCharacter::RefreshHUD()
{
	// 1. 메인 화면 UI 업데이트 (PlayerController 경유)
	if (ADuckPlayerController* PC = Cast<ADuckPlayerController>(GetController()))
	{
		PC->UpdateHUDHealth(CurrentHealth, MaxHealth);
		PC->UpdateHUDXP(Level, CurrentExp, ExpToNextLevel);
		PC->UpdateHUDGold(Gold);
	}

	// 2. 머리 위 체력바 업데이트
	if (HealthBarWidget)
	{
		UHealthBarWidget* HPWidget = Cast<UHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
		if (HPWidget)
		{
			HPWidget->UpdateHealthPercent(GetHealthPercent());
		}
	}
}

void ADuckCharacter::AddExp(float Amount)
{
	CurrentExp += Amount;

	// 레벨업 체크
	while (CurrentExp >= ExpToNextLevel)
	{
		LevelUp();
	}

	RefreshHUD();
}

void ADuckCharacter::LevelUp()
{
	CurrentExp -= ExpToNextLevel;
	Level++;

	// 다음 레벨 요구치 상승 (예: 레벨당 20% 증가)
	ExpToNextLevel *= 1.2f;

	UE_LOG(LogTemp, Warning, TEXT("Level Up! Current Level: %d"), Level);

	// TODO: 게임 일시정지 및 강화 선택 UI 팝업 로직 추가 예정
}

void ADuckCharacter::AddGold(int32 Amount)
{
	Gold += Amount;
	RefreshHUD();
}
