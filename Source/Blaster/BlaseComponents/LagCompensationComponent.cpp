// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Misc/AssetRegistryInterface.h"


ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	

	FFramePackage Package;
	SaveFramePackage(Package);
	ShowFramePackage(Package,FColor::Orange);
	
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
		true
		);
	}
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




