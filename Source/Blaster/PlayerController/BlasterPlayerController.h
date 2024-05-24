// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Blaster/GameMode/BlasterGameMode.h"
#include "GameFramework/PlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "BlasterPlayerController.generated.h"

/**
 * PlayerController类，这里主要是在控制HUD界面的内容显示
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABlasterPlayerController();

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//Synced with server world clock
	//与服务器世界时钟同步
	virtual float GetServerTime();
	
	//Sync with server clock as soon as possible
	//尽快与服务器时钟同步
	virtual void ReceivedPlayer() override;
	
	//从GameMode设置MatchState
	void OnMatchStateSet(FName State);
	
	////进入游戏开始阶段后，把角色主界面添加到屏幕，然后将等待界面设置为隐藏
	void HandleMatchHasStarted();
	
	//游戏结束后进入冷却阶段，这个时候移除界面内容  然后把等待界面显示出来
	void HandleCooldown();
	
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	//用来刷新界面上的UI和数值
	void PollInit();

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

	//作为服务器
	//从BlasterPlayerController获取到比赛相关的时间和比赛状态
	//然后调用ClientJoinMidgame，将值传给客户端
	//最后在BlasterPlayerController开始的时候调用这个方法，并且显示初始的热身倒计时界面
	UFUNCTION(Server, Reliable)
	void ServerChenckMatchState();

	//作为客户端
	//接受ServerChenckMatchState传来的比赛相关的时间和比赛状态的值
	//然后设置到自己的比赛相关的时间和比赛状态的值
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float StatingTime, float Cooldown);

private:
	UPROPERTY()
	ABlasterHUD* BlasterHUD;
	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	//比赛时长（秒）,从GameMode中赋值
	float MatchTime = 0.f;
	//比赛前等待时间（热身时间）,从GameMode中赋值
	float WarmupTime = 0.f;
	//比赛倒计时时间,从GameMode中赋值
	float LevelStartingTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	//存下GameMode比赛状态,从GameMode中赋值
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;
	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	bool bInitializeCharacterOverlay = false;

	float HUDHealth;
	float HUDMaxHealth;
	
	float HUDShield;
	float HUDMaxShield;
	
	float HUDScore;
	int32 HUDDefeats;
	int32 HUDGrenades;
	
	
};
