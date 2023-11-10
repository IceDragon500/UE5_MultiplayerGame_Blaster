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
	void SetHUDMatchCountdown(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	
	//Synced with server world clock
	//与服务器世界时钟同步
	virtual float GetServerTime();
	
	//Sync with server clock as soon as possible
	//尽快与服务器时钟同步
	virtual void ReceivedPlayer() override;
	
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	/*
	 *Sync time between client and server
	 *在客户端和服务器之间同步时间
	 */
	//Requests the current server time, passing in the client's time when the request was sent
	//请求当前服务器时间，传入发送请求时的客户端时间
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	//Reports the current server time to the client in response to ServerRequestServerTime
	//作为对ServerRequestServerTime的响应，向客户端报告当前服务器时间
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	//difference between client and server time
	//客户机和服务器时间之间的差异
	float ClientServerDelta = 0.f;

	//设置同步的时间间隔，目前设置为5秒同步一次
	UPROPERTY(EditAnywhere, Category= Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	void CheckTimeSync(float DeltaTime);

private:
	UPROPERTY()
	ABlasterHUD* BlasterHUD;

	//比赛时长（秒）
	float MatchTime = 120.f;
	uint32 CountdownInt = 0;
	
};
