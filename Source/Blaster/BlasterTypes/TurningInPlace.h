#pragma once

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	ETIP_Left  UMETA(DisplayName = "Turing Left"),//向左转
	ETIP_Right  UMETA(DisplayName = "Turing Right"),//向右转
	ETIP_NotTurning UMETA(DisplayName = "Dropped State"),//没有转
	
	ETIP_MAX UMETA(DisplayName = "DefaultMAX")//旋转状态的默认值
};