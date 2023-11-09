// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "InputTriggers.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialExpressionChannelMaskParameterColor.h"
#include "Net/UnrealNetwork.h"

#define TRACE_LENGTH 80000.f  //定义一下射线检测的长度为一个宏

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
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
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

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Add(EWeaponType::EWT_AssaultRifle, 300);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	

	if(Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosehairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		//每一帧都设置准星
		SetHUDCrosshair(DeltaTime);

		//设置视野缩放
		InterpFOV(DeltaTime);
	}
	
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;  //将boo置为与bPressed相同
	if(bFireButtonPressed && EquippedWeapon)  //如果开火键按下
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if(CanFire())
	{
		bCanFire = false;
		ServerFire(HitTarget);  //对命中目标点进行开火逻辑
		if(EquippedWeapon)//修改开火时的准星扩张程度
			{
			CrosshairShootingFactor = 0.2f;
			}
		StartFireTimer();
	}
}

void UCombatComponent::StartFireTimer()
{
	if(EquippedWeapon == nullptr || Character == nullptr) return;
	//传入计时器、当前这个类、到时间之后调用的函数，时间间隔
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &ThisClass::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;
	if(bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
}

bool UCombatComponent::CanFire()
{
	if(EquippedWeapon == nullptr) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
	//return !EquippedWeapon->IsEmpty();
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->GetController()) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr) return;
	if(Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);  //执行角色身上的开火逻辑：播放开火动画
		EquippedWeapon->Fire(TraceHitTarget); //执行武器上的开火逻辑：播放武器开火动画和特效音效
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	//判断是否有效
	if(Character == nullptr || WeaponToEquip == nullptr) return;

	//如果当前手上有武器（就是不是空指针），则执行武器的Dropped方法
	if(EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}

	//将装备的武器设置为传入的WeaponToEquip
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket")))
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);//设置武器的拥有者 为当前玩家
	EquippedWeapon->SetHUDAmmo();//显示武器当前的弹药至界面上

	//如果CarriedAmmoMap中可以找到装备武器的类型，那我们就可以从map中找到武器类型对应的CarriedAmmo的数量
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponTyep()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponTyep()];
	}

	//将当前CarriedAmmo设置显示至界面
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->GetController()) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	
}

void UCombatComponent::Reload()
{
	if(CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading)
	{
		ServerReload();
	}
}

void UCombatComponent::FinishReloading()
{
	if(Character == nullptr) return;
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

void UCombatComponent::ServerReload_Implementation()
{	
	//将状态更改为换弹
	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

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
		HandleReload();
		break;
	case ECombatState::ECS_Other:
		break;
	case ECombatState::ECS_Max:
		break;
	default: ;
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponTyep()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponTyep()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponTyep()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->GetController()) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(-ReloadAmount);
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if(EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponTyep()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponTyep()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if(HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::TraceUnderCrosehairs(FHitResult& TraceHitResult)
{
	FVector2d ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//获得屏幕中心点的坐标
	FVector2d CrosshairLocation(ViewportSize.X/ 2.f , ViewportSize.Y / 2.0f);

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
			FVector2d WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			//再设置速度变量
			FVector2d VeloctiyMultiplierRange(0.f, 1.f);
			//获得当前的速度
			//我们用当前的速度，对比0~600之间，映射到0~1是多少
			FVector Velocity = Character->GetVelocity();
			//获得最终映射到0~1的值
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VeloctiyMultiplierRange, Velocity.Size());

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

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed ;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed ;
	}
}