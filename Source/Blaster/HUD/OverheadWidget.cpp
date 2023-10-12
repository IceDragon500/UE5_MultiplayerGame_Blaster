// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if(DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString Role;
	switch (LocalRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString(TEXT("授权"));
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString(TEXT("自主代理"));
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString(TEXT("模拟代理"));
		break;
	case ENetRole::ROLE_None:
		Role = FString(TEXT("无"));
		break;
	}
	FString LocalRoleString = FString::Printf(TEXT("本地角色: %s"), *Role);

	SetDisplayText(LocalRoleString);
	
}

void UOverheadWidget::ShowPlayerName(APawn* InPawn)
{
	APlayerState* PlayerState = InPawn->GetPlayerState();
	
	if(PlayerState)
	{
		SetDisplayText(PlayerState->GetPlayerName());
	}
	else
	{
		SetDisplayText(TEXT("NULL"));
	}
	
	
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}
