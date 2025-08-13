// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	const UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("PublicConnectionsNum : %d"),Subsystem->GetNumPublicConnections()));
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("PlayersNum : %d"),NumberOfPlayers));
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, Subsystem->GetMatchType());
		}


		if(NumberOfPlayers == Subsystem->GetNumPublicConnections())
		{
			UWorld* World = GetWorld();
			if(World)
			{
				bUseSeamlessTravel = true;
				if(Subsystem->GetMatchType() == "FreeForAllPlayers")
				{
					World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
				}else if(Subsystem->GetMatchType() == "Teams")
				{
					World->ServerTravel(FString("/Game/Maps/TeamMap?listen"));
				}else if(Subsystem->GetMatchType() == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/Maps/CaptureFlagMap?listen"));
				}
				
				
				/*
				FString  MatchType = Subsystem->GetMatchType();
				if(MatchType == "FreeForAllPlayers")
				{
					World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
				}else if(MatchType == "Teams")
				{
					World->ServerTravel(FString("/Game/Maps/TeamMap?listen"));
				}else if(MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/Maps/CaptureFlagMap?listen"));
				}
				*/
			}
		}
	}
}
