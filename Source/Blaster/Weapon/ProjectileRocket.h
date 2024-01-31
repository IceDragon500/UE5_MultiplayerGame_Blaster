// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "RocketMovementComponent.h"
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



	//火箭飞行的声音
	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;
	
	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	//创建一个火箭飞行声音的衰减
	UPROPERTY(EditAnywhere)
	USoundAttenuation* LooppingSoundAttenuation;

	UPROPERTY(VisibleAnywhere)
	URocketMovementComponent* RocketMovementComponent;
	
private:

	
};
