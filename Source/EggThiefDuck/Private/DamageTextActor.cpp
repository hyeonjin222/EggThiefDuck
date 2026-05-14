// Fill out your copyright notice in the Description page of Project Settings.

#include "DamageTextActor.h"
#include "Components/WidgetComponent.h"

ADamageTextActor::ADamageTextActor()
{
	PrimaryActorTick.bCanEverTick = true;

	DamageWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidget"));
	RootComponent = DamageWidget;
	DamageWidget->SetWidgetSpace(EWidgetSpace::Screen);
	DamageWidget->SetDrawAtDesiredSize(true);
}

void ADamageTextActor::BeginPlay()
{
	Super::BeginPlay();
	
	// 랜덤한 위치 오프셋을 주어 숫자가 겹치지 않게 함
	FVector RandomOffset = FVector(FMath::RandRange(-20.f, 20.f), FMath::RandRange(-20.f, 20.f), 50.f);
	AddActorLocalOffset(RandomOffset);

	SetLifeSpan(LifeTime);
}

void ADamageTextActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 위로 떠오르는 효과
	FVector NewLocation = GetActorLocation();
	NewLocation.Z += FloatSpeed * DeltaTime;
	SetActorLocation(NewLocation);

	// 점점 투명해지는 효과 (선택 사항: 위젯에서 애니메이션으로 처리 가능)
	ElapsedTime += DeltaTime;
	float Alpha = 1.0f - (ElapsedTime / LifeTime);
	// Widget의 Opacity 조절 로직은 위젯 블루프린트에서 더 정교하게 가능
}
