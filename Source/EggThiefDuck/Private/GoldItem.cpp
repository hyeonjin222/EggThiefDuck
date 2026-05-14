// Fill out your copyright notice in the Description page of Project Settings.

#include "GoldItem.h"
#include "DuckCharacter.h"

AGoldItem::AGoldItem()
{
	// 기본값 설정
	GoldAmount = 10;
}

void AGoldItem::OnPickedUp_Implementation(AActor* Deliverer)
{
	Super::OnPickedUp_Implementation(Deliverer);

	ADuckCharacter* Player = Cast<ADuckCharacter>(Deliverer);
	if (Player)
	{
		Player->AddGold(GoldAmount);
		
		UE_LOG(LogTemp, Log, TEXT("Gold Picked Up! Amount: %d, Total: %d"), GoldAmount, Player->GetGold());
	}
}
