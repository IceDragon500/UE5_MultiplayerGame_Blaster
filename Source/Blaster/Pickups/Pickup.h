// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "NiagaraCommon.h"
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

	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* PickupEffectComponent;//添加一个奶瓜组件用来实现血包的模型

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* PickupEffect;//添加一个奶瓜特效，用来实现拾取血包后的治疗效果

	FTimerHandle BindOverlapTimer; //用来控制碰撞生效的间隔时间
	float BindOverlapTime = 0.25f;
	void BindOverlapTimerFinish();
	

};
