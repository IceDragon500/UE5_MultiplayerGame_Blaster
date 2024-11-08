// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
/*
 * 子弹父类
 */
#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Sound/SoundCue.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	

	AProjectile();
	virtual void Tick(float DeltaTime) override;
	//我们重写Destroyed这个方法，并将播放命中特效和命中音效放在这里面
	//这样我们就不用专门给两个命中逻辑做网络复制
	virtual void Destroyed() override;

	/**
	 * Used with server-side rewind
	 */

	//是否启用服务器倒带技术
	bool bUserServerSideRewind = false;
	//起始的位置
	FVector_NetQuantize TraceStart;
	//初始的速度 
	FVector_NetQuantize100 InitialVelocity;//因为速度包含了方向，因此使用了精度更高的FVector_NetQuantize100
	//初始速度
	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;
	
	float Damage = 20.f;
	
protected:

	virtual void BeginPlay() override;

	void SpawnTrailSystem();

	void StartDestroyTimer();
	
	//触发计时器的事件
	void DestroyTimerFinished();

	//爆炸伤害
	void ExplodeDamage();

	//HitComp 参与碰撞的组件，在这里就是我们的碰撞盒子
	//OtherActor 被击中的actor
	//OtherComp 被击中的组件
	//NormalImpulse 被击中表面的法线 就是垂直于命中表面的放哪股那个
	//Hit 命中结果
	//我们需要将这个函数进行绑定在我们的碰撞盒子上
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp,	AActor* OtherActor,	UPrimitiveComponent* OtherComp,	FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;
	
	//命中特效
	UPROPERTY(EditAnywhere, Category= "子弹属性")
	UParticleSystem* ImpactParticles;

	//命中音效
	UPROPERTY(EditAnywhere, Category= "子弹属性")
	USoundCue* ImpactSound;

	UPROPERTY(VisibleAnywhere, Category= "子弹属性")
	UProjectileMovementComponent* ProjectileMovementComponent;

	//导弹尾部的烟雾特效
	UPROPERTY(EditAnywhere, Category= "子弹属性")
	UNiagaraSystem* TrailSystem;

	//发生导弹尾部烟雾的特效组件
	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;

	//炸弹爆炸内圈的半径
	UPROPERTY(EditAnywhere, Category= "子弹属性")
	float DamageInnerRadius = 200.f;
	//炸弹爆炸外圈的半径
	UPROPERTY(EditAnywhere, Category= "子弹属性")
	float DamageOuterRadius = 500.f;
	
private:
	//子弹特效
	UPROPERTY(EditAnywhere, Category= "子弹属性")
	UParticleSystem* Tracer;

	//特效组件
	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

	//用来控制导弹尾部烟雾消失的计时器
	FTimerHandle DestroyTimer;

	//轨迹烟雾消失的时间
	UPROPERTY(EditAnywhere, Category= "子弹属性")
	float DestroyTime = 3.f;
};
