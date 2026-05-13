// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageTextActor.generated.h"

class UWidgetComponent;

UCLASS()
class EGGTHIEFDUCK_API ADamageTextActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ADamageTextActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/** 텍스트 설정 및 데미지 값 전달 */
	void SetDamageValue(float DamageAmount);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> DamageWidget;

	/** 위로 떠오르는 속도 */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float FloatSpeed = 100.0f;

	/** 수명 (자동 파괴 시간) */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float LifeTime = 0.8f;

	float ElapsedTime = 0.0f;
};
