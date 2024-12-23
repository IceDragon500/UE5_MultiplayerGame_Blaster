﻿#pragma once

/*
 * 用于设置在战斗组件CombatComponent中 用于区分当前持枪的战斗状态
 * 
 */

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"), //无
	ECS_Reloading UMETA(DisplayName = "Reloading"),  //换弹中
	ECS_ThrowingGrenade UMETA(DisplayName = "ThrowingGrenade"),  //投掷手雷
	ECS_SwappingWeapon UMETA(DisplayName = "SwappingWeapon"),  //其他
	
	ECS_Max UMETA(DisplayName = "DefaultMAX")
};
