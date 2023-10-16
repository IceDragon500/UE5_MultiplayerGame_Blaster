// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 
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

	//是否蹲下
	UPROPERTY(BlueprintReadOnly, Category="角色", meta=(AllowPrivateAccess = "true"))
	bool bIsCrouched;
};
