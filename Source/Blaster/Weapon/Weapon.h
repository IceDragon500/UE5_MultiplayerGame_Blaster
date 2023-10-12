// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	// Sets default values for this actor's properties
	AWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	
protected:


	
private:

	UPROPERTY(VisibleAnywhere, Category= "武器属性")
	USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, Category= "武器属性")
	USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category= "武器属性")
	EWeaponState WeaponState;

};

