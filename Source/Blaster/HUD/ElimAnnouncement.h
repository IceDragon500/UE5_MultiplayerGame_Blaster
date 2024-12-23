// 自学开发！！

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "ElimAnnouncement.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UElimAnnouncement : public UUserWidget
{
	GENERATED_BODY()
public:
	//设置需要显示文本到公告上AnnouncementText
	//AttackerName攻击者的名字
	//VictimName被攻击者的名字
	void SetAnnouncementText(FString AttackerName, FString VictimName);
	
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* AnnouncementBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock*	AnnouncementText;

protected:

private:
	
};
