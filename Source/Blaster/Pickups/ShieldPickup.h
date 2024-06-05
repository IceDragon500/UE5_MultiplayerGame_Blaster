// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AShieldPickup : public APickup
{
	GENERATED_BODY()
public:

	
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
private:

	UPROPERTY(EditAnywhere, Category= "属性修改")
	float ShieldAmount = 100.f;//可以治疗的点数
	
	UPROPERTY(EditAnywhere, Category= "属性修改")
	float ShieldTime = 5.f;//回复的时间 ，这里是设计了一个缓慢回复的效果	
};
