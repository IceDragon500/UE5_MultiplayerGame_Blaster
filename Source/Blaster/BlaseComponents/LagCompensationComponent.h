// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

//我们需要为每个帧分别存储HitBox的位置，旋转、大小
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
	
	UPROPERTY()
	ABlasterCharacter* Character;
	
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

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> HeadShots;

	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> BodyShots;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	ULagCompensationComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//将存储的帧包显示在游戏中
	void ShowFramePackage(const FFramePackage& framePackage, const FColor& Color);

	//计算射线类武器Hitscan在开启服务器倒带功能后，命中结果如何
	//传入被击中的角色，起点，终点，命中时间	
	FServerSideRewindResult ServerSideRewind(
	ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation,
	float HitTime
	);

	//计算飞弹类武器Projectile在开启服务器倒带功能后，命中结果如何
	FServerSideRewindResult ProjectileServerSideRewind(
	ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize100& InitialVelocity,
	float HitTime
	);

	//计算霰弹类武器Shotgun在开启服务器倒带功能后，命中结果如何
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
	const TArray<ABlasterCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations,
	float HitTime);
	
	//对射线类武器Hitscan在开启服务器倒带功能后，计算伤害的逻辑
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);

	//对飞弹类武器Projectile在开启服务器倒带功能后，计算伤害的逻辑
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
	ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize100& InitialVelocity,
	float HitTime
		);

	//对霰弹类武器Shotgun在开启服务器倒带功能后，计算伤害的逻辑
	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
		);

protected:

	virtual void BeginPlay() override;

	//传入一个Package，对每一帧保存时间和Box的信息至Package中
	void SaveFramePackage(FFramePackage& Package);
	
	//传入Older和Younger两个帧包，计算出中间值，并将其返回
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	void CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	
	//还原HitBox的位置，并且将碰撞重置为无
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	
	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	void SaveFramePackage();

	//传入被击中的角色和时间，返回这个角色在那个时间的HitBox信息
	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime);
	
	//Hitscan
	//这里我们需要通过计算得到的FFramePackage帧包，来和被命中的角色、射击起始位置、命中位置，来判断被命中的角色是否真的被命中了
	//我们这里需要创建这个函数，并且返回一个bool的结构体FServerSideRewindResult，这个结构体要包括是否命中了、是否爆头
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	/**
	 *Projectile
	 */

	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
		);
	
	/**
	 * Shotgun
	 */

	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations);

	void PrintBoxExtent(FVector Box, const FColor& Color, int32 key);
	
private:
	UPROPERTY()
	ABlasterCharacter* Character;
	
	UPROPERTY()
	ABlasterPlayerController* Controller;

	//用来保存帧包
	//数据从上至下进来，上面是头，始终是新的，下面是尾，始终是旧的
	TDoubleLinkedList<FFramePackage> FrameHistory;

	//保存帧包的最大时间 
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
};
