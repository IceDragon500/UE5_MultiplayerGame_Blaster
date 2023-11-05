﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Blaster/BlaseComponents/CombatComponent.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "GameFramework/Character.h"
#include "Blaster/Interface/InteractWithCrosshairsInterface.h"
#include "BlasterCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AWeapon;
class UWidgetComponent;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
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

	virtual void OnRep_ReplicatedMovement() override;

	//返回是否已经装备武器
	bool IsWeaponEquipped();

	//返回是否在瞄准
	bool IsAiming();

	FORCEINLINE float GetAO_Yaw() const { return  AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }

	AWeapon* GetEquippedWeapon();

	//播放开火动画
	void PlayFireMontage(bool bAiming);
	//播放死亡动画
	void PlayElimMontage();

	FVector GetHitTarget() const;

	void Elim();
	//被淘汰之后的逻辑
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();
	
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
	void FireButtonPressed(const FInputActionValue& Value);
	void FIreButtonReleased(const FInputActionValue& Value);

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
	float CalculateSpeed();
	void CalculateAO_Pitch();

	//用来计算瞄准偏移
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();

	//用来播放受击动画
	void PlayHitReactMontage();

	//受到伤害
	//收到伤害的方法
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatroController, AActor* DamageCasuer);

	//更新显示血量的HUD
	void UpdateHUDHealth();

private:
	//创建一个OnRep的方法，给需要复制的变量指定这个方法
	//是一个接收到服务器replicate后需要执行的函数，replicate过程是单向的，所以On_RepXXX只能再客户端执行
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	//用来处理原地转身
	void TurnInPlace(float DeltaTime);

	//用来处理靠墙时，隐藏角色模型
	void HideCameraIfCharacterClose();
	//摄像机靠墙阈值
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	

private:
	UPROPERTY(VisibleAnywhere);
	ABlasterPlayerController* BlasterPlayerController;
	
	UPROPERTY(VisibleAnywhere, Category = "设置|摄像机")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "设置|摄像机")
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
	
	float AO_Yaw;//瞄准偏移Yaw
	float InterpAO_Yaw;//用来计算转身的Yaw
	float AO_Pitch;//瞄准偏移Pitch
	FRotator  StartingAimRotation;//用来计算左手相较于右手的位置变化

	ETurningInPlace TurningInPlace;//转身的状态

	//开火的动画
	UPROPERTY(EditAnywhere, Category= "设置|战斗")
	UAnimMontage* FireWeaponMontage;

	//受击动画
	UPROPERTY(EditAnywhere, Category= "设置|战斗")
	UAnimMontage* HitReactMontage;
	
	//死亡动画
	UPROPERTY(EditAnywhere, Category= "设置|战斗")
	UAnimMontage* ElimMontage;

	//是否旋转根骨骼
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;//上次运动复制后的时间

	//角色健康值
	UPROPERTY(EditAnywhere, Category= "设置|角色状态")
	float MaxHealth = 100.f;

	//当前血量
	//将其绑定至OnRep_Health方法，进行网络复制
	UPROPERTY(ReplicatedUsing= OnRep_Health, VisibleAnywhere, Category= "设置|角色状态")
	float Health = 100.f;

	//我们需要在血量值变化的时候，进行复制
	UFUNCTION()
	void OnRep_Health();

	//是否被淘汰
	bool bElimmed = false;

	//设置复活的定时器
	FTimerHandle ElimTimer;
	//复活的时间间隔
	UPROPERTY(EditAnywhere)
	float ElimDelay = 3.f;

	//定时器到时间之后调用的方法
	void ElimTimerFinished();
	
	
};
