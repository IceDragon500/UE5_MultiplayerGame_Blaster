// 自学开发！！

#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ATeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()
public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	
	//计算伤害
	//用来区分队友攻击和敌方攻击
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;

	//用来处理玩家被淘汰时需要执行的逻辑
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;
	
protected:
	virtual void HandleMatchHasStarted() override;
private:
	
};
