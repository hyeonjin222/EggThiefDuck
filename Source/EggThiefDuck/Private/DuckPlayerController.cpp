// Fill out your copyright notice in the Description page of Project Settings.

#include "DuckPlayerController.h"
#include "UI/MainHUDWidget.h"
#include "UI/HealthBarWidget.h"
#include "UI/DuckCursorWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EnemyBase.h"
#include "Components/WidgetComponent.h"

void ADuckPlayerController::QuitGame()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false);
}

void ADuckPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 시작 시 화면 페이드 인 연출 (2초)
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(1.0f, 0.0f, 2.0f, FLinearColor::Black, false, true);
	}

	// 커스텀 커서 생성 (인트로 중에도 필요)
	if (CursorWidgetClass)
	{
		CursorWidget = CreateWidget<UDuckCursorWidget>(this, CursorWidgetClass);
		if (CursorWidget)
		{
			CursorWidget->AddToViewport(999); // 최상단에 배치
			bShowMouseCursor = false; // 윈도우 기본 커서는 숨김
		}
	}
}

void ADuckPlayerController::InitializeHUD()
{
	// 이미 생성되어 있다면 무시
	if (MainHUDWidget) return;

	// 메인 HUD 생성
	if (MainHUDClass)
	{
		MainHUDWidget = CreateWidget<UMainHUDWidget>(this, MainHUDClass);
		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport(100);
		}
	}
}

void ADuckPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// 커서 위치 업데이트
	if (CursorWidget)
	{
		// 시스템 커서가 활성화되면(UI 상황 등) 조준점 숨김, 꺼지면(전투 상황) 보임
		CursorWidget->SetVisibility(bShowMouseCursor ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);

		float MouseX, MouseY;
		if (GetMousePosition(MouseX, MouseY))
		{
			CursorWidget->SetPositionInViewport(FVector2D(MouseX, MouseY));
		}
	}
}

void ADuckPlayerController::PlayCursorFireAnimation()
{
	if (CursorWidget)
	{
		CursorWidget->NotifyFire();
	}
}

void ADuckPlayerController::UpdateHUDHealth(float CurrentHP, float MaxHP)
{
	if (MainHUDWidget)
	{
		MainHUDWidget->UpdateHealth(CurrentHP, MaxHP);
	}
}

void ADuckPlayerController::UpdateHUDXP(int32 Level, float CurrentXP, float MaxXP)
{
	if (MainHUDWidget)
	{
		MainHUDWidget->UpdateXP(Level, CurrentXP, MaxXP);
	}
}

void ADuckPlayerController::UpdateHUDGold(int32 Amount)
{
	if (MainHUDWidget)
	{
		MainHUDWidget->UpdateGold(Amount);
	}
}

void ADuckPlayerController::UpdateHUDTime(int32 Day, float Hour)
{
	if (MainHUDWidget)
	{
		MainHUDWidget->UpdateTime(Day, Hour);
	}
}

void ADuckPlayerController::ShowHUDDayNotification(int32 Day)
{
	if (MainHUDWidget)
	{
		MainHUDWidget->ShowDayNotification(Day);
	}
}

void ADuckPlayerController::StartDeathFadeSequence()
{
	// 1. 모든 적의 체력바 숨기기 (사망 즉시 시각적 정리)
	HideAllEnemyHealthBars();

	// 2. 메인 HUD 부드럽게 숨기기 (2초 동안)
	if (MainHUDWidget)
	{
		MainHUDWidget->StartFadeOut(2.0f);
	}

	// 3. 화면 서서히 검게 페이드 아웃 (2초 동안)
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(0.0f, 1.0f, 2.0f, FLinearColor::Black, false, true);
	}

	// 4. 2초 후 (화면이 완전히 검게 변했을 때) 모든 적 제거
	FTimerHandle ClearEnemiesTimerHandle;
	GetWorldTimerManager().SetTimer(ClearEnemiesTimerHandle, this, &ADuckPlayerController::ClearAllEnemies, 2.0f, false);

	// 5. 완전 암전 상태에서 2초 더 대기 (총 4초 후 재시작)
	FTimerHandle RestartTimerHandle;
	GetWorldTimerManager().SetTimer(RestartTimerHandle, this, &ADuckPlayerController::RestartLevel, 4.0f, false);
}

void ADuckPlayerController::HideAllEnemyHealthBars()
{
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), FoundEnemies);
	
	for (AActor* EnemyActor : FoundEnemies)
	{
		if (AEnemyBase* Enemy = Cast<AEnemyBase>(EnemyActor))
		{
			if (UActorComponent* HealthBarComp = Enemy->GetComponentByClass(UWidgetComponent::StaticClass()))
			{
				if (UWidgetComponent* WidgetComp = Cast<UWidgetComponent>(HealthBarComp))
				{
					// [변경] 즉시 숨기지 않고 부드럽게 페이드 아웃 실행
					if (UHealthBarWidget* HealthWidget = Cast<UHealthBarWidget>(WidgetComp->GetUserWidgetObject()))
					{
						HealthWidget->StartFadeOut(2.0f);
					}
					else
					{
						// 혹시라도 위젯 인스턴스가 없으면 즉시 숨김
						WidgetComp->SetVisibility(false);
					}
				}
			}
		}
	}
}

void ADuckPlayerController::SetHUDVisibility(bool bVisible)
{
	if (MainHUDWidget)
	{
		MainHUDWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void ADuckPlayerController::ClearAllEnemies()
{
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), FoundEnemies);
	for (AActor* EnemyActor : FoundEnemies)
	{
		EnemyActor->Destroy();
	}
	UE_LOG(LogTemp, Log, TEXT("All enemies cleared after fade out."));
}

void ADuckPlayerController::RestartLevel()
{
	// 현재 레벨 다시 로드 (인트로 화면으로 복귀)
	FString LevelName = GetWorld()->GetMapName();
	LevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelName));
}
