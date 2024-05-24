// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ASpeedPickup : public APickup
{
	GENERATED_BODY()
public:
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
private:
	
	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1600.f;//移动速度增加到1600

	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 850.f;//蹲下移动速度增加到850

	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 30.f;//移速增加持续时间
};
