// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

//我们需要为每个帧分别存储位置，旋转、框的范围（大小）
USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;
	
	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

//创建一个帧包的结构体
//需要存储有关给定角色的所有命中框的信息
//还需要存储这些信息的时间
//我们需要为每个帧分别存储位置，旋转、框的范围（大小）
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;
	
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;
	
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class ABlasterCharacter;

	void ShowFramePackage(const FFramePackage& framePackage, const FColor& Color);
	
protected:

	virtual void BeginPlay() override;

	//传入一个Package，对每一帧保存时间和Box的信息至Package中
	void SaveFramePackage(FFramePackage& Package);

private:
	UPROPERTY()
	ABlasterCharacter* Character;
	
	UPROPERTY()
	ABlasterPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;//用来保存帧包

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;//保存帧包的最大时间 
};
