// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2d ViewportSize;
	if(GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2d ViewportCenter(ViewportSize.X / 2.f , ViewportSize.Y/2.f);

		//控制准星变化的量
		//乘以的值，就是变量，是0~1之间的一个值
		float SpreadScaled = CrosshairSpreaMax * HUDPackage.CrosshairSpread;
		
		if(HUDPackage.CrosshairCenter)
		{
			DrawCrosshair(HUDPackage.CrosshairCenter , ViewportCenter, FVector2d(0.f, 0.f), HUDPackage.CrosshairColor);
		}
		if(HUDPackage.CrosshairLeft)
		{
			DrawCrosshair(HUDPackage.CrosshairLeft , ViewportCenter,FVector2d(-SpreadScaled, 0.f), HUDPackage.CrosshairColor);
		}
		if(HUDPackage.CrosshairRight)
		{
			DrawCrosshair(HUDPackage.CrosshairRight , ViewportCenter, FVector2d(SpreadScaled, 0.f), HUDPackage.CrosshairColor);
		}
		if(HUDPackage.CrosshairTop)
		{
			DrawCrosshair(HUDPackage.CrosshairTop , ViewportCenter, FVector2d(0.f, -SpreadScaled), HUDPackage.CrosshairColor);
		}
		if(HUDPackage.CrosshairDown)
		{
			DrawCrosshair(HUDPackage.CrosshairDown , ViewportCenter, FVector2d(0.f, SpreadScaled), HUDPackage.CrosshairColor);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2d ViewportSize, FVector2d Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2d TextureDrawPoint(
		ViewportSize.X - TextureWidth / 2.f + Spread.X,
		ViewportSize.Y - TextureHeight / 2.f + Spread.Y
		);

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
		CrosshairColor  //顶点的颜色，默认为白色
		);
	
}
