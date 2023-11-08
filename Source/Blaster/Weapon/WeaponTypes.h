#pragma once

//武器类型的枚举
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_Rifle UMETA(DisplayName = "EWT_Rifle"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX"),
};