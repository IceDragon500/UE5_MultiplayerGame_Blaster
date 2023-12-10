// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;// 对应蓝图中的抛射物-旋转跟踪速度：发射物每帧更新他的旋转，用来匹配他前进的方向
	ProjectileMovementComponent->SetIsReplicated(true);
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if(OwnerCharacter)
	{
		AController* OwnerControll = OwnerCharacter->Controller;
		if(OwnerControll)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerControll, this, UDamageType::StaticClass());
		}
	}
	

	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
