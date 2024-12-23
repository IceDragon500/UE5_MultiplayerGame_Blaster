﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blaster/Blaster.h"
#include "Blaster/BlasterTypes/Announcement.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerStart/TeamPlayerStart.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"


// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//控制在与其他物体发生碰撞时，如何生成这个pawn,使用AdjustIfPossibleButAlwaysSpawn表示可以调整位置，但是始终生成
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

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

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);

	//我们计划是只在服务器上使用它，所以没有必要让他成为复制的组件
	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

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
	//GetCharacterMovement()->RotationRate = FRotator(0, 850, 0);
	GetCharacterMovement()->RotationRate = FRotator(0, 0, 850);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	//创建溶解特效的时间轴
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	//创建手雷
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	/*
	 * Hit boxes for server-side rewind
	 */

	headbox = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	headbox->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("Head"), headbox);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);
	
	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);
	
	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);
	
	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);
	
	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);
	
	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);
	
	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);
	
	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);
	
	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);
	
	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for(auto Box : HitCollisionBoxes)
	{
		if(Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//注册需要复制的变量，这个将显示给所有客户端
	//DOREPLIFETIME(ABlasterCharacter, OverlappingWeapon);

	//注册需要复制的变量，使用COND_OwnerOnly，选择只会复制到所有者的客户端
	DOREPLIFETIME_CONDITION(ThisClass, OverlappingWeapon, COND_OwnerOnly);

	//注册血量
	DOREPLIFETIME(ThisClass, Health);
	//注册护盾值
	DOREPLIFETIME(ThisClass, Shield);
	DOREPLIFETIME(ThisClass, bDisableGameplay);
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::Elim(bool bPlayerLeftGame)
{
	DropOrDestroyWeapons();
	MulticastElim(bPlayerLeftGame);
}

void ABlasterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed = true;
	//实现玩家淘汰之后的逻辑
	PlayElimMontage();

	//Start dissolve effect
	//播放淘汰的特效和动画
	if(DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	//disable character movement
	//禁用移动
	GetCharacterMovement()->StopMovementImmediately();//禁止通过鼠标旋转角色
	
	bDisableGameplay = true;//禁止角色输入
	GetCharacterMovement()->DisableMovement();//禁止移动
	if(Combat)  //这一段在教程里面启用了132课，但是我这边先不用
	{
		Combat->FireButtonPressed(false);
	}
	
	//Disable Collision
	//关闭碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Spawn elim bot
	//生成被淘汰时的机器人特效和音效
	if(ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z +200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
			);
	}
	if(ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
			);
	}
	//这里判断当玩家使用狙击枪正在瞄准时，被淘汰，需要关闭瞄准镜界面
	bool bHideSniperScope = IsLocallyControlled() &&
		Combat &&
		Combat->bAiming &&
		Combat->EquippedWeapon &&
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle; 
	if(bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}

	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}

	GetWorldTimerManager().SetTimer(
	ElimTimer,
	this,
	&ThisClass::ElimTimerFinished,
	ElimDelay
	);
}

void ABlasterCharacter::ElimTimerFinished()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	//这里用bLeftGame区分玩家是被淘汰，还是玩家退出了游戏
	if(BlasterGameMode && !bLeftGame)
	{
		//如果玩家是被淘汰了，最后需要复活玩家重新进入游戏
		BlasterGameMode->RequestRespawn(this, Controller);
	}
	if(bLeftGame && IsLocallyControlled())
	{
		//如果这里是玩家退出了游戏，则需要检查积分这些东西
		OnLeftGame.Broadcast();
	}
}

void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	BlasterPlayerState = BlasterPlayerState==nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if(BlasterGameMode && BlasterPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
	}
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlasterCharacter::DropOrDestroyWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
		if (Combat->TheFlag)
		{
			Combat->TheFlag->Dropped();
		}
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();
	if(ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	if(Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
	
}

void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if(CrownSystem == nullptr) return;
	if(CrownSystem)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetCapsuleComponent(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
			);
	}
	if(CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void ABlasterCharacter::SetTeamColor(ETeam Team)
{
	if(GetMesh()==nullptr || OriginalMaterial == nullptr) return;
	
	switch (Team) {
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDissolveMatInst;
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, OriginalMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, OriginalMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	case ETeam::ET_Max:
		break;
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//这段代码修复了一个问题
	//当GameMode中启用延迟开始时 bDelayStart = true
	//会导致作为服务器的客户端无法操作，是因为上面的PlayerController转换失败了，导致if里面的增强输入没有生效
	//解决办法是用如下的if判断
	if(const ULocalPlayer* Player = GEngine && GetWorld() ? GEngine->GetFirstGamePlayer(GetWorld()) : nullptr)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Player);
		if(Subsystem)
		{
			Subsystem->AddMappingContext(InputContext, 0);
		}
		else
		{
			UE_LOG(LogTemp,Warning,TEXT("增强输入Subsystem初始化失败"));
		}
	}

	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	
	UpdateHUDHealth();
	UpdateHUDShield();
	
	if(HasAuthority())
	{
		//绑定ReceiveDamage到OnTakeAnyDamage，
		//通过OnTakeAnyDamage接受伤害，执行ReceiveDamage的逻辑
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
	if(AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//修改旋转
	RotateInPlace(DeltaTime);
	
	//检测玩家是否靠墙，如果靠墙则隐藏玩家模型
	HideCameraIfCharacterClose();

	//轮询玩家状态，将分数重置为0
	PollInit();
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if(Combat && Combat->bHoldingTheFlag)
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	if (Combat && Combat->EquippedWeapon) GetCharacterMovement()->bOrientRotationToMovement = false;
	if (Combat && Combat->EquippedWeapon) bUseControllerRotationYaw = true;
	
	/*这里在教程131课，我不想调整这个
 *这里主要是在结束游戏后，关闭玩家的旋转
if(bDisableGameplay)
{
	bUseControllerRotationYaw = false;
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	return;
}
*/	

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
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//增强输入的绑定方式
	//使用CastChecked<>转换时，如果转换失败，则会抛出一个异常，Cast<>则会返回空指针
	if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Completed, this, &ThisClass::Move);
		
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &ThisClass::Look);
		
		
		EnhancedInputComponent->BindAction(PickupKeyAction, ETriggerEvent::Triggered, this, &ThisClass::EquipButtonPressed);
		EnhancedInputComponent->BindAction(Crouching, ETriggerEvent::Started, this, &ThisClass::CrouchKeyPressed);//注意这里是ETriggerEvent::Started
		EnhancedInputComponent->BindAction(Aiming,ETriggerEvent::Started, this, &ThisClass::AimButtonPressed);
		EnhancedInputComponent->BindAction(Aiming,ETriggerEvent::Completed, this, &ThisClass::AimButtonReleased);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::FireButtonPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &ThisClass::FireButtonReleased);
		EnhancedInputComponent->BindAction(ReloadKeyAction, ETriggerEvent::Triggered, this, &ThisClass::ReloadButtonPressed);
		EnhancedInputComponent->BindAction(ThrowGrenadeKeyAction, ETriggerEvent::Triggered, this, &ThisClass::ThrowGrenadePressed);
		
	}
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//初始化Combat组件
	//这里主要是对组件中的角色引用进行赋值，以下几个都相同道理
	if(Combat)
	{
		Combat->Character = this;
	}
	
	//初始化buff组件
	if(BuffComponent)
	{
		BuffComponent->Character = this;
		BuffComponent->SetInitialSpeed(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComponent->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}

	//初始化LagCompensation
	if(LagCompensation)
	{
		LagCompensation->Character = this;
		if(Controller)
		{
			LagCompensation->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
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

void ABlasterCharacter::PlayReloadMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType()) {
		case EWeaponType::EWT_AssaultRifle://自动步枪
			SectionName = FName("AssaultRifle") ;
			break;
		case EWeaponType::EWT_RocketLauncher://火箭发射器
			SectionName = FName("RocketLauncher") ;
			break;
		case EWeaponType::EWT_Rifle://半自动步枪
			SectionName = FName("Rifle") ;
			break;
		case EWeaponType::EWT_Pistol://手枪
			SectionName = FName("Pistol") ;
			break;
		case EWeaponType::EWT_SubmachineGun://冲锋枪
			SectionName = FName("Pistol") ;//继续使用手枪的换弹动画，没有问题！
			break;
		case EWeaponType::EWT_Shotgun://来复枪
			SectionName = FName("ShotGun") ;
			break;
		case EWeaponType::EWT_SniperRifle://狙击枪
			SectionName = FName("Sniper") ;
			break;
		case EWeaponType::EWT_GrenadeLauncher://榴弹发射器
			SectionName = FName("GrenadeLaunche") ;
			break;
		case EWeaponType::EWT_MAX:
			break;
		default: ;
		}
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
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

void ABlasterCharacter::ThrowGrenadePressed(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;
	if(Combat && Combat->bHoldingTheFlag) return;
	if(Combat)
	{
		Combat->ThrowGrenade();
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatroController, AActor* DamageCasuer)
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if(bElimmed || BlasterGameMode == nullptr) return; //防止在被击杀之后，播放死亡特效时被反复击杀
	Damage = BlasterGameMode->CalculateDamage(InstigatroController, Controller, Damage);

	float DamageToHealth = Damage;//扣除血量的值
	if(Shield > 0.f)
	{
		if(Shield >= Damage) //当盾比伤害多的时候，也就是伤害不足以减少health，这里就只需要扣除盾里面的值就可以了
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else//否则就是伤害比护盾值多，所以要先把护盾置0，然后扣除护盾值，再看看有多少打在Health上了
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}
	
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth); //生命值计算
	
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	//当血量为0的时候，处理淘汰的逻辑
	//获得游戏模式，然后执行游戏模式中的淘汰相关的逻辑
	if (Health == 0.f)
	{
		if(BlasterGameMode)
		{
			//获得受害者的控制器，就是当前这个类
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) :BlasterPlayerController;
			//获得攻击者的控制器，就是传入的InstigatroController，需要从AController转换到ABlasterPlayerController
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatroController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}

void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;
	const FVector2D MovementVector = Value.Get<FVector2D>();

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
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);//以x轴，向上下转动
	AddControllerYawInput(LookAxisVector.X);//以z轴，向左右转动
}

void ABlasterCharacter::EquipButtonPressed(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;
	if(Combat)
	{
		if(Combat->bHoldingTheFlag) return;
		if(Combat->CombatState == ECombatState::ECS_Unoccupied) ServerEquipButtonPressed();
		bool bSwap =Combat->ShouldSwapWeapons() &&
			!HasAuthority() &&
			Combat->CombatState == ECombatState::ECS_Unoccupied &&
			OverlappingWeapon == nullptr;
		if(bSwap)
		{
			PlaySwapMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapon;
			bFinishedSwapping = false;
		}
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if(Combat)
	{
		if(OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if(Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
	}
}

void ABlasterCharacter::CrouchKeyPressed(const FInputActionValue& Value)
{
	if(Combat && Combat->bHoldingTheFlag) return;
	if(bDisableGameplay) return;
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

void ABlasterCharacter::ReloadButtonPressed(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;
	if(Combat && Combat->bHoldingTheFlag) return;
	if(Combat)
	{
		Combat->Reload();
	}
}



void ABlasterCharacter::AimButtonPressed(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;
	if(Combat && Combat->bHoldingTheFlag) return;
	if(Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;
	if(Combat && Combat->bHoldingTheFlag) return;
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
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
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

void ABlasterCharacter::Jump()
{
	if(Combat && Combat->bHoldingTheFlag) return;
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::FireButtonPressed(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;
	if(Combat && Combat->bHoldingTheFlag) return;
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;
	if(Combat && Combat->bHoldingTheFlag) return;
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
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
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;//只对拥有者不可见
		}
		if(Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())//隐藏武器的模型
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;//只对拥有者不可见
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);//隐藏角色的模型
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())//隐藏武器的模型
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;//只对拥有者不可见
		}
		if(Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())//隐藏武器的模型
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;//只对拥有者不可见
		}
	}
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	//if(!bElimmed || Health < LastHealth)
	if(Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if(Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
	
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if(BlasterPlayerController && Combat && Combat->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void ABlasterCharacter::SpawnDefaultWeapon()
{
	//在Blaster的地图中的时候 我们才持有默认武器
	//在大厅的时候 不需要默认武器
	//所以我们需要获取一下GameMode
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	UWorld* World = GetWorld();
	if(BlasterGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if(Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

void ABlasterCharacter::OnPlayerStateInitialized()
{
	BlasterPlayerState->AddToScore(0.f);
	BlasterPlayerState->AddToDefeats(0);
	SetTeamColor(BlasterPlayerState->GetTeam());
	SetSpawnPoint();
}

void ABlasterCharacter::SetSpawnPoint()
{
	if(HasAuthority() && BlasterPlayerState->GetTeam() == ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for(auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if(TeamStart && TeamStart->GetTeam() == BlasterPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		if(TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation(
				ChosenPlayerStart->GetActorLocation(),
				ChosenPlayerStart->GetActorRotation());
		}
	}
}

void ABlasterCharacter::PollInit()
{
	if(BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if(BlasterPlayerState)
		{
			OnPlayerStateInitialized();
			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if(BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ThisClass::UpdateDissolveMaterial);
	if(DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
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

FVector ABlasterCharacter::GetHitTarget() const
{
	if(Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if(Combat == nullptr) return ECombatState::ECS_Max;
	return Combat->CombatState;
}

bool ABlasterCharacter::IsLocallyReloading() const
{
	if(Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}

bool ABlasterCharacter::IsHoldingTheFlag() const
{
	if(Combat == nullptr) return false;
	return Combat->bHoldingTheFlag;
}

ETeam ABlasterCharacter::GetTeam()
{
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if(BlasterPlayerState == nullptr) return ETeam::ET_NoTeam;
	return BlasterPlayerState->GetTeam();
}

void ABlasterCharacter::SetHoldingTheFlag(bool bHolding)
{
	if(Combat == nullptr) return;
	Combat->bHoldingTheFlag = bHolding;
}