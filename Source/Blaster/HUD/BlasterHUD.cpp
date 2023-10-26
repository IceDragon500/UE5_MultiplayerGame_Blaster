// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2d ViewportSize;
	if(GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2d ViewportCenter(ViewportSize.X / 2.f , ViewportSize.Y/2.f);

		if(HUDPackage.CrosshairCenter)
		{
			DrawCrosshair(HUDPackage.CrosshairCenter , ViewportCenter);
		}
		if(HUDPackage.CrosshairLeft)
		{
			DrawCrosshair(HUDPackage.CrosshairLeft , ViewportCenter);
		}
		if(HUDPackage.CrosshairRight)
		{
			DrawCrosshair(HUDPackage.CrosshairRight , ViewportCenter);
		}
		if(HUDPackage.CrosshairTop)
		{
			DrawCrosshair(HUDPackage.CrosshairTop , ViewportCenter);
		}
		if(HUDPackage.CrosshairDown)
		{
			DrawCrosshair(HUDPackage.CrosshairDown , ViewportCenter);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2d ViewportSize)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2d TextureDrawPoint(ViewportSize.X - TextureWidth / 2.f,ViewportSize.Y - TextureHeight / 2.f);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		FLinearColor::White  //顶点的颜色，默认为白色
		);
	
}
