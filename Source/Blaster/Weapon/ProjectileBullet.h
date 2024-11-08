// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
/*
 * 继承自子弹类，实现了一个 步枪武器发射的子弹
 */
#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

public:

	AProjectileBullet();

	/**
	 * #if WITH_EDITOR 和 #endif 是预处理器指令，用于判断是否在编辑器环境下编译代码。
	 * WITH_EDITOR 是一个宏定义，通常在使用 Unreal Editor 编译项目时会被定义为 true。
	 * 如果不是在编辑器环境下（例如，在游戏运行时），则不会编译这两行之间的代码。
	 */
#if WITH_EDITOR	
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& Event) override;
#endif
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;



private:
	
};
