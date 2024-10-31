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
	FVector Location;//HitBox位置
	
	UPROPERTY()
	FRotator Rotation;//HitBox的旋转

	UPROPERTY()
	FVector BoxExtent;//HitBox的大小
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

//返回一个bool的结构体FServerSideRewindResult，这个结构体要包括是否命中了、是否爆头
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;//是否命中了
	
	UPROPERTY()
	bool bHeadShot;//是否爆头
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class ABlasterCharacter;

	//将存储的帧包显示在游戏中
	void ShowFramePackage(const FFramePackage& framePackage, const FColor& Color);
	FServerSideRewindResult ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	//
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, class AWeapon* DamageCauser);
	
protected:

	virtual void BeginPlay() override;

	//传入一个Package，对每一帧保存时间和Box的信息至Package中
	void SaveFramePackage(FFramePackage& Package);
	
	//传入Older和Younger两个帧包，计算出中间值，并将其返回
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	//这里我们需要通过计算得到的FFramePackage帧包，来和被命中的角色、射击起始位置、命中位置，来判断被命中的角色是否真的被命中了
	//我们这里需要创建这个函数，并且返回一个bool的结构体FServerSideRewindResult，这个结构体要包括是否命中了、是否爆头
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	
	void CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	void SaveFramePackage();

private:
	UPROPERTY()
	ABlasterCharacter* Character;
	
	UPROPERTY()
	ABlasterPlayerController* Controller;

	//用来保存帧包
	TDoubleLinkedList<FFramePackage> FrameHistory;

	//保存帧包的最大时间 
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 2.f;
};
