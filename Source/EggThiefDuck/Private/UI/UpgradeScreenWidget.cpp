// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UpgradeScreenWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/BackgroundBlur.h"
#include "Data/UpgradeDataAsset.h"
#include "DuckCharacter.h"
#include "Kismet/GameplayStatics.h"

void UUpgradeScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 블러 강도 초기 설정 (에디터에서 수정 가능하도록 기본값 부여)
	if (BG_Blur)
	{
		BG_Blur->SetBlurStrength(15.0f);
	}

	// 버튼 클릭 이벤트 바인딩
	if (Button_Upgrade_0) Button_Upgrade_0->OnClicked.AddDynamic(this, &UUpgradeScreenWidget::OnUpgradeSelected_0);
	if (Button_Upgrade_1) Button_Upgrade_1->OnClicked.AddDynamic(this, &UUpgradeScreenWidget::OnUpgradeSelected_1);
	if (Button_Upgrade_2) Button_Upgrade_2->OnClicked.AddDynamic(this, &UUpgradeScreenWidget::OnUpgradeSelected_2);
	if (Button_Reroll) Button_Reroll->OnClicked.AddDynamic(this, &UUpgradeScreenWidget::OnRerollClicked);
}

void UUpgradeScreenWidget::InitUpgradeScreen(const TArray<UUpgradeDataAsset*>& Options)
{
	CurrentOptions = Options;

	// 카드 0 설정
	if (Options.IsValidIndex(0) && Options[0])
	{
		if (Button_Upgrade_0) Button_Upgrade_0->SetVisibility(ESlateVisibility::Visible);
		if (Text_Name_0) Text_Name_0->SetText(Options[0]->UpgradeName);
		if (Text_Desc_0) Text_Desc_0->SetText(Options[0]->UpgradeDescription);
		if (Image_Icon_0) Image_Icon_0->SetBrushFromTexture(Options[0]->UpgradeIcon.Get());
	}
	else if (Button_Upgrade_0)
	{
		Button_Upgrade_0->SetVisibility(ESlateVisibility::Hidden);
	}

	// 카드 1 설정
	if (Options.IsValidIndex(1) && Options[1])
	{
		if (Button_Upgrade_1) Button_Upgrade_1->SetVisibility(ESlateVisibility::Visible);
		if (Text_Name_1) Text_Name_1->SetText(Options[1]->UpgradeName);
		if (Text_Desc_1) Text_Desc_1->SetText(Options[1]->UpgradeDescription);
		if (Image_Icon_1) Image_Icon_1->SetBrushFromTexture(Options[1]->UpgradeIcon.Get());
	}
	else if (Button_Upgrade_1)
	{
		Button_Upgrade_1->SetVisibility(ESlateVisibility::Hidden);
	}

	// 카드 2 설정
	if (Options.IsValidIndex(2) && Options[2])
	{
		if (Button_Upgrade_2) Button_Upgrade_2->SetVisibility(ESlateVisibility::Visible);
		if (Text_Name_2) Text_Name_2->SetText(Options[2]->UpgradeName);
		if (Text_Desc_2) Text_Desc_2->SetText(Options[2]->UpgradeDescription);
		if (Image_Icon_2) Image_Icon_2->SetBrushFromTexture(Options[2]->UpgradeIcon.Get());
	}
	else if (Button_Upgrade_2)
	{
		Button_Upgrade_2->SetVisibility(ESlateVisibility::Hidden);
	}

	// 리롤 버튼 상태 업데이트
	ADuckCharacter* Player = Cast<ADuckCharacter>(GetOwningPlayerPawn());
	if (Player)
	{
		bool bCanAfford = Player->GetGold() >= 20;
		if (Button_Reroll) Button_Reroll->SetIsEnabled(bCanAfford);
		if (Text_RerollCost) Text_RerollCost->SetText(FText::FromString("20 Gold"));
	}
}

void UUpgradeScreenWidget::OnUpgradeSelected_0() { HandleUpgradeSelection(0); }
void UUpgradeScreenWidget::OnUpgradeSelected_1() { HandleUpgradeSelection(1); }
void UUpgradeScreenWidget::OnUpgradeSelected_2() { HandleUpgradeSelection(2); }

void UUpgradeScreenWidget::OnRerollClicked()
{
	ADuckCharacter* Player = Cast<ADuckCharacter>(GetOwningPlayerPawn());
	if (Player)
	{
		Player->RerollUpgrades(this);
	}
}

void UUpgradeScreenWidget::HandleUpgradeSelection(int32 Index)
{
	if (!CurrentOptions.IsValidIndex(Index)) return;

	ADuckCharacter* Player = Cast<ADuckCharacter>(GetOwningPlayerPawn());
	if (Player)
	{
		// 캐릭터에게 선택된 업그레이드 전달 및 적용
		Player->SelectUpgrade(CurrentOptions[Index]);
	}

	// 위젯 제거
	RemoveFromParent();
}
