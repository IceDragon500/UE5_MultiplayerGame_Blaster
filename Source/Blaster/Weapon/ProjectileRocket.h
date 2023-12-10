// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "ProjectileRocket.generated.h"

/**
 * 继承自子弹类的 火箭弹
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	
	AProjectileRocket();

	virtual void Destroyed() override;
	
protected:
	
	virtual void OnHit(UPrimitiveComponent* HitComp,	AActor* OtherActor,	UPrimitiveComponent* OtherComp,	FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;

	//触发计时器的事件
	void DestroyTimerFinished();
	
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	//导弹尾部的烟雾特效
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess = "true"))
	UNiagaraSystem* TrailSystem;

	//发生导弹尾部烟雾的特效组件
	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;

	//火箭飞行的声音
	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;
	
	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	//创建一个火箭飞行声音的衰减
	UPROPERTY(EditAnywhere)
	USoundAttenuation* LooppingSoundAttenuation;

	//用来控制导弹尾部烟雾消失的计时器
	FTimerHandle DestroyTimer;

	//轨迹烟雾消失的时间
	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
};
