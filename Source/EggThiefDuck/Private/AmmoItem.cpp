// Fill out your copyright notice in the Description page of Project Settings.

#include "AmmoItem.h"
#include "DuckCharacter.h"
#include "DuckCombatComponent.h"

AAmmoItem::AAmmoItem()
{
	AmmoRestoreAmount = 100.0f;
}

void AAmmoItem::OnPickedUp_Implementation(AActor* Deliverer)
{
	Super::OnPickedUp_Implementation(Deliverer);

	ADuckCharacter* Player = Cast<ADuckCharacter>(Deliverer);
	if (Player && Player->GetCombatComponent())
	{
		Player->GetCombatComponent()->RefillGauge(AmmoRestoreAmount);
		
		UE_LOG(LogTemp, Log, TEXT("Ammo Picked Up! Restored: %f"), AmmoRestoreAmount);
	}
}
