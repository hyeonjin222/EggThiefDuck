// Fill out your copyright notice in the Description page of Project Settings.

#include "DuckAnimInstance.h"
#include "DuckCharacter.h"
#include "DuckCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UDuckAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 캐릭터 참조 가져오기
	DuckCharacter = Cast<ADuckCharacter>(TryGetPawnOwner());
}

void UDuckAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!DuckCharacter)
	{
		DuckCharacter = Cast<ADuckCharacter>(TryGetPawnOwner());
	}

	if (!DuckCharacter) return;

	// 1. 속도 계산
	FVector Velocity = DuckCharacter->GetVelocity();
	Speed = Velocity.Size2D();

	// 2. 이동 방향 계산 (몸이 바라보는 방향 기준 상대적 이동 방향)
	// KismetAnimationLibrary를 사용하여 -180 ~ 180도 사이의 방향값을 구함
	Direction = CalculateDirection(Velocity, DuckCharacter->GetActorRotation());

	// 3. 사격(조준) 상태 가져오기
	if (UDuckCombatComponent* Combat = DuckCharacter->GetCombatComponent())
	{
		bIsFiring = Combat->IsFiring();
	}

	// 4. 점프/낙하 상태
	if (UCharacterMovementComponent* Movement = DuckCharacter->GetCharacterMovement())
	{
		bIsFalling = Movement->IsFalling();
	}
}
