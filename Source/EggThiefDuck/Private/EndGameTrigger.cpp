// Fill out your copyright notice in the Description page of Project Settings.

#include "EndGameTrigger.h"
#include "Components/SphereComponent.h"
#include "DuckCharacter.h"
#include "DuckPlayerController.h"
#include "DuckGameMode.h"
#include "Kismet/GameplayStatics.h"

AEndGameTrigger::AEndGameTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetSphereRadius(200.0f); // 탈출 구역은 조금 더 크게
	CollisionComponent->SetCollisionProfileName(TEXT("Trigger"));
}

void AEndGameTrigger::BeginPlay()
{
	Super::BeginPlay();
	
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AEndGameTrigger::OnOverlapBegin);
}

void AEndGameTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ADuckCharacter* Player = Cast<ADuckCharacter>(OtherActor))
	{
		// 1. 게임 종료 로그 출력
		UE_LOG(LogTemp, Log, TEXT("EndGameTrigger: Player reached escape zone. Quitting Game..."));

		// 2. 플레이어 컨트롤러를 통해 즉시 게임 종료(프로그램 종료) 실행
		if (ADuckPlayerController* PC = Cast<ADuckPlayerController>(Player->GetController()))
		{
			PC->QuitGame();
		}
		
		// 트리거 파괴
		Destroy();
	}
}
