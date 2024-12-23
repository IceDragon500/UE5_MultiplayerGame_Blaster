// 自学开发！！

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/Team.h"
#include "GameFramework/PlayerStart.h"
#include "TeamPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()
public:
protected:
private:
	UPROPERTY(EditAnywhere)
	ETeam Team = ETeam::ET_NoTeam;
public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	
};
