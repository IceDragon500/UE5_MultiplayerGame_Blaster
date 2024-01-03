#pragma once

#define TRACE_LENGTH 80000.f  //定义一下射线检测的长度为一个宏

//武器类型的枚举
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "自动步枪"),
	EWT_RocketLauncher UMETA(DisplayName = "火箭弹发射器"),
	EWT_Rifle UMETA(DisplayName = "半自动步枪"),
	EWT_Pistol UMETA(DisplayName = "手枪"),
	EWT_SubmachineGun UMETA(DisplayName = "冲锋枪"),
	EWT_ShotGun UMETA(DisplayName = "来复枪"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX"),
};