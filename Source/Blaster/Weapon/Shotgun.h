// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;
	void ShotGunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector>& HitTargets);
protected:
private:

	//散弹枪子弹数量
	UPROPERTY(EditAnywhere , Category= "Weapon Scatter")
	uint32 NumberOfPellets = 10;
};
