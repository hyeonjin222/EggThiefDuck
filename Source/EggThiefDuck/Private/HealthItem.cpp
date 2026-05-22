// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthItem.h"
#include "DuckCharacter.h"

AHealthItem::AHealthItem()
{
	HealPercent = 0.1f;
}

void AHealthItem::OnPickedUp_Implementation(AActor* Deliverer)
{
	Super::OnPickedUp_Implementation(Deliverer);

	ADuckCharacter* Player = Cast<ADuckCharacter>(Deliverer);
	if (Player)
	{
		Player->Heal(HealPercent);
	}
}
