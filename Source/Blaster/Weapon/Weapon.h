// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Casing.h"
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
	void ShowPickupWidget(bool bShowWidget);

public:
	
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return  AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	//开火相关的逻辑
	virtual void Fire(const FVector& HitTarget);
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
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

};