// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"

#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
//------初始化角色实例-----------------------------------------------------

	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(TryGetPawnOwner()) : BlasterCharacter;
	if(BlasterCharacter == nullptr) return;//这里需要检查一下是否赋值成功了
	
//------获得旋转初始值-----------------------------------------------------
	//获得上一帧模型的旋转
	RotationLastTick = PlayerRotation;
	
	//获得控制器（镜头）的旋转
	//往右转是从0到180，往左是从-1到-180
	PlayerControlRotation = BlasterCharacter->GetBaseAimRotation(); //AimRotation
	
	//获得模型Actor的旋转
	PlayerRotation = BlasterCharacter->GetActorRotation();

//------赋值基础属性-----------------------------------------------------

	FVector Velocity = BlasterCharacter->GetCharacterMovement()->Velocity;
	Velocity.Z = 0.f;
	Speed = Velocity.Size();//移动速度
	
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();//是否在空中
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;//是否加速
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();//是否装备了武器
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;//是否蹲下
	bAiming = BlasterCharacter->IsAiming();//是否瞄准
	TurningInPlace = BlasterCharacter->GetTurningInPlace();//是否原地转身
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bElimmed = BlasterCharacter->IsElimmed();
	bHoldingTheFlag = BlasterCharacter->IsHoldingTheFlag();
	
//------计算当前运动方向（相对于控制器）Direction-----------------------------------------------------
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());

	//这里获取角色当前运动的方向，例如角色正在前进，就是0，角色正在向左运动，就是-90，
	// 角色可能已经经过了旋转，所以需要将角色旋转的角度，减去运动的旋转，这两个旋转都是以世界坐标为参考系
	// 得到的旋转，就是以角色自己为坐标的参考系
	//设置Direction方向，运动向前为0，运动向左为-90，运动向右为90， 运动向后为180或-180
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, PlayerControlRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);
	Direction = DeltaRotation.Yaw;
	
	//设置最后停下的方向StopDirection
	if (bIsAccelerating == true)
	{
		StopDirection = Direction;
	}
//------计算奔跑时，向左向右转动的倾斜度Lean-----------------------------------------------------

	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(PlayerRotation, RotationLastTick);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

//------计算Direction-----------------------------------------------------

	FRotator RotatorDeltar = UKismetMathLibrary::NormalizedDeltaRotator(PlayerControlRotation, PlayerRotation);

	//Pitch = RotatorDeltar.Pitch;
	Roll = RotatorDeltar.Roll;
	//Yaw = RotatorDeltar.Yaw;
	
	Yaw = BlasterCharacter->GetAO_Yaw();
	Pitch = BlasterCharacter->GetAO_Pitch();

	//获得当前方向（就是速度的方向）到X轴的一个旋转
	//这里得到了当Actor在世界中运动时，Actor前进方向 相对于世界坐标的旋转
	//这个方向与控制器的方向无关

//------添加一个FBRIK结算器，用来设置左手在武器上的位置-----------------------------------------------------
	if(bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(),FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		//为这个设置只需要对本地玩家来说比较精确就可以了，不用对其他玩家展示如此精细
		//因为AnimInstance是可以复制的，所以我们需要排除一下，当只有本地角色控制时才有效果
		if(BlasterCharacter ->IsLocallyControlled())
		{
			bLocallyControlled = true;
			//获得右手持枪那个骨骼的变换
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			//添加一个插值，当瞄准近距离物体的时候，骨骼切换不会那么直接
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
			//添加一个插值，当瞄准近距离物体的时候，骨骼切换不会那么直接
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 30.f);
		}
	}
	bUseFABRIK = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	bool bFABRIKOverride = BlasterCharacter->IsLocallyControlled() &&
		BlasterCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade &&
		BlasterCharacter->bFinishedSwapping;
	if(bFABRIKOverride)
	{
		bUseFABRIK = !BlasterCharacter->IsLocallyReloading() ;
	}
	bUseAimOffset = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
	bTransformRightHand  = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
}
