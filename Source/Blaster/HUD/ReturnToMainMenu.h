// 自学开发！！

#pragma once

#include "CoreMinimal.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "ReturnToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	void MenuSetup();
	void MenuTearDown();

protected:

	virtual bool Initialize() override;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnPlayerLeftGame();

private:
	UPROPERTY(meta = (BindWidget))
	UButton* ReturnButton;

	UFUNCTION()
	void ReturnButtonClicked();

	UPROPERTY()
	UMultiplayerSessionsSubsystem* MulitplayerSessionsSubsystem;
	
	UPROPERTY()
	APlayerController* PlayerController;
	
};
