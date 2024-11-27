// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Announcement.h"
#include "CharacterOverlay.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"
/*
 * HUD主类，其他的UserWidget需要通过这里来显示
 */
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
	FLinearColor CrosshairColor;
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

/*
 * 创建CharacterOverlay角色主界面
 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCharacterOverlay> CharacterOverlayClass;
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;
	//将游戏中主界面添加到窗口
	void AddCharacterOverlay();

/*
 * 创建Announcement公告界面
*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAnnouncement> AnnouncementClass;
	UPROPERTY()
	UAnnouncement* Announcement;
	//将开始游戏的公告界面，添加至窗口
	void AddAnnouncement();

	//将Elim消息 添加至窗口
	void AddElimAnnouncement(FString Attacker, FString Victim);

protected:
	virtual  void BeginPlay() override;

	
private:
	
	UPROPERTY()
	class APlayerController* OwningPlayer;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportSize, FVector2D Spread, FLinearColor CrosshairColor);
	FHUDPackage HUDPackage;

	//准星扩散的最大值
	//这里设置了最大值，控制的变量是0~1，用变量乘以这个最大值
	UPROPERTY(EditAnywhere)
	float CrosshairSpreaMax = 16.f;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 5.5f;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);

	TArray<UElimAnnouncement*> ElimMessages;
	
};
