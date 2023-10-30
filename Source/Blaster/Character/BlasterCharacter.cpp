// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blaster/Blaster.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/LowLevelTestAdapter.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//初始化摄像机臂和摄像机
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 350.f;
	CameraBoom->bUsePawnControlRotation = true;


	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;//这一段感觉没有必要

	bUseControllerRotationYaw = false;//在没有武器的情况下，我们不希望我们的角色与我们的控制器一起旋转
	GetCharacterMovement()->bOrientRotationToMovement = true;//让角色朝运动方向旋转

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);//指定组件是可以被复制的 网络相关

	//蹲下的设置，这里我们使用角色移动组件里面的蹲下功能
	//设置可以蹲下
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	//设置蹲下后碰撞胶囊体的高度
	GetCharacterMovement()->SetCrouchedHalfHeight(60.f);
	//设置蹲下后的移动速度
	GetCharacterMovement()->MaxWalkSpeedCrouched = 100.f;

	//将碰撞盒设置为摄像机忽略，避免镜头臂扫到其他角色时，会莫名其妙的缩短
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	//将网格体设置为摄像机忽略，避免镜头臂扫到其他角色时，会莫名其妙的缩短
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	//设置转身速率
	GetCharacterMovement()->RotationRate = FRotator(0, 850, 0);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputContext, 0);
		}
	}
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//这里判断是否旋转根骨骼
	//角色在本地/远程网络上下文中的网络角色
	//如果当前是代理的状态，无论是
	if(GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if(TimeSinceLastMovementReplication > 0.25)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
	
	HideCameraIfCharacterClose();
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0;
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//增强输入的绑定方式
	//使用CastChecked<>转换时，如果转换失败，则会抛出一个异常，Cast<>则会返回空指针
	if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Completed, this, &ThisClass::Move);
		
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &ThisClass::Look);
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
		
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::FireButtonPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &ThisClass::FIreButtonReleased);
		
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &ThisClass::EKeyPressed);
		
		EnhancedInputComponent->BindAction(Crouching, ETriggerEvent::Started, this, &ThisClass::CrouchKeyPressed);//注意这里是ETriggerEvent::Started
		
		EnhancedInputComponent->BindAction(Aiming,ETriggerEvent::Started, this, &ThisClass::AimKeyPressed);
		EnhancedInputComponent->BindAction(Aiming,ETriggerEvent::Completed, this, &ThisClass::AImKeyReleased);
		
	}
}

void ABlasterCharacter::Jump()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//注册需要复制的变量，这个将显示给所有客户端
	//DOREPLIFETIME(ABlasterCharacter, OverlappingWeapon);

	//注册需要复制的变量，使用COND_OwnerOnly，选择只会复制到所有者的客户端
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	const FVector2d MovementVector = Value.Get<FVector2d>();

	/*使用前后左右来移动角色
	const FVector Forward = GetActorForwardVector();
	AddMovementInput(Forward, MovementVector.Y);

	const FVector Right = GetActorRightVector();
	AddMovementInput(Right, MovementVector.X);
	*/

	//使用旋转矩阵来移动角色
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
	
}

void ABlasterCharacter::Look(const FInputActionValue& Value)
{
	const FVector2d LookAxisVector = Value.Get<FVector2d>();

	AddControllerPitchInput(LookAxisVector.Y);//以x轴，向上下转动
	AddControllerYawInput(LookAxisVector.X);//以z轴，向左右转动
}

void ABlasterCharacter::FireButtonPressed(const FInputActionValue& Value)
{
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FIreButtonReleased(const FInputActionValue& Value)
{
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::EKeyPressed(const FInputActionValue& Value)
{
	if(Combat)
	{
		if(HasAuthority())//如果是拥有权限的服务器端，则执行下面进行武器装备
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else//如果不是，则调用函数来执行
		{
			ServerEquipButtonPressed();
		}
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if(Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::CrouchKeyPressed(const FInputActionValue& Value)
{
	//在Character类中，UE实现了 Crouch的网络复制的申明，我们可以直接重写下面的方法
	//virtual void OnRep_IsCrouched();
	// bIsCrouched:1
	// 系统封账的蹲下可以自动调整碰撞盒高度和移动速度
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::AimKeyPressed(const FInputActionValue& Value)
{
	if(Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AImKeyReleased(const FInputActionValue& Value)
{
	if(Combat)
	{
		Combat->SetAiming(false);
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if(Combat && Combat->EquippedWeapon == nullptr) return;
	
	float Speed = CalculateSpeed();
	
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	

	if(Speed == 0.f && !bIsInAir) // 当没有移动并且没有在空中（跳跃）的时候，
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f); //获取当前控制器镜头的旋转Yaw
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation); 
		AO_Yaw = DeltaAimRotation.Yaw;
		if(TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true; //将Yaw的控制设置为false，这样转动镜头不会同时转动角色
		TurnInPlace(DeltaTime);
	}
	
	if(Speed >0.f || bIsInAir) // 当在移动或者在空中（跳跃）的时候，
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);  //将StartingAimRotation设置为控制器镜头的旋转
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;  //将Yaw控制设置为true，这样镜头旋转会带动角色旋转
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	
	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if(AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch form [270, 360) to [-90, 0)
		//因为存在数据压缩，会导致负数变为整数，结果就是从-90 到 0度，会变成 270到360度
		//所以我们需要使用FMath::GetMappedRangeValueClamped  将一个值到赢外一个值进行映射和钳制
		FVector2d InRange(270.f, 360.f);
		FVector2d OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if(Combat == nullptr && Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if(Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if(FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if(ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if(ProxyYaw > -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw >90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if(TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if(FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if(!IsLocallyControlled()) return;
	//当摄像机距离和角色距离小于阈值时，说明已经靠墙了
	if((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);//隐藏角色的模型
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())//隐藏武器的模型
		{
			//Combat->EquippedWeapon->GetWeaponMesh()->SetVisibility(false);
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;//只对拥有者不可见
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);//隐藏角色的模型
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())//隐藏武器的模型
			{
			//Combat->EquippedWeapon->GetWeaponMesh()->SetVisibility(false);
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;//只对拥有者不可见
			}
	}
}

void ABlasterCharacter::MuticastHit_Implementation()
{
	PlayHitReactMontage();
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;
	
	//使用IsLocallyControlled()，判断当前是否在本地控制的人物
	if(IsLocallyControlled())
	{
		if(OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(Combat)
	{
		Combat->Character = this;
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if(Combat == nullptr)  return nullptr;
	return Combat->EquippedWeapon;
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if(Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}


void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}
