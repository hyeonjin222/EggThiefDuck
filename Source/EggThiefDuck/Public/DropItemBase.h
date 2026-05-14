// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DropItemBase.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class EGGTHIEFDUCK_API ADropItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ADropItemBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/** 아이템이 팡 튀어오르는 초기 힘 설정 */
	void InitVelocity(FVector LaunchVelocity);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UBoxComponent> BoxComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	/** 아이템 획득 시 호출 (BP에서 구현) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Item")
	void OnPickedUp(AActor* Deliverer);

	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 자석 감지용 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<class USphereComponent> MagnetSphere;

	UFUNCTION()
	void OnMagnetOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 자석(빨려 들어가는) 속도 */
	UPROPERTY(EditAnywhere, Category = "Item|Magnet")
	float MagnetSpeed = 15.0f;

	/** 자석 감지 범위 */
	UPROPERTY(EditAnywhere, Category = "Item|Magnet")
	float MagnetRange = 300.0f;

	/** 회전 애니메이션 속도 */
	UPROPERTY(EditAnywhere, Category = "Item|Visual")
	float RotationSpeed = 100.0f;

	/** 자동 소멸 시간 */
	UPROPERTY(EditAnywhere, Category = "Item")
	float LifeTime = 10.0f;

	/** 중력 배율 (높을수록 빨리 떨어짐) */
	UPROPERTY(EditAnywhere, Category = "Item|Physics")
	float GravityScale = 2.0f;

private:
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	bool bIsHoming = false;
};
