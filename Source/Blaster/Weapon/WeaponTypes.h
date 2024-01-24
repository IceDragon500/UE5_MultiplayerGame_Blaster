#pragma once

#define TRACE_LENGTH 80000.f  //定义一下射线检测的长度为一个宏

//武器类型的枚举
//在这里新增武器类型后，需要在以下几个地方新增对应的数据
//在CombatComponent.h中，添加新增武器 在角色身上携带子弹的数量
//在CombatComponent.h中，在InitializeCarriedAmmo()方法中，给CarriedAmmoMap添加对应子弹和武器类型的映射
//在BlasterCharacter.h中，在PlayReloadMontage()播放换弹动画的方法中，给新增的武器类型添加换弹动画的分类
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "自动步枪"),
	EWT_RocketLauncher UMETA(DisplayName = "火箭弹发射器"),
	EWT_Rifle UMETA(DisplayName = "半自动步枪"),
	EWT_Pistol UMETA(DisplayName = "手枪"),
	EWT_SubmachineGun UMETA(DisplayName = "冲锋枪"),
	EWT_ShotGun UMETA(DisplayName = "来复枪"),
	EWT_SniperRifle UMETA(DisplayName = "狙击枪"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX"),
};