// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"

#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	if(BlasterCharacter)
	{
		CharacterMovement = BlasterCharacter->GetCharacterMovement();
	}
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(BlasterCharacter == nullptr)//如果是空指针，则再获取一次角色实例
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	//再次确保如果取得的是空指针，那就直接返回
	if(BlasterCharacter == nullptr) return;

	FVector Velocity = CharacterMovement->Velocity;
	Velocity.Z = 0.f;
	//移动速度
	Speed = Velocity.Size();

	//是否在空中
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

	//是否加速
	bIsAccelerating = CharacterMovement->GetCurrentAcceleration().Size() > 0.f ? true : false;

	//是否装备了武器
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
}
