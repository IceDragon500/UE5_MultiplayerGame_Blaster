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
	//定义一个淘汰玩家的方法
	//ABlasterCharacter* ElimmedCharacter 被淘汰的玩家
	//ABlasterPlayerController* VictimController 被淘汰玩家的控制器
	//ABlasterPlayerController* AttackerControlle 攻击者的控制器
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);

	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
protected:

private:
	
};
