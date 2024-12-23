// 自学开发！！


#include "CaptureTheFlagGameMode.h"

#include "Blaster/CaptureTheFlag/FlagZone.h"
#include "Blaster/GameState/BlasterGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter,	ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(class AFlag* Flag, class AFlagZone* FlagZone)
{
	bool bValidCapture = Flag->GetTeam() != FlagZone->GetTeam();
	ABlasterGameState* BGamesState = Cast<ABlasterGameState>(GameState);
	if(BGamesState)
	{
		if(FlagZone->GetTeam() == ETeam::ET_BlueTeam)
		{
			BGamesState->BlueTeamScores();
		}
		if(FlagZone->GetTeam() == ETeam::ET_RedTeam)
		{
			BGamesState->RedTeamScores();
		}
	}
}