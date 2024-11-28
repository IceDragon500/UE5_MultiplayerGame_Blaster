// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/ReturnToMainMenu.h"
#include "BlasterPlayerController.generated.h"

struct FInputActionValue;
class UInputAction;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);


/**
 * PlayerController类，这里主要是在控制HUD界面的内容显示
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void HideTeamScores();
	void InitTeamScores();
	void SetHUDRedTeamScore(int32 RedScore);
	void SetHUDBlueTeamScore(int32 BlueScore);
	
	//Synced with server world clock
	//与服务器世界时钟同步
	virtual float GetServerTime();
	
	//Sync with server clock as soon as possible
	//尽快与服务器时钟同步
	virtual void ReceivedPlayer() override;
	
	//从GameMode设置MatchState
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	
	////进入游戏开始阶段后，把角色主界面添加到屏幕，然后将等待界面设置为隐藏
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	
	//游戏结束后进入冷却阶段，这个时候移除界面内容  然后把等待界面显示出来
	void HandleCooldown();

	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;

	void BroadcastElie(APlayerState* Attacker, APlayerState* Victim);
	
protected:

	virtual void SetupInputComponent() override;

	//增强输入-返回菜单菜单按钮
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|增强输入")
	UInputAction* ReturnMenu;
	
	//教程里面是ShowReturnToMainMenu()
	void ReturnKeyPressed(const FInputActionValue& Value);

	
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
	void ServerCheckMatchState();

	//作为客户端
	//接受ServerChenckMatchState传来的比赛相关的时间和比赛状态的值
	//然后设置到自己的比赛相关的时间和比赛状态的值
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float StatingTime, float Cooldown);

	void CheckPing(float DeltaTime);//检查ping值，在设定的时候显示和关闭ping的动画
	void ShowPingDate(float Ping);//显示ping值
	void HighPingWarning();//播放ping的动画
	void StopHighPingWarning();//停止播放ping的动画

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);
	
	UPROPERTY(ReplicatedUsing=OnRep_ShowTeamScores)
	bool bShowTeamScores = false;
	
	UFUNCTION()
	void OnRep_ShowTeamScores();

	FString GetInfoText(const TArray<class ABlasterPlayerState*>& PlayerStates);
	FString GetTeamsInfoText(class ABlasterGameState* BlasterGameState);
	
private:
	UPROPERTY()
	ABlasterHUD* BlasterHUD;
	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	/**
	 * return to main menu
	 */
	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	UPROPERTY()
	UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;

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

	float HUDHealth;
	float HUDMaxHealth;
	bool bInitializeHealth = false;
	
	float HUDShield;
	float HUDMaxShield;
	bool bInitializeShield = false;
	
	float HUDScore;
	bool bInitializeScore = false;
	
	int32 HUDDefeats;
	bool bInitializeDefeats = false;
	
	int32 HUDGrenades;
	bool bInitializeGrenades = false;

	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;

	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;

	float HighPingRunningTime = 0.f;

	//显示ping图标的时间 默认5秒
	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	float PingAnimationRunningTime = 0.f;

	//间隔检查ping的的时间，默认20秒检查一次
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	//当ping太高了，才开启Server倒带功能
	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHightPing);

	//超过设定值，就显示延迟图标
	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
	
};
