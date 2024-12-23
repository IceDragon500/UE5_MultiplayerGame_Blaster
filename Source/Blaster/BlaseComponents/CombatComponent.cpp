﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Shotgun.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	//设置该组件在游戏启动时初始化，并在每一帧打勾。 如果不需要这些功能
	// off to improve performance if you don't need them.
	//如果不需要，可以关闭这些功能以提高性能。

	//这里设为false，默认不开启tick事件，我们在需要的时候再启用
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//在这里注册需要复制的值，这样才能与对应的Rep方法
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
	DOREPLIFETIME(UCombatComponent, TheFlag);
	DOREPLIFETIME(UCombatComponent, bHoldingTheFlag);
}

void UCombatComponent::ShotgunShellReload()
{
	if(Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if(CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0 , MaxCarriedAmmo);//把拾取后的弹药数量，限制在最大范围内

		UpdateCarriedAmmo();
	}
	if(EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		DefaultFOV = Character->GetFollowCamera()->FieldOfView;
		CurrentFOV = DefaultFOV;
		if(Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		//每一帧都设置准星
		SetHUDCrosshair(DeltaTime);

		//设置视野缩放
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;  //将bool置为与bPressed相同
	if(bFireButtonPressed && EquippedWeapon)  //如果开火键按下
	{
		Fire();
	}
}
/*
 *主要实现当开火键按下时的逻辑
 */
void UCombatComponent::Fire()
{
	if(CanFire())
	{
		bCanFire = false;
		
		if(EquippedWeapon)//修改开火时的准星扩张程度
		{
			CrosshairShootingFactor = 0.2f;
			
			switch(EquippedWeapon->FireType) {
			case EFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Projectile:
				FireProjectileWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgun();
				break;
			case EFireType::EFT_MAX:
				break;
			}
		
		}
		StartFireTimer();
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if(EquippedWeapon)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if(Character && !Character->HasAuthority())//这一段是177讲下方的问答中，有人给出了一次按键两次设计的解决方式
		{
			LocalFire(HitTarget); //在本地播放枪口火焰和音效
		}
		ServerFire(HitTarget);  //对命中目标点进行开火逻辑 这一步需要放在服务器上做
	}
}

void UCombatComponent::FireHitScanWeapon()
{
	if(EquippedWeapon)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if(Character && !Character->HasAuthority())//这一段是177讲下方的问答中，有人给出了一次按键两次设计的解决方式
		{
			LocalFire(HitTarget); //在本地播放枪口火焰和音效
		}
		ServerFire(HitTarget);  //对命中目标点进行开火逻辑 这一步需要放在服务器上做
	}
}

void UCombatComponent::FireShotgun()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if(Shotgun)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotGunTraceEndWithScatter(HitTarget,HitTargets);
		if(Character && !Character->HasAuthority())//这一段是177讲下方的问答中，有人给出了一次按键两次设计的解决方式
		{
			ShotgunLocalFire(HitTargets);
		}
		ServerShotgunFire(HitTargets);
	}
}

void UCombatComponent::StartFireTimer()
{
	if(EquippedWeapon == nullptr || Character == nullptr) return;
	//传入计时器、当前这个类、到时间之后调用的函数，时间间隔
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&ThisClass::FireTimerFinished,
		EquippedWeapon->FireDelay
		);
}

void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;
	if(bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	ReloadEmptyWeapon();//如果开火后发现子弹打空了，则自动触发换弹
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//如果角色是本地控制并且没有权限，那我们知道这是由开火玩家控制的角色
	if(Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	//如果角色是本地控制并且没有权限，那我们知道这是由开火玩家控制的角色
	if(Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	ShotgunLocalFire(TraceHitTargets);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr) return;
	if(Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);  //执行角色身上的开火逻辑：播放开火动画
		EquippedWeapon->Fire(TraceHitTarget); //执行武器上的开火逻辑：播放武器开火动画和特效音效
	}
}

void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	
	if(Shotgun == nullptr || Character == nullptr) return;
	if(CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	//判断是否有效
	if(Character == nullptr || WeaponToEquip == nullptr) return;
	if(CombatState != ECombatState::ECS_Unoccupied ) return;

	if(WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Flag)
	{
		Character->Crouch();
		bHoldingTheFlag = true;
		WeaponToEquip->SetWeaponState(EWeaponState::EWS_Equipped);
		AttackFlagToLeftHand(WeaponToEquip);
		WeaponToEquip->SetOwner(Character);
		TheFlag = Cast<AFlag>(WeaponToEquip);
	}
	else
	{
		if(EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
		{
			EquipSecondaryWeapon(WeaponToEquip);
		}
		else
		{
			EquipPrimaryWeapon(WeaponToEquip);
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::SwapWeapons()
{
	if(CombatState != ECombatState::ECS_Unoccupied || Character == nullptr) return;

	//首先播放换武器的动画 并修改当前CombatState的状态
	Character->PlaySwapMontage();
	CombatState = ECombatState::ECS_SwappingWeapon;
	Character->bFinishedSwapping = false;

	/*//通过TempWeapon，将当前武器和第二个武器进行交换
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;*/

	if(SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(false);
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if(WeaponToEquip == nullptr) return;
	
	//如果当前手上有武器（就是不是空指针），则执行武器的DropEquippedWeapon方法
	DropEquippedWeapon();

	//将装备的武器设置为传入的WeaponToEquip
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	//将武器EquippedWeapon附加在右手上
	AttachActorToRightHand(EquippedWeapon);

	EquippedWeapon->SetOwner(Character);//设置武器的拥有者 为当前玩家
	EquippedWeapon->SetHUDAmmo();//显示武器当前的弹药至界面上

	//更新一下弹药
	UpdateCarriedAmmo();

	//播放装备武器的音效
	PlayEquipWeaponSound(WeaponToEquip);

	//如果装备的武器是空子弹，则还需要重新换弹
	ReloadEmptyWeapon();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if(WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(WeaponToEquip);
	PlayEquipWeaponSound(WeaponToEquip);//播放装备武器的音效

	SecondaryWeapon->SetOwner(Character);//设置武器的拥有者 为当前玩家	
}

void UCombatComponent::OnRep_Aiming()
{
	if(Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UCombatComponent::DropEquippedWeapon()
{
	if(EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if(Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr ) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttackFlagToLeftHand(AWeapon* Flag)
{
	if(Character == nullptr || Character->GetMesh() == nullptr || Flag == nullptr ) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("FlagSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(Flag, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if(Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr) return;
	bool bUsePistolSocket =
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;
	FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if(HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if(Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr ) return;

	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if(BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if( EquippedWeapon == nullptr ) return;
	//如果CarriedAmmoMap中可以找到装备武器的类型，那我们就可以从map中找到武器类型对应的CarriedAmmo的数量
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	//将当前CarriedAmmo设置显示至界面
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}
void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	//播放装备武器的声音
	if(Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
			Character->GetActorLocation()
			);
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	//如果拾取的武器是空子弹的，但是身上有携带有子弹，则执行换弹
	if(EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

/*
* 武器换弹相关的逻辑
*/
void UCombatComponent::Reload()
{
	//如果当前子弹是满的，则直接返回，避免触发一个空的换弹动作
	if(CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReloading)
	{
		ServerReload();
		HandleReload();//这里的调整在185课中实现
		bLocallyReloading = true;
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if(Character == nullptr || EquippedWeapon == nullptr ) return;
	//将状态更改为换弹
	CombatState = ECombatState::ECS_Reloading;
	if(!Character->IsLocallyControlled()) HandleReload();
	
}

void UCombatComponent::FinishReloading()
{
	if(Character == nullptr) return;
	bLocallyReloading = false;
	
	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		//子弹计算逻辑
		UpdateAmmoValues();
	}
	if(bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::FinishSwap()
{
	if(Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
	if(Character) Character->bFinishedSwapping = true;
	if(SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(true);
}

void UCombatComponent::FinishSwapAttachWeapons()
{
	//修改当前武器的武器状态
	//将其附加在右手上
	//更新界面上的数值
	//播放换武器的音效

	PlayEquipWeaponSound(SecondaryWeapon);
	if(Character == nullptr || !Character->HasAuthority()) return;
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
	
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	

	//修改第二武器的状态
	//将其附加在背上
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::UpdateAmmoValues()
{
	//首先判断角色和武器是否为空
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	//通过AmountToReload方法获取，需要填充多少子弹
	int32 ReloadAmount = AmountToReload();
	//通过武器类型查询当前武器 角色身上携带的数量，然后从中扣除需要填充子弹的数量ReloadAmount
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	//更新HUD界面上的显示数字
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	//给武器添加子弹
	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	//专门用来处理散弹枪换弹的逻辑
	//散弹枪是一发一发进行上单，所以我们需要每次换弹都是加1，然后更新HUD，再判断是否满，如果没有满，继续处理
	if(Character == nullptr || EquippedWeapon == nullptr) return;

	//把角色身上携带的子弹数量减1
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	//更新HUD
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	//将散弹枪的子弹加1
	EquippedWeapon->AddAmmo(1);
	bCanFire = true; //当我们填充了1个子弹后，我们就希望可以开火，或者说我们希望在填充子弹的时候打断reload进行fire动作
	if(EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if(AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotGunEnd"));
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
	//ShowAttachGrenade(false);
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if(Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if(Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
				);
		}
	}
}

/*
* 战斗状态更新相关逻辑
*/
void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState) {
	case ECombatState::ECS_Unoccupied:
		if(bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_Reloading:
		if(Character && !Character->IsLocallyControlled()) HandleReload();
		break; 
	case ECombatState::ECS_ThrowingGrenade:
		if(Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	case ECombatState::ECS_SwappingWeapon:
		if(Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapMontage();
		}
		break;
	case ECombatState::ECS_Max:
		break;
	default: ;
	}
}

void UCombatComponent::HandleReload()
{
	if(Character)
	{
		Character->PlayReloadMontage();
	}
}

int32 UCombatComponent::AmountToReload()
{
	if(EquippedWeapon == nullptr) return 0;
	//取得应该填充多少子弹
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	//
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}
/*
* 投掷手雷相关的逻辑
*/
void UCombatComponent::ThrowGrenade()
{
	if(Grenades == 0) return;
	if(CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if(Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	if(Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
	if(Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if(Grenades == 0) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if(Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	if(Character == nullptr || Character->Controller == nullptr )  return;

	//如果控制器是空，则通过Character获得控制器，创建ABlasterPlayerController
	//否则那就不是空
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}

bool UCombatComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if(Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		PlayEquipWeaponSound(EquippedWeapon);//播放装备武器的声音
		EquippedWeapon->EnableCustomDepth(false);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if(SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBackpack(SecondaryWeapon);
		PlayEquipWeaponSound(EquippedWeapon);//播放装备武器的声音
	}
}

void UCombatComponent::OnRep_TheFlag()
{
	if(TheFlag && Character)
	{
		TheFlag->SetWeaponState(EWeaponState::EWS_Equipped);
		AttackFlagToLeftHand(TheFlag);
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//获得屏幕中心点的坐标
	FVector2D CrosshairLocation(ViewportSize.X/ 2.f , ViewportSize.Y / 2.0f);

	FVector CrosshairWorldPosition;//准星的世界坐标位置
	FVector CrosshairWorldDirection;//准星位置的方向
	
	//将屏幕坐标转换到游戏中空间坐标
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),//通过这个方法 获得玩家控制
		CrosshairLocation,//传入屏幕上的坐标
		CrosshairWorldPosition,//传出世界坐标位置
		CrosshairWorldDirection//传出世界坐标方向
		);

	//开始设置射线检测
	if(bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;//设置射线检测的起点 ，是上面获得由屏幕中心点坐标转换的世界坐标位置
		//这里修正一下射线检测起始的位置
		//预期是将起始位置从摄像机当前位置推到角色的位置，然后再加一点
		if(Character)
		{
			//获得角色到摄像机的距离
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			//将摄像机的距离 往前推一段距离再加100
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}
		
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;//设置射线检测的终点

		GetWorld()->LineTraceSingleByChannel(
		TraceHitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility
		);
		
		if(!TraceHitResult.bBlockingHit)//如果射线检测没有碰撞到任何东西
		{
			TraceHitResult.ImpactPoint = End;//那我们就设置碰撞到的那个点为射线的终点
		}

		//实现当检测目标是角色时，改变准星的颜色
		//判断角色， 并且判断获取到的角色是否有对应的接口
		if(TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
	if(Character == nullptr || Character->Controller == nullptr )  return;

	//如果控制器是空，则通过Character获得控制器，创建ABlasterPlayerController
	//否则那就不是空
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		HUD = HUD==nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if(HUD)
		{
			if(EquippedWeapon)
			{
				
				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairDown = EquippedWeapon->CrosshairDown;
			}
			else
			{
				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairRight = nullptr;
				HUDPackage.CrosshairTop = nullptr;
				HUDPackage.CrosshairDown = nullptr;
			}
			//计算准星的扩展
			//计算运动速度与扩展的关系
			//我们将速度0~600 映射到 0~1
			//先获得角色的速度，存在一个FVector2D变量中
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			//再设置速度变量
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			//获得当前的速度
			//我们用当前的速度，对比0~600之间，映射到0~1是多少
			FVector Velocity = Character->GetVelocity();
			//获得最终映射到0~1的值
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if(Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}
			
			if(bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f ,DeltaTime, 40.f);
			
			//赋值给HUDPackage
			HUDPackage.CrosshairSpread =0.5f + CrosshairVelocityFactor +CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if(EquippedWeapon == nullptr ) return;
	if(bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if(Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
	
}

/*
 * 瞄准的全部逻辑
 */
void UCombatComponent::SetAiming(bool bIsAiming)
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed ;
	}
	if(Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
	//如果角色是本地控制的，则需要改变设置本地的bAimButtonPressed与服务器同步的bIsAiming一致
	if(Character->IsLocallyControlled()) bAimButtonPressed = bIsAiming;
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed ;
	}
}

bool UCombatComponent::CanFire()
{
	//当没有装备武器的时候，不能发射，返回false
	if(EquippedWeapon == nullptr) return false;
	
	//这个判断是为了散弹枪装弹时可以进行射击的特殊判断
	//当装备的是散弹枪并且当前处在reload状态，并且bCanFire为true（说明这个时候散弹枪至少有一颗子弹），则返回ture
	if(!EquippedWeapon->IsEmpty()&& bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) return true;

	if(bLocallyReloading) return false; //如果在换弹，也返回false

	//只要当前装备了武器并且bCanFire为true，且为ECS_Unoccupied状态，就可以进行开火
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	bool bJumpToShotgunEnd =
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarriedAmmo == 0;
	if(bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}

/*
* 弹药更新相关逻辑
*/
void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, PistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotGunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeAmmo);
}