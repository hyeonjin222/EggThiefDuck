// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DropItemBase.h"
#include "AmmoItem.generated.h"

/**
 * 탄약(달걀 게이지) 회복 아이템 클래스
 */
UCLASS()
class EGGTHIEFDUCK_API AAmmoItem : public ADropItemBase
{
	GENERATED_BODY()

public:
	AAmmoItem();

protected:
	virtual void OnPickedUp_Implementation(AActor* Deliverer) override;

	/** 획득 시 회복할 게이지 양 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Ammo")
	float AmmoRestoreAmount = 100.0f;
};
