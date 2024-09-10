// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Projectile.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

/*
 * 继承自ActorComponet类
 * 主要用来实现战斗过程中的一些逻辑，避免写在角色类里面
 * 比如：装备武器、换弹、开火、
 * 这类需要在角色类中 添加组件来形成功能
 */

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

	//处理需要复制的变量
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
/*
* 装备武器相关的逻辑
*/
	
	void EquipWeapon(AWeapon* WeaponToEquip);//装备武器的完整逻辑
	UFUNCTION()
	void OnRep_EquippedWeapon();
	UFUNCTION()
	void OnRep_SecondaryWeapon();

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);//装备主武器
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);//装备副武器
	
	
	void DropEquippedWeapon();//扔出武器
	void AttachActorToRightHand(AActor* ActorToAttach);//将武器附加在右手上
	void AttachActorToLeftHand(AActor* ActorToAttach);//将武器附加在左手上
	void AttachActorToBackpack(AActor* ActorToAttach);
	void UpdateCarriedAmmo();//更新子弹的HUD和角色身上携带的子弹数量
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);//播放装备武器时的音效
	void ReloadEmptyWeapon();//如果使其的武器为空子弹，尝试换弹
/*
* 交换武器的逻辑
*/
	void SwapWeapons();

	
/*
* 武器换弹相关的逻辑
*/
	void Reload();//武器换弹
	
	UFUNCTION(Server, Reliable)
	void ServerReload();//服务器上的换弹
	
	//结束换弹动画后，触发子弹数量更新的逻辑
	//用在AM_Reload中的动画通知
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	
	//散弹枪结束换弹动画后，触发子弹数量更新的逻辑
	//用在AM_Reload中的动画通知
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void HandleReload();
	//计算一下需要装多少子弹
	//这里会判断当携带子弹数量不够的情况
	int32 AmountToReload();
public:
	//散弹枪在满足各中换弹终止后，跳转到最后ShotgunEnd通知的方法
	//有几种情况：1、正常换弹结束后，2、携带弹药为0，换弹需要终止
	void JumpToShotgunEnd();
	
	
/*
* 投掷手雷相关的逻辑
*/
public:
	void ThrowGrenade();//投掷手雷
	
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass;
	
	//结束投掷动画后，触发手雷相关更新的逻辑
	//用在AM_ThrowGrenade中的动画通知
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	void ShowAttachGrenade(bool bShowGrenade);

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

public:
	//拾取弹药
	//用来做弹药拾取，数量的更新操作
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);
	
/*
 * 瞄准的全部逻辑
 */
protected:	
	void SetAiming(bool bIsAiming);//瞄准的方法

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	
/*
 *主要实现当开火键按下时的逻辑
 */
	//完整的开火逻辑
	void Fire();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	
	//服务端上进行的开火逻辑  ,传入参数为命中的位置FVector
	//这里使用FVector_NetQuantize进行一定的网络优化
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	//多播的开火逻辑  ,传入参数为命中的位置FVector
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	
	//进行射线检测
	//调用Character 和 Weapon中的fire方法
	void FireButtonPressed(bool bPressed);
	
	//用于检测命中目标
	void TraceUnderCrosehairs(FHitResult& TraceHitResult);

	//设置准星
	void SetHUDCrosshair(float DeltaTime);
	
	FTimerHandle FireTimer;
	bool bCanFire = true;
	//开始开火计时器
	void StartFireTimer();
	//结束开火计时器
	void FireTimerFinished();
	//只用来判断当前是否可以开火的方法
	bool CanFire();
	

private:
	//角色实例
	UPROPERTY()
	ABlasterCharacter* Character;
	//角色控制器实例
	UPROPERTY()
	ABlasterPlayerController* Controller;
	//角色HUD实例
	UPROPERTY()
	ABlasterHUD* HUD;

	//已经装备上的武器
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	//是否在瞄准
	UPROPERTY(Replicated)
	bool bAiming;

	//基础移动速度
	UPROPERTY(EditAnywhere , Category= "战斗设置", meta=( AllowPrivateAccess = true))
	float BaseWalkSpeed = 600.f;

	//瞄准下的移动速度
	UPROPERTY(EditAnywhere , Category= "战斗设置", meta=( AllowPrivateAccess = true))
	float AimWalkSpeed = 450.f;

	//是否按下开火键
	bool bFireButtonPressed;

	//准星在移动下的扩散量
	float CrosshairVelocityFactor;
	//准星在掉落时的扩散量
	float CrosshairInAirFactor;
	//准星在瞄准时的扩散量
	float CrosshairAimFactor;
	//准星在射击时的扩散量
	float CrosshairShootingFactor;

	FVector HitTarget;
	FHUDPackage HUDPackage;

	//不瞄准时候的视野
	float DefaultFOV;
	//瞄准时候的默认视野
	UPROPERTY(EditAnywhere, Category= "战斗设置")
	float ZoomedFOV = 30.f;

	//当前视野值
	float CurrentFOV;
	//视野切换速度
	UPROPERTY(EditAnywhere, Category= "战斗设置")
	float ZoomInterpSpeed= 20.f;

	void InterpFOV(float DeltaTime);
	
	
/*
* 弹药更新相关逻辑
*/
	//初始化弹药，如果有新增类型的武器，需要在这里添加
	void InitializeCarriedAmmo();
	//子弹计算逻辑
	void UpdateAmmoValues();
	//用来专门更新散弹枪上弹的逻辑
	void UpdateShotgunAmmoValues();

	//Carried ammo for the currently-equipped weapon
	//为当前装备的武器携带弹药
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;
	
	//为当前装备的武器携带弹药的复制方法
	UFUNCTION()
	void OnRep_CarriedAmmo();
	
	//用来存放武器类型与武器子弹数量的map
	//不直接写当前武器有多少子弹，而是通过武器类型来区分
	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;//单种弹药携带最大数量

	//角色携带AR步枪子弹的数量
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 300;
	
	//角色携带火箭弹的数量
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 30;

	//角色携带手枪子弹的数量
	UPROPERTY(EditAnywhere)
	int32 PistolAmmo = 120;

	//角色携带冲锋枪子弹的数量
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 120;

	//角色携带来复枪子弹的数量
	UPROPERTY(EditAnywhere)
	int32 StartingShotGunAmmo = 20;

	//角色携带狙击枪子弹的数量
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 10;

	//角色携带榴弹子弹的数量
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeAmmo = 10;

	//当前手雷数量
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();
	
	//最大手雷数量
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 10;

	void UpdateHUDGrenades();

/*
* 战斗状态更新相关逻辑
*/
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;
	//复制CombateState变化的方法
	UFUNCTION()
	void OnRep_CombatState();

public:
	FORCEINLINE int32 GetGrenades() const {return Grenades;}
	bool ShouldSwapWeapons();
	
		
};
