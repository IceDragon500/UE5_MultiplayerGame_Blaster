// 自学开发！！


#include "ElimAnnouncement.h"

void UElimAnnouncement::SetAnnouncementText(FString AttackerName, FString VictimName)
{
	FString ElimAnnouncementText = FString::Printf(TEXT("%s 击败了 %s!"), *AttackerName, *VictimName);
	if(AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(ElimAnnouncementText));
	}
}
