// 自学开发！！


#include "ReturnToMainMenu.h"

#include "GameFramework/GameModeBase.h"

void UReturnToMainMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	UWorld* world = GetWorld();;
	if(world)
	{
		PlayerController = PlayerController == nullptr ? world->GetFirstPlayerController() : PlayerController;
		if(PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	if(ReturnButton)
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		MulitplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if(MulitplayerSessionsSubsystem)
		{
			MulitplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
		}
	}
}

bool UReturnToMainMenu::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}
	return true;
}

void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
	if(!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		return;
	}
	UWorld* world = GetWorld();
	if(world)
	{
		AGameModeBase* GameMode = world->GetAuthGameMode<AGameModeBase>();
		if(GameMode)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			PlayerController = PlayerController == nullptr ? world->GetFirstPlayerController() : PlayerController;
			if(PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

void UReturnToMainMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* world = GetWorld();;
	if(world)
	{
		PlayerController = PlayerController == nullptr ? world->GetFirstPlayerController() : PlayerController;
		if(PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

void UReturnToMainMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);
	
	if(MulitplayerSessionsSubsystem)
	{
		MulitplayerSessionsSubsystem->DestroySession();
	}
}
