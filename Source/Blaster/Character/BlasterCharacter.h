// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Blaster/BlaseComponents/CombatComponent.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AWeapon;
class UWidgetComponent;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

public:
	//跳跃
	virtual void Jump() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetOverlappingWeapon(AWeapon* Weapon);

	//AActor::PostInitializeComponents: Called after the actor’s components have been initialized, only during gameplay and some editor previews
	//在角色的组件初始化后调用，仅在游戏和某些编辑器预览时调用
	//用这个来初始化CombatComponent相关的变量
	virtual void PostInitializeComponents() override;

	//返回是否已经装备武器
	bool IsWeaponEquipped();

	//返回是否在瞄准
	bool IsAiming();

	FORCEINLINE float GetAO_Yaw() const { return  AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }

	AWeapon* GetEquippedWeapon();
	
protected:
	//增强输入
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputMappingContext* InputContext;

	//增强输入-移动
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputAction* MovementAction;
	void Move(const FInputActionValue& Value);

	
	//增强输入-鼠标
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputAction* LookAction;
	void Look(const FInputActionValue& Value);

	//增强输入-跳
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputAction* JumpAction;

	//增强输入-鼠标左键攻击
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputAction* AttackAction;
	void AttackKeyPressed(const FInputActionValue& Value);

	//增强输入-E键功能
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputAction* EKeyAction;
	void EKeyPressed(const FInputActionValue& Value);

	//增强输入-蹲
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputAction* Crouching;
	void CrouchKeyPressed(const FInputActionValue& Value);

	//增强输入-瞄准
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputAction* Aiming;
	void AimKeyPressed(const FInputActionValue& Value);
	void AImKeyReleased(const FInputActionValue& Value);

	void AimOffset(float DeltaTime);

private:
	//创建一个OnRep的方法，给需要复制的变量指定这个方法
	//是一个接收到服务器replicate后需要执行的函数，replicate过程是单向的，所以On_RepXXX只能再客户端执行
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();
	

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	//创建拾取提示组件
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	//使用ReplicatedUsing = OnRep_OverlappingWeapon，表示我们在这个变量上调用OnRep_OverlappingWeapon这个方法
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	//创建战斗组件CombatComponent
	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;


	float AO_Yaw;
	float AO_Pitch;
	FRotator  StartingAimRotation;
};
