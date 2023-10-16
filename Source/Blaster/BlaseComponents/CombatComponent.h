// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();
	//设置ABlasterCharacter是友元类，这样ABlasterCharacter可以直接访问UCombatComponent中的protected和private方法和变量
	friend class ABlasterCharacter;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//处理需要
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	void EquipWeapon(AWeapon* WeaponToEquip);

	

protected:
	

private:
	ABlasterCharacter* Character;

	UPROPERTY(Replicated)
	AWeapon* EquippedWeapon;

		
};
