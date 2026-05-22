// Fill out your copyright notice in the Description page of Project Settings.

#include "StartGameTrigger.h"
#include "Components/SphereComponent.h"
#include "DuckCharacter.h"
#include "DuckGameMode.h"
#include "Kismet/GameplayStatics.h"

AStartGameTrigger::AStartGameTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetSphereRadius(100.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Trigger"));
}

void AStartGameTrigger::BeginPlay()
{
	Super::BeginPlay();
	
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AStartGameTrigger::OnOverlapBegin);
}

void AStartGameTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ADuckCharacter* Player = Cast<ADuckCharacter>(OtherActor))
	{
		if (ADuckGameMode* GM = Cast<ADuckGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			GM->StartGame();
			
			// 트리거는 한 번만 작동하면 되므로 파괴하거나 비활성화
			Destroy();
		}
	}
}
