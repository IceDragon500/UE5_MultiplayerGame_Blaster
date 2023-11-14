#pragma once

/*
 * 用于设置在战斗组件CombatComponent中 用于区分当前持枪的战斗状态
 * 
 */

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"), //无
	ECS_Reloading UMETA(DisplayName = "Reloading"),  //换弹中
	ECS_Other UMETA(DisplayName = "Other"),  //其他
	
	ECS_Max UMETA(DisplayName = "DefaultMAX")
};
