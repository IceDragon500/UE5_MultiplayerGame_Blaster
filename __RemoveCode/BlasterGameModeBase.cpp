// Copyright Epic Games, Inc. All Rights Reserved.


#include "BlasterGameModeBase.h"

#include "GameFramework/PlayerState.h"

void ABlasterGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if(GameState)
	{
		APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
		if(PlayerState)
		{
			PlayerName = PlayerState->GetPlayerName();
		}
	}
}
