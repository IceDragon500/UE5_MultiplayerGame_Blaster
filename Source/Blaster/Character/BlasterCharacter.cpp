// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//初始化摄像机臂和摄像机
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
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
	//将网格体设置为摄像机忽略，避免镜头臂扫到其他角色时，会莫名其妙的缩短
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

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
		
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ThisClass::AttackKeyPressed);
		
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &ThisClass::EKeyPressed);
		
		EnhancedInputComponent->BindAction(Crouching, ETriggerEvent::Started, this, &ThisClass::CrouchKeyPressed);//注意这里是ETriggerEvent::Started
		
		EnhancedInputComponent->BindAction(Aiming,ETriggerEvent::Started, this, &ThisClass::AimKeyPressed);
		EnhancedInputComponent->BindAction(Aiming,ETriggerEvent::Completed, this, &ThisClass::AImKeyReleased);
		
	}
}

void ABlasterCharacter::Jump()
{
	Super::Jump();

	
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

void ABlasterCharacter::AttackKeyPressed(const FInputActionValue& Value)
{
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
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("AimKeyPress")));
	}
	
}

void ABlasterCharacter::AImKeyReleased(const FInputActionValue& Value)
{
	if(Combat)
	{
		Combat->SetAiming(false);
	}
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



