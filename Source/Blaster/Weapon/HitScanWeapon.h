// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 直接命中的武器，一般是手枪步枪狙击之类的
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:

	virtual void Fire(const FVector& HitTarget) override;
	
	
protected:
	
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	
	//命中部位的粒子特效
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	//命中的音效
	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;
	
private:
	
	//命中轨迹特效
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	//枪口闪光特效
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	//开火的音效
	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;
	
};
