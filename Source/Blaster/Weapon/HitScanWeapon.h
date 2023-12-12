// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:

	virtual void Fire(const FVector& HitTarget);
	
protected:

private:
	//射线武器的伤害值
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	//命中部位的粒子特效
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	//命中轨迹特效
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;
};
