// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	AProjectileWeapon();

	//覆写父类Weapon中的fire方法
	virtual void Fire(const FVector& HitTarget) override;

protected:

private:

	//设置子弹类
	UPROPERTY(EditAnywhere, Category= "武器属性|子弹")
	TSubclassOf<AProjectile> ProjectileClass;
	
};
