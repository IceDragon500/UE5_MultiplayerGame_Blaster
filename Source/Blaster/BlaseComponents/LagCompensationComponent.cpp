// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Misc/AssetRegistryInterface.h"


ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	

	/*
	FFramePackage Package;
	SaveFramePackage(Package);
	ShowFramePackage(Package,FColor::Orange);
	*/
	
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		//我们先获得一下FrameHistory中，头和尾相差多少时间HistoryLength，
		//中间差的时间HistoryLength表示这个FrameHistory记录了多长时间的帧包
		
		//如果计算出来的时间比我们设定的长，也就是说帧包记录多了
		//则需要删除末尾的帧包，然后再计算一次HistoryLength

		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while(HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}

		//我们继续保存帧包至FrameHistory中，直到HistoryLength长度达到while判断的标准
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		ShowFramePackage(ThisFrame, FColor::Red);
	}
	
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& framePackage, const FColor& Color)
{
	for(auto& BoxInfo : framePackage.HitBoxInfo)
	{
		DrawDebugBox(
		GetWorld(),
		BoxInfo.Value.Location,
		BoxInfo.Value.BoxExtent,
		FQuat(BoxInfo.Value.Rotation),
		Color,
		false,
		4.f
		);
	}
}

void ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;


	//Frame package that we check to verify a hit
	//我们检查以验证命中的帧包
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	
	//Frame histroy of the HitCharacter
	//被击中角色的帧历史记录
	const TDoubleLinkedList<FFramePackage>& Histroy = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = FrameHistory.GetTail()->GetValue().Time;
	const float NewestHistoryTime = FrameHistory.GetHead()->GetValue().Time;
	if(OldestHistoryTime > HitTime)
	{
		// too far back - too laggy to do ServerSideRewind
		//太远 - 太滞后，无法进行服务器端倒带
		return;
	}
	if(OldestHistoryTime == HitTime)
	{
		FrameToCheck = Histroy.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if(NewestHistoryTime <= HitTime)
	{
		FrameToCheck = Histroy.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = Histroy.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

	while(Older->GetValue().Time > HitTime)
	{
		if(Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if(Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if(Older->GetValue().Time == HitTime) // highly unlikely, but we found our frame to check 可能性很小，HitTime和Older的Time完全相同
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}

	if(bShouldInterpolate)
	{
		// Interpolate between Younger and Older
		//在两个帧包之间使用插值
	}
	
	
	if(bReturn) return;
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if(Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		for(auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}




