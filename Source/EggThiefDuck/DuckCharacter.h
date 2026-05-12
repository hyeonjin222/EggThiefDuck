// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DuckCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class EGGTHIEFDUCK_API ADuckCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ADuckCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/** 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* CameraComp;

	/** 입력 에셋 */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** 이동 로직 */
	void Move(const FInputActionValue& Value);

	/** 마우스 조준 로직 */
	void LookAtMouseCursor();

	/** 캐릭터 이동 속도 */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveSpeed = 600.f;
};
