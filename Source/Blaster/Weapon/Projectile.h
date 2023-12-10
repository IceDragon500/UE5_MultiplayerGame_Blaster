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

protected:

	virtual void BeginPlay() override;

	//HitComp 参与碰撞的组件，在这里就是我们的碰撞盒子
	//OtherActor 被击中的actor
	//OtherComp 被击中的组件
	//NormalImpulse 被击中表面的法线 就是垂直于命中表面的放哪股那个
	//Hit 命中结果
	//我们需要将这个函数进行绑定在我们的碰撞盒子上
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp,	AActor* OtherActor,	UPrimitiveComponent* OtherComp,	FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;
	
	//命中特效
	UPROPERTY(EditAnywhere, Category= "子弹属性")
	UParticleSystem* ImpactParticles;

	//命中音效
	UPROPERTY(EditAnywhere, Category= "子弹属性")
	USoundCue* ImpactSound;

	//对命中目标造成伤害
	UPROPERTY(EditAnywhere, Category= "子弹属性")
	float Damage;

	UPROPERTY(VisibleAnywhere, Category= "子弹属性")
	UProjectileMovementComponent* ProjectileMovementComponent;
	
private:
	//子弹特效
	UPROPERTY(EditAnywhere, Category= "子弹属性")
	UParticleSystem* Tracer;

	//特效组件
	UPROPERTY()
	UParticleSystemComponent* TracerComponent;
};
