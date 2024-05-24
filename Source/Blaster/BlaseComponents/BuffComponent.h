// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	
	//设置ABlasterCharacter是友元类，这样ABlasterCharacter可以直接访问UBuffComponent中的protected和private方法和变量
	friend class ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

public:
	//处理治疗相关
	void Heal(float HealAmount, float HealingTime);
	//处理速度相关
	void BuffSpeed(float BuffbaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);
	
protected:
	//tick中检查治疗
	void HealRmapUp(float DeltaTime);

private:
	UPROPERTY()
	ABlasterCharacter* Character;

	/**
	 * Heal buff
	 */
	bool bHealing = false;//当前是否在恢复治疗中
	float HealingRate = 0;//每秒恢复的治疗量 = 总治疗量HealAmount / 治疗时间 HealingTime
	float AmountToHeal = 0;

	/**
	 * Speed buff
	 */

	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitialBaseSpeed;//初始的移动速度
	float InitialCrouchSpeed;//初始的蹲下移动速度

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);
		
};
