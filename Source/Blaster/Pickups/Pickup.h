// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Pickup.generated.h"

UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	
public:
protected:
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	virtual void Destroyed() override;

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;//给拾取物一个自动的旋转
	
private:
	
	UPROPERTY(EditAnywhere)
	USphereComponent* OverlapSphere;
	
	UPROPERTY(EditAnywhere)
	USoundCue* PickupSound;
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;
	

};
