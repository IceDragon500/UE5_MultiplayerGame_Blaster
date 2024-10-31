// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
/*
 * 武器父类
 */
#include "CoreMinimal.h"
#include "Casing.h"
#include "WeaponTypes.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),//武器初始化状态
	EWS_Equipped UMETA(DisplayName = "Equipped State"),//武器已装备状态
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),//第二把武器已装备状态
	EWS_Dropped UMETA(DisplayName = "Dropped State"),//武器被丢弃状态
	
	EWS_MAX UMETA(DisplayName = "DefaultMAX")//武器状态默认值
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"), //命中扫描武器 直接按照准星点命中目标的武器
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"), //射弹武器 需要射出弹丸的 带抛物线的武器
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"), //射弹武器 根据中心点，有随机散射的武器

	EFT_MAX UMETA(DisplayName = "DefaultMAX")//武器状态默认值
};



UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	

	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void ShowPickupWidget(bool bShowWidget);//显示拾取界面
	void Dropped();//扔出武器
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();	
	void SetWeaponState(EWeaponState State);//设置武器在各种情况下的状态参数
	void AddAmmo(int32 AmmoToAdd);//给武器添加子弹的方法
	FVector TraceEndWithScatter(const FVector& HitTarget);
	
	FORCEINLINE USphereComponent* GetAreaSphere() const { return  AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; } //获取当前子弹数量(斜杠前面的数值)
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; } //获取最大子弹数量(弹夹容量 斜杠前面的数值)
	FORCEINLINE float GetDamage() const { return Damage; } //获取当前武器的伤害值
	

	//开火相关的逻辑
	virtual void Fire(const FVector& HitTarget);

	UPROPERTY(EditAnywhere, Category="武器属性|准星")
	UTexture2D* CrosshairCenter;
	UPROPERTY(EditAnywhere, Category="武器属性|准星")
	UTexture2D* CrosshairLeft;
	UPROPERTY(EditAnywhere, Category="武器属性|准星")
	UTexture2D* CrosshairRight;
	UPROPERTY(EditAnywhere, Category="武器属性|准星")
	UTexture2D* CrosshairTop;
	UPROPERTY(EditAnywhere, Category="武器属性|准星")
	UTexture2D* CrosshairDown;

	//开火延迟
	UPROPERTY(EditAnywhere, Category= "武器属性")
	float FireDelay = 0.1f;
	//是否可以自动开火
	UPROPERTY(EditAnywhere, Category= "武器属性")
	bool bAutomatic = true;
	//子弹是否为空
	bool IsEmpty();
	//判断子弹是否装满，满true，不满false
	bool IsFull();

	//装备武器的声音
	UPROPERTY(EditAnywhere, Category= "武器属性")
	USoundCue* EquipSound;

	//打开自定义深度
	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon = false;


	UPROPERTY(EditAnywhere)
	EFireType FireType = EFireType::EFT_HitScan;
	
	
	//启用或者禁用散射
	UPROPERTY(EditAnywhere, Category= "Weapon Scatter")
	bool bUseScatter = false;

	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();

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

	//射线武器的伤害值
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	//作为一个开关，用来选择是否使用服务器倒带的计算方式来计算伤害
	UPROPERTY(EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter = nullptr;
	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController = nullptr;
	
private:
	
	UPROPERTY(VisibleAnywhere, Category= "武器属性")
	USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, Category= "武器属性")
	USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing=OnRep_WeaponState, VisibleAnywhere, Category= "武器属性")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	//拾取UI
	UPROPERTY(VisibleAnywhere, Category= "武器属性")
	class UWidgetComponent* PickupWidget;

	//开火动画
	UPROPERTY(EditAnywhere, Category= "武器属性")
	UAnimationAsset* FireAnimation;

	//设置抛落的弹壳
	UPROPERTY(EditAnywhere, Category= "武器属性")
	TSubclassOf<ACasing> CasingClass;

	//这把武器的镜头缩放值
	UPROPERTY(EditAnywhere, Category= "武器属性")
	float ZoomedFOV = 30.f;
	//这把武器镜头缩放速度
	UPROPERTY(EditAnywhere, Category= "武器属性")
	float ZoomInterpSpeed = 20.f;

	//当前子弹数量
	UPROPERTY(EditAnywhere, Category= "武器属性")
	int32 Ammo;

	//UFUNCTION() 这里因为我们需要使用服务器对账的方式核对Ammo 避免在有延迟的情况下出现子弹归0但是无法换弹的问题
	// 所以我们这里不在使用复制的方法复制ammo
	// 我们需要一个单独的RPC方式去处理ammo 或者说 去处理AddAmmo()方法
	//void OnRep_Ammo();
	//下面两个方法就是用来取代这里的注释内容

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);
	
	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	//the number of unprocessed server requests for ammo
	//Incremented in SpendRound, decremented in ClientUpdateAmmo.
	//未处理的服务器请求数量
	//在 SpendRound 中递增，在 ClientUpdateAmmo 中递减
	int32 AmmoSequence = 0;
	
	//进行一轮射击，里面包含了消耗子弹的逻辑
	void SpendRound();
	
	//最大子弹数量(弹夹容量)
	UPROPERTY(EditAnywhere, Category= "武器属性")
	int32 MagCapacity;

	UPROPERTY(EditAnywhere, Category= "武器属性")
	EWeaponType WeaponType;
	
};
