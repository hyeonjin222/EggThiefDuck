// Fill out your copyright notice in the Description page of Project Settings.

#include "DropItemBase.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DuckCharacter.h"

ADropItemBase::ADropItemBase()
{
	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = BoxComp;

	// 1. 물리 및 기본 설정
	BoxComp->SetSimulatePhysics(true);
	BoxComp->SetEnableGravity(true);
	BoxComp->BodyInstance.bUseCCD = true;
	BoxComp->SetNotifyRigidBodyCollision(true);

	BoxComp->BodyInstance.bLockXRotation = true;
	BoxComp->BodyInstance.bLockYRotation = true;
	BoxComp->BodyInstance.bLockZRotation = true;

	// 2. 명시적 콜리전 설정
	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxComp->SetCollisionObjectType(ECC_PhysicsBody);

	// 반응 설정: 바닥(Static/Dynamic)은 무조건 막고, 나머지는 겹침
	BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);  // 표준 바닥
	BoxComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block); // 사용자 환경 바닥 (중요)
	BoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);         // 플레이어 습득
	BoxComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);  // 아이템끼리 겹침

	// 자석 감지 범위
	MagnetSphere = CreateDefaultSubobject<USphereComponent>(TEXT("MagnetSphere"));
	MagnetSphere->SetupAttachment(RootComponent);
	MagnetSphere->SetSphereRadius(MagnetRange);
	MagnetSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MagnetSphere->SetCollisionObjectType(ECC_WorldDynamic);
	MagnetSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	MagnetSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetLifeSpan(LifeTime);
}

void ADropItemBase::BeginPlay()
{
	Super::BeginPlay();
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ADropItemBase::OnOverlapBegin);
	MagnetSphere->OnComponentBeginOverlap.AddDynamic(this, &ADropItemBase::OnMagnetOverlap);
}

void ADropItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 자석 효과
	if (bIsHoming && TargetActor)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector TargetLocation = TargetActor->GetActorLocation();
		FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, MagnetSpeed);
		SetActorLocation(NewLocation);
		MagnetSpeed += DeltaTime * 20.0f;
	}
	else
	{
		// 수동 중력 가속도
		if (BoxComp && BoxComp->IsSimulatingPhysics() && GravityScale != 1.0f)
		{
			float GravityZ = GetWorld()->GetGravityZ();
			FVector ExtraGravityForce = FVector(0.f, 0.f, GravityZ * (GravityScale - 1.0f));
			BoxComp->AddForce(ExtraGravityForce, NAME_None, true);
		}
	}

	if (ItemMesh)
	{
		ItemMesh->AddLocalRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));
	}
}

void ADropItemBase::InitVelocity(FVector LaunchVelocity)
{
	if (BoxComp)
	{
		BoxComp->SetSimulatePhysics(true);
		BoxComp->AddImpulse(LaunchVelocity, NAME_None, true);
	}
}

void ADropItemBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(ADuckCharacter::StaticClass()))
	{
		OnPickedUp(OtherActor);
		Destroy();
	}
}

void ADropItemBase::OnPickedUp_Implementation(AActor* Deliverer)
{
	// 기본 클래스에서는 아무것도 하지 않음. 하위 클래스에서 오버라이드.
}

void ADropItemBase::OnMagnetOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(ADuckCharacter::StaticClass()))
	{
		TargetActor = OtherActor;
		bIsHoming = true;
		if (BoxComp)
		{
			BoxComp->SetSimulatePhysics(false);
			BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
	}
}
