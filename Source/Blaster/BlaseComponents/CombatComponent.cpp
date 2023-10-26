// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#define TRACE_LENGTH 80000.f  //定义一下射线检测的长度为一个宏

// Sets default values for this component's properties
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

	//复制已经装备的武器，当他有变化的时候会复制给所有客户端
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
	
}
// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//每一帧都设置准星
	SetHUDCrosshair(DeltaTime);

	if(Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosehairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
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
			FHUDPackage HUDPackage;
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
			
			
			//赋值给HUDPackage
			HUDPackage.CrosshairSpread = CrosshairVelocityFactor +CrosshairInAirFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
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

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	//判断是否有效
	if(Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;  //将boo置为与bPressed相同
	if(bFireButtonPressed)  //如果开火键按下
	{
		FHitResult HitResult; //创建一个FHitResult结构体接受命中检测结果
		TraceUnderCrosehairs(HitResult); //进行射线检测
		ServerFire(HitResult.ImpactPoint);  //对命中目标点进行开火逻辑
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
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr) return;
	if(Character)
	{
		Character->PlayFireMontage(bAiming);  //执行角色身上的开火逻辑：播放开火动画
		EquippedWeapon->Fire(TraceHitTarget); //执行武器上的开火逻辑：播放武器开火动画和特效音效
	}
}