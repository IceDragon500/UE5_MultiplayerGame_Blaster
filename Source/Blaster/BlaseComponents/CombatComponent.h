// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
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

	//主要实现当开火键按下时的逻辑
	//进行射线检测
	//调用Character 和 Weapon中的fire方法
	void FireButtonPressed(bool bPressed);

	//服务端上进行的开火逻辑  ,传入参数为命中的位置FVector
	//这里使用FVector_NetQuantize进行一定的网络优化
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	//多播的开火逻辑  ,传入参数为命中的位置FVector
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	//用于检测命中目标
	void TraceUnderCrosehairs(FHitResult& TraceHitResult);

	//设置准星
	void SetHUDCrosshair(float DeltaTime);
	

private:
	//角色实例
	ABlasterCharacter* Character;
	//角色控制器实例
	ABlasterPlayerController* Controller;
	//角色HUD实例
	ABlasterHUD* HUD;

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

	//准星在移动下的扩散量
	float CrosshairVelocityFactor;
	//准星在掉落时的扩散量
	float CrosshairInAirFactor;
	
		
};
