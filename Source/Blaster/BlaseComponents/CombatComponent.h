﻿// Fill out your copyright notice in the Description page of Project Settings.

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

	void EquipWeapon(AWeapon* WeaponToEquip);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire();

	//用于检测命中目标
	void TraceUnderCrosehairs(FHitResult& TraceHitResult);
	

private:
	//角色实例
	ABlasterCharacter* Character;

	//已经装备上的武器
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	//是否在瞄准
	UPROPERTY(Replicated)
	bool bAiming;

	//基础移动速度
	UPROPERTY(EditAnywhere , meta=( AllowPrivateAccess = true))
	float BaseWalkSpeed = 600.f;

	//瞄准下的移动速度
	UPROPERTY(EditAnywhere , meta=( AllowPrivateAccess = true))
	float AimWalkSpeed = 450.f;

	//是否按下开火键
	bool bFireButtonPressed;

	//命中目标的位置
	FVector HitTarget;
	
		
};
