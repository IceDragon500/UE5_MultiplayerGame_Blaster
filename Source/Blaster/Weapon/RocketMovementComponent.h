// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "RocketMovementComponent.generated.h"

/*
Enum indicating how simulation should proceed after HandleBlockingHit() is called.
枚举，表示调用 HandleBlockingHit() 后模拟应如何进行。
	enum class EHandleBlockingHitResult
	{
		Deflect,				 Assume velocity has been deflected, and trigger HandleDeflection(). This is the default return value of HandleBlockingHit().
                                假设速度已发生偏转，并触发 HandleDeflection()。这是 HandleBlockingHit() 的默认返回值。
		AdvanceNextSubstep,		 Advance to the next simulation update. Typically used when additional slide/multi-bounce logic can be ignored,
									such as when an object that blocked the projectile is destroyed and movement should continue.
								进入下一次仿真更新。通常用于可以忽略额外的滑动/多重弹跳逻辑时、 例如，当阻挡弹丸的物体被摧毁，运动应该继续时。
		Abort,					 Abort all further simulation. Typically used when components have been invalidated or simulation should stop.
								终止所有进一步模拟。通常用于组件已失效或模拟应停止的情况。
	}
*/


/**
	 * 处理模拟更新过程中的阻塞碰撞。检查碰撞后模拟是否仍然有效。
	 * 如果正在模拟，则调用 HandleImpact()，默认返回 EHandleHitWallResult::Deflect，以便通过 HandleDeflection()启用多重反弹和滑动支持。
	 * 如果不再模拟，则返回 EHandleHitWallResult::Abort，终止进一步模拟的尝试。
	 *
	 * @param Hit Blocking 发生的撞击。
	 * @param TimeTick 导致阻塞命中的最后一次移动的时间差。
	 * @param MoveDelta 当前子步骤的移动延迟时间。
	 * @param SubTickTimeRemaining 在当前子步骤中要继续模拟多少时间，可能会因这个函数而改变。
	 * 初始默认值为 TimeTick * (1.f - Hit.Time)
	 * @return 结果显示模拟应如何进行。
	 * 参见 EHandleHitWallResult、HandleImpact()
	 *
	 * virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining);
	 */


/**
	 * 如果启用弹跳逻辑，则应用弹跳逻辑以影响撞击时的速度（使用 ComputeBounceResult()）、
	 * 或在未启用弹跳或速度低于 BounceVelocityStopSimulatingThreshold 时停止弹射。
	 * 触发适用事件（OnProjectileBounce）。
	 * virtual void HandleImpact(const FHitResult& Hit, float TimeSlice=0.f, const FVector& MoveDelta = FVector::ZeroVector) override;
	 */

UCLASS()
class BLASTER_API URocketMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

public:
	virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;

	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice=0.f, const FVector& MoveDelta = FVector::ZeroVector) override;
	
protected:

	
private:

	
	
};
