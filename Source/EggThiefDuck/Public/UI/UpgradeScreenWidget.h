// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeScreenWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;
class UUpgradeDataAsset;

/**
 * 뱀파이어 서바이벌 스타일 업그레이드 화면의 C++ 베이스 클래스
 */
UCLASS()
class EGGTHIEFDUCK_API UUpgradeScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** UI 초기화 및 데이터 설정 */
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void InitUpgradeScreen(const TArray<UUpgradeDataAsset*>& Options);

protected:
	virtual void NativeConstruct() override;

	/** --- UI 컴포넌트 바인딩 (BP의 위젯 이름과 일치해야 함) --- */

	// 카드 1
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Name_0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Desc_0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon_0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Upgrade_0;

	// 카드 2
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Name_1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Desc_1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon_1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Upgrade_1;

	// 카드 3
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Name_2;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Desc_2;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon_2;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Upgrade_2;

private:
	/** 버튼 클릭 시 호출될 내부 함수 */
	UFUNCTION()
	void OnUpgradeSelected_0();
	UFUNCTION()
	void OnUpgradeSelected_1();
	UFUNCTION()
	void OnUpgradeSelected_2();

	/** 선택된 업그레이드를 처리하는 공통 로직 */
	void HandleUpgradeSelection(int32 Index);

	/** 현재 표시 중인 옵션들 저장 */
	UPROPERTY()
	TArray<TObjectPtr<UUpgradeDataAsset>> CurrentOptions;
};
