// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Casing.generated.h"
/*
 * 弹壳类
 */
UCLASS()
class BLASTER_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasing();
	//弹壳不需要Tick函数
	//virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp,	AActor* OtherActor,	UPrimitiveComponent* OtherComp,	FVector NormalImpulse, const FHitResult& Hit);
private:
	UPROPERTY(VisibleAnywhere, Category= "弹壳属性")
	UStaticMeshComponent* CasingMesh;

	//弹壳抛出的冲量
	UPROPERTY(EditAnywhere, Category= "弹壳属性")
	float ShellEjectionImpulse;

	//弹壳落地的声音
	UPROPERTY(EditAnywhere, Category= "弹壳属性")
	USoundCue* ShellSound;

};
