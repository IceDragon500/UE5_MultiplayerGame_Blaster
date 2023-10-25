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
	bFireButtonPressed = bPressed;
	
	if(bFireButtonPressed)
	{
		ServerFire();
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
		if(!TraceHitResult.bBlockingHit)//如果射线检测么有碰撞到任何东西
		{
			TraceHitResult.ImpactPoint = End;//那我们就设置碰撞到的那个点为射线的终点
			HitTarget = End;//同样也是子弹的终点
		}
		else
		{
			HitTarget = TraceHitResult.ImpactPoint;//将命中点的坐标赋值
			DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 13.f, 12, FColor::Red);
		}
	}
}

void UCombatComponent::ServerFire_Implementation()
{
	MulticastFire();
}

void UCombatComponent::MulticastFire_Implementation()
{
	if(EquippedWeapon == nullptr) return;
	if(Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(HitTarget);
	}
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	FHitResult HitResult;
	TraceUnderCrosehairs(HitResult);
}