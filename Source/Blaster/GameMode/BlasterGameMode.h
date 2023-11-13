// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	extern BLASTER_API const FName Cooldown; //游戏结束后的冷却时间，显示当前游戏的结果，和下一场开启的倒计时
	
}

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	ABlasterGameMode();

	virtual void Tick(float DeltaSeconds) override;

	
	//定义一个淘汰玩家的方法
	//ABlasterCharacter* ElimmedCharacter 被淘汰的玩家
	//ABlasterPlayerController* VictimController 被淘汰玩家的控制器
	//ABlasterPlayerController* AttackerController 攻击者的控制器
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);

	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
	
	//定义一个游戏进行的最大时间限制
	UPROPERTY(EditAnywhere)
	float MatchTime = 120.f;
	
	//定义一个延迟开始游戏的延迟时间
	UPROPERTY(EditAnywhere)
	float WarmupTime = 20.f;
	
	//关卡开始的时间
	float LevelStartingTime = 0.f;

	//冷却时长
	UPROPERTY(EditAnywhere)
	float CooldownTime = 30.f;
	
protected:
	
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;

private:
	//倒计时时间
	float CountdownTime = 0.f;
	
};