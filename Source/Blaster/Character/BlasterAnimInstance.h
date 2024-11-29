// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Weapon/Weapon.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 角色的动画蓝图类
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	
protected:

	

private:

	//角色实例
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	class ABlasterCharacter* BlasterCharacter;

	//角色运动组件
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	class UCharacterMovementComponent* CharacterMovement;

	//角色速度
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	float Speed;

	//是否在空中
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	bool bIsInAir;

	//是否在加速
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	//是否装备了武器
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	UPROPERTY()
	AWeapon* EquippedWeapon;

	//是否蹲下
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	bool bIsCrouched;

	//是否瞄准
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	bool bAiming;
	
	//前进的方向  就是教程里面的YawOffset
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	float Direction;
	//最后停下的方向
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	float StopDirection;

	//倾斜角度
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	float Lean;

	//模型上一帧的旋转
	//教程里面是CharacterRotationLastFrame
	FRotator RotationLastTick;
	//获得控制器（镜头）的旋转 教程里面是AimRotation
	FRotator  PlayerControlRotation;
	//获得模型Actor的旋转
	//教程里面是CharacterRotation
	FRotator PlayerRotation;

	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	float Pitch = 0;
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	float Roll = 0;
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	float Yaw = 0;

	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;
	
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace;
	
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	FRotator RightHandRotation;

	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	bool bRotateRootBone;

	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	bool bElimmed;

	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	bool bUseFABRIK;

	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	bool bUseAimOffset;

	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	bool bTransformRightHand;

	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	bool bHoldingTheFlag;
};
