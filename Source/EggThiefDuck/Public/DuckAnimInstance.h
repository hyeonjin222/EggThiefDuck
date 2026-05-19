// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DuckAnimInstance.generated.h"

/**
 * 오리 캐릭터의 애니메이션 로직을 담당하는 C++ 베이스 클래스
 */
UCLASS()
class EGGTHIEFDUCK_API UDuckAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

protected:
	/** 현재 이동 속도 */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed;

	/** 이동 방향 (뒷걸음질 처리를 위함, -180 ~ 180) */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction;

	/** 현재 사격(조준) 중인지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsFiring;

	/** 사망 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	bool bIsDead;

	/** 공중에 떠 있는지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;

private:
	UPROPERTY()
	TObjectPtr<class ADuckCharacter> DuckCharacter;
};
