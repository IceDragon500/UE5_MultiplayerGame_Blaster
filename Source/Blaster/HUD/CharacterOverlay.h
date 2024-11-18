// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "CharacterOverlay.generated.h"

/**
 * 主界面的UserWidget
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ShieldText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ScoreAmount;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* DefeatsAmount;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* MatchCountdownText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* GrenadesText;

	UPROPERTY(meta=(BindWidget))
	class UImage* HighPingImage;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* PingDate;

	UPROPERTY(meta=(BindWidgetAnim), Transient)//Transient仅表示此属性不会序列化到磁盘
	UWidgetAnimation* HighPingAnimation;
	
protected:
private:
};
