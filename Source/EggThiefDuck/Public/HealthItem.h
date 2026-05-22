// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DropItemBase.h"
#include "HealthItem.generated.h"

/**
 * 체력 회복 아이템 클래스
 */
UCLASS()
class EGGTHIEFDUCK_API AHealthItem : public ADropItemBase
{
	GENERATED_BODY()

public:
	AHealthItem();

protected:
	virtual void OnPickedUp_Implementation(AActor* Deliverer) override;

	/** 획득 시 회복할 체력 비율 (0.1 = 10%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Health")
	float HealPercent = 0.1f;
};
