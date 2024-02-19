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
	EWS_Dropped UMETA(DisplayName = "Dropped State"),//武器被丢弃状态
	
	EWS_MAX UMETA(DisplayName = "DefaultMAX")//武器状态默认值
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
	void SetWeaponState(EWeaponState State);
	void AddAmmo(int32 AmmoToAdd);//给武器添加子弹的方法

	
	FORCEINLINE USphereComponent* GetAreaSphere() const { return  AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; } //获取当前子弹数量(斜杠前面的数值)
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; } //获取最大子弹数量(弹夹容量 斜杠前面的数值)
	

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
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
private:
	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter = nullptr;
	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController = nullptr;
	
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
	UPROPERTY(EditAnywhere, Category= "武器属性", ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();
	//进行一轮射击，里面包含了消耗子弹的逻辑
	void SpendRound();
	
	//最大子弹数量(弹夹容量)
	UPROPERTY(EditAnywhere, Category= "武器属性")
	int32 MagCapacity;

	UPROPERTY(EditAnywhere, Category= "武器属性")
	EWeaponType WeaponType;
};