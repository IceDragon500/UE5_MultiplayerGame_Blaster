// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//设置该角色每帧调用一次 Tick()。 如果不需要，可以将其关闭以提高性能。
	PrimaryActorTick.bCanEverTick = false;

	//因为服务器要负责所有武器对象
	//设置为可以复制
	bReplicates = true;
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);
	
	WeaponMesh->SetCollisionResponseToChannels(ECollisionResponse::ECR_Block);//先设置对所有检测通道阻挡
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);//然后设置对Pawn（角色）忽略阻挡
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//设置没有碰撞


	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaShpere"));
	AreaSphere->SetupAttachment(RootComponent);

	AreaSphere->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//我们先把碰撞盒子关掉

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

}

void AWeapon::Fire(const FVector& HitTarget)
{
	//播放动画
	//播放开火特效 和 音效，开火特效和音效集合在了开火动画中，通过通知来进行播放
	if(FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	//TODO 抛壳
	if(CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if(AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(GetWeaponMesh()); //获得抛弹口的变换
			
			UWorld* World = GetWorld();
			if(World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,//传入需要生成的类
					SocketTransform.GetLocation(), //传入生成类的坐标
					SocketTransform.GetRotation().Rotator()//传入生成类的旋转
					);
			}
			
		}
	}
	//TODO 进行一轮的射击逻辑（比如扣除子弹数量）
	SpendRound();
	
}

bool AWeapon::IsEmpty()
{
	return Ammo <=0 ? true : false ;
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 如果当前LocalRole是否为ROLE_Authority，就是当前引擎对这个实例是否有权威性
	// if(GetLocalRole() == ENetRole::ROLE_Authority)
	// HasAuthority()和上面那个判断是一致的
	if(HasAuthority())
	{
		//如果有，我们则将武器的碰撞盒子设置为查询和物理
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//并且，将盒子碰撞与Pawn设置为重叠
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		//将碰撞方法与碰撞球体进行绑定
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this,&ThisClass::OnSphereEndOverlap);
	}

	if(PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//将WeaponState注册，这样可以复制
	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
	
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	//扔掉武器后，要将武器的Owner和几个变量置为nullptr
	//防止其他玩家拾起之后，Character和Controller出错
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if(Owner == nullptr)//如果武器当前拥有者是空，则吧控制器和玩家两个变量置空，防止出错
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if(BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->GetController()) : BlasterOwnerController;
		if(BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDAmmo(Ammo);//在Actor中，使用调用PlayerController类中的设置HUD数值的方式，来设置CharacterOverlay 
		}
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor)
	{
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
		if(BlasterCharacter)
		{
			BlasterCharacter->SetOverlappingWeapon(this);
		}
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(OtherActor)
	{
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
		if(BlasterCharacter)
		{
			BlasterCharacter->SetOverlappingWeapon(nullptr);
		}
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		/*
		 *根据我的搜索结果，这个错误通常是由于物体的“启用碰撞”属性设置为“查询”或“无碰撞”，但“模拟物理”被设置为“是”导致的。
		 *将“启用碰撞”属性更改为其他值或确保物体的“模拟物理”标志未设置即可解决此问题。
		 */
		WeaponMesh->SetSimulatePhysics(false);//这里两个需要设置成false，否则会报错
		WeaponMesh->SetEnableGravity(false);
		
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if(HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_MAX:
		break;
	default: ;
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ShowPickupWidget(true);
		break;
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_MAX:
		break;
	default: ;
	}
}

void AWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo -1, 0, MagCapacity);
	SetHUDAmmo();
}