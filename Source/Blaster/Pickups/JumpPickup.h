// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "JumpPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AJumpPickup : public APickup
{
	GENERATED_BODY()

public:
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
private:
	UPROPERTY(EditAnywhere, Category= "属性修改")
	float JumpZVelovityBuff = 1200.f;//修改起跳速度
	 
	UPROPERTY(EditAnywhere, Category= "属性修改")
	float JumpBuffTime = 10.f;//修改buff持续时间
	
};
