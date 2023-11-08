// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABlasterPlayerController();

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Score);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;
	

private:
	UPROPERTY()
	ABlasterHUD* BlasterHUD;
	
};
