// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

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
	//ABlasterPlayerController* AttackerControlle 攻击者的控制器
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);

	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	//定义一个延迟开始游戏的延迟时间
	UPROPERTY(EditAnywhere)
	float WarmupTime = 10.f;
	
	//关卡开始的时间
	float LevelStartingTime = 0.f;
	
protected:
	
	virtual void BeginPlay() override;

private:
	//倒计时时间
	float CountdownTime = 0.f;
	
};