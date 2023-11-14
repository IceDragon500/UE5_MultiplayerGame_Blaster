// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn =	GetInstigator();
	if(FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if(FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,  //传入世界上下文
				Damage,  //伤害值
				10.f,  //最小伤害值
				GetActorLocation(),  //命中圆点中心的位置
				200.f,   //最小半径 最小半径内的伤害就是满额伤害
				500.f,   //最大半径 最大半径到最小半径中间是线性递减的伤害
				1.f,     //衰减方式,
				UDamageType::StaticClass(),  //伤害类型
				TArray<AActor*>(),  //被忽略的actor，创建一个数组并且添加任何你想忽略的参与者，因为我们这里是都可以受到伤害，所以传入了一个空数组
				this,  //创造这个伤害的actor，这里是火箭弹，所以是这个子弹类创建了这个伤害，所以是this
				FiringController   //负责造成伤害的控制器（例如，投掷手榴弹的玩家）
				);
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
	
}
