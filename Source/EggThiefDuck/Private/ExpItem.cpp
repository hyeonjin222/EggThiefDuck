// Fill out your copyright notice in the Description page of Project Settings.

#include "ExpItem.h"
#include "DuckCharacter.h"

AExpItem::AExpItem()
{
	// 기본값 설정
	ExpAmount = 20.0f;
}

void AExpItem::OnPickedUp_Implementation(AActor* Deliverer)
{
	Super::OnPickedUp_Implementation(Deliverer);

	ADuckCharacter* Player = Cast<ADuckCharacter>(Deliverer);
	if (Player)
	{
		Player->AddExp(ExpAmount);
		
		UE_LOG(LogTemp, Log, TEXT("XP Picked Up! Amount: %f, Current Level: %d"), ExpAmount, Player->GetCharacterLevel());
	}
}
