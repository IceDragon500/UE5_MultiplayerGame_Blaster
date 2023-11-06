// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"

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


