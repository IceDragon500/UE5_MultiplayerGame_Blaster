// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	//Score += ScoreAmount;
	Character = Cast<ABlasterCharacter>(GetPawn());
	if(Character)
	{
		Controller = Cast<ABlasterPlayerController>(Character->Controller);
		if(Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
	DOREPLIFETIME(ABlasterPlayerState, Team);
	
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	Character = Cast<ABlasterCharacter>(GetPawn());
	if(Character)
	{
		Controller = Cast<ABlasterPlayerController>(Character->Controller);
		if(Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}	
}

void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Cast<ABlasterCharacter>(GetPawn());
	if(Character)
	{
		Controller = Cast<ABlasterPlayerController>(Character->Controller);
		if(Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_Defeats失败"))
	}
}

void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	//Score += ScoreAmount;
	Character = Cast<ABlasterCharacter>(GetPawn());
	if(Character)
	{
		Controller = Cast<ABlasterPlayerController>(Character->Controller);
		if(Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlasterPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;
	ABlasterCharacter* BCharacter = Cast<ABlasterCharacter>(GetPawn());
	if(BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}

void ABlasterPlayerState::OnRep_Team()
{
	ABlasterCharacter* BCharacter = Cast<ABlasterCharacter>(GetPawn());
	if(BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}


