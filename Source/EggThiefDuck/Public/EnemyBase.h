// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnemyBase.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class EGGTHIEFDUCK_API AEnemyBase : public APawn
{
	GENERATED_BODY()

public:
	AEnemyBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void ApplyKnockback(FVector ImpactImpulse);

	/** --- 컴포넌트 (에디터 노출을 위해 public으로 배치) --- */

	/** 박스 형태의 물리 루트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Collision")
	TObjectPtr<UBoxComponent> BoxComp;

	/** 비주얼을 위한 스태틱 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	TObjectPtr<UStaticMeshComponent> EnemyMesh;

protected:
	/** --- 물리 점프 로직 --- */
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement")
	float JumpImpulse = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement")
	float ForwardImpulse = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement")
	float HopInterval = 0.5f;

	float HopTimer = 0.0f;

	bool IsGrounded();
	void PhysicalHop();

	/** 스탯 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float Health = 100.0f;
};
