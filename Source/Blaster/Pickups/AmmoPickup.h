// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AAmmoPickup : public APickup
{
	GENERATED_BODY()


public:
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	
private:
	UPROPERTY(EditAnywhere, Category= "属性修改")
	int32 AmmoAmount = 30;//弹药数量

	UPROPERTY(EditAnywhere, Category= "属性修改")
	EWeaponType WeaponType = EWeaponType::EWT_Pistol; //弹药类型
};
