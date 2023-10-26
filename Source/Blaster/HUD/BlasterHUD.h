// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairDown;
	float CrosshairSpread;
};

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

protected:

private:

	void DrawCrosshair(UTexture2D* Texture, FVector2d ViewportSize, FVector2d Spread);
	FHUDPackage HUDPackage;

	//准星扩散的最大值
	//这里设置了最大值，控制的变量是0~1，用变量乘以这个最大值
	UPROPERTY(EditAnywhere)
	float CrosshairSpreaMax = 16.f;
	
};