// Fill out your copyright notice in the Description page of Project Settings.

#include "DuckCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DuckCombatComponent.h"

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

	// 3. 캐릭터 회전 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = false; // 이동 방향으로 자동 회전 끔 (조준을 위해)
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
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
}

void ADuckCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LookAtMouseCursor();
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
		// 마우스 위치에서 바닥으로 레이캐스트
		if (PC->GetHitResultUnderCursor(ECC_Visibility, false, TraceHitResult))
		{
			FVector LookTarget = TraceHitResult.ImpactPoint;
			FVector Direction = LookTarget - GetActorLocation();
			Direction.Z = 0.f; // 높이 차이 무시

			FRotator LookAtRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
			SetActorRotation(LookAtRotation);
		}
	}
}
