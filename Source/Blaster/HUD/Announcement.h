// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Announcement.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* WarmupTime;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* AnnouncementText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* InfoText;
	
protected:
private:
	
	
	
};
