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

	virtual void Fire(const FVector& HitTarget) override;
	
protected:

	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

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

	//枪口闪光特效
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	//开火的音效
	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;

	//命中的音效
	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;

	/**
	 * Trace end with scatter
	 * 以散点图结尾点击并应用
	 */
	//从枪口到球体的距离
	UPROPERTY(EditAnywhere, Category= "Weapon Scatter")
	float DistanceToSphere = 800.f;

	//球体的半径
	UPROPERTY(EditAnywhere, Category= "Weapon Scatter")
	float SphereRadius = 75.f;

	//启用或者禁用散射
	UPROPERTY(EditAnywhere, Category= "Weapon Scatter")
	bool bUserScatter = false;
};
