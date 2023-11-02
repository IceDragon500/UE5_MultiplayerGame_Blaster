// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BlasterGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	
protected:

	
private:

	FString PlayerName = (TEXT("NULL"));
	
	
};
