// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 角色头上的名称显示UserWidget
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	void SetDisplayText(FString TextToDisplay);
	
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerName(APawn* InPawn);

	
protected:

	//virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;  5.2不再使用这个语句
	virtual void NativeDestruct() override;

	
private:

	
	
};
