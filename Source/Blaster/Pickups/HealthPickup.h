// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()
public:
	AHealthPickup();
	
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
private:

	UPROPERTY(EditAnywhere, Category= "属性修改")
	float HealAmount = 100.f;//可以治疗的点数
	
	UPROPERTY(EditAnywhere, Category= "属性修改")
	float HealingTime = 5.f;//回复的时间 ，这里是设计了一个缓慢回复的效果
	
};
