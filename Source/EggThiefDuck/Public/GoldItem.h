// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DropItemBase.h"
#include "GoldItem.generated.h"

/**
 * 재화(골드) 아이템 클래스
 */
UCLASS()
class EGGTHIEFDUCK_API AGoldItem : public ADropItemBase
{
	GENERATED_BODY()

public:
	AGoldItem();

protected:
	virtual void OnPickedUp_Implementation(AActor* Deliverer) override;

	/** 획득 시 증가할 골드 양 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Gold")
	int32 GoldAmount = 10;
};
