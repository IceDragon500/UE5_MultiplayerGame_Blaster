// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()
public:
	AHealthPickup();
	virtual void Destroyed() override;
	
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
private:

	UPROPERTY(EditAnywhere)
	float HealAmount = 100.f;//可以治疗的点数
	
	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f;//回复的时间 ，这里是设计了一个缓慢回复的效果

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;//添加一个奶瓜组件用来实现血包的模型

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;//添加一个奶瓜特效，用来实现拾取血包后的治疗效果
	
};
