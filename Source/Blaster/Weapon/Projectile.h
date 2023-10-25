﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	

	AProjectile();
	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;


private:

	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, Category= "子弹属性")
	UProjectileMovementComponent* ProjectileMovementComponent;

	//子弹特效
	UPROPERTY(EditAnywhere, Category= "子弹属性")
	UParticleSystem* Tracer;

	//特效组件
	UParticleSystemComponent* TracerComponent;
	

};