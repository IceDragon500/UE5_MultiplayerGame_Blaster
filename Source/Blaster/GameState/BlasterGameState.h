// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;

	/**
	 * Teams队伍
	 */
	void RedTeamScores();
	void BlueTeamScores();
	//用来保存红队角色的BlasterPlayerState
	TArray<ABlasterPlayerState*> RedTeam;
	//用来保存蓝队角色的BlasterPlayerState
	TArray<ABlasterPlayerState*> BlueTeam;
	
	UFUNCTION()
	void OnRep_RedTeamScore();
	
	UFUNCTION()
	void OnRep_BlueTeamScore();

	
protected:
private:
	float TopScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

public:
	FORCEINLINE float GetRedTeamScore() const { return RedTeamScore; }
	FORCEINLINE float GetBlueTeamScore() const { return BlueTeamScore; }
	
};
