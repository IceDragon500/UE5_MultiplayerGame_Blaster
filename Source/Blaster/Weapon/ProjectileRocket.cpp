// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	//这里不是很清楚为什么需要重新在这里绑定OnHit方法，而且是 非HasAuthority
	//AI的解释：通过这种方式，无论是在拥有者还是非拥有者的客户端上，都能确保OnHit事件被正确地绑定到CollisionBox上，从而在发生碰撞时触发相应的处理逻辑。
	if(!HasAuthority())
	{
		//将重写的OnHit事件与CollisionBox进行绑定
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}

	SpawnTrailSystem();
	
	//生成一个附件的声音，可以同时设置声音衰减
	if(ProjectileLoop && LooppingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LooppingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
			);
	}
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	//避免一开始炸到自己
	if(OtherActor == GetOwner())
	{
		return;
	}
	ExplodeDamage();//处理爆炸伤害
	
	StartDestroyTimer();//开启自毁的倒计时

	//因为在上面我们为了让拖尾的烟雾特效持续显示，做了actor延迟3秒的销毁
	//所以我们需要先播放粒子特效和音效，然后隐藏掉模型和关闭碰撞
	//最后等待上面3秒延迟之后的销毁
	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	//当火箭触碰到碰撞物
	if(ProjectileMesh)
	{
		//设置模型可见为不可见
		ProjectileMesh->SetVisibility(false);
	}
	if(CollisionBox)
	{
		//设置碰撞为关闭碰撞
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if(TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		//设置拖尾的特效发生器停止
		//TrailSystemComponent->GetSystemInstanceController()->Deactivate();//GetSystemInstance()不可用

		//UE5.6更新
		TrailSystemComponent->Deactivate();
	}
	if(ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		//设置火箭飞行的声音停止播放
		ProjectileLoopComponent->Stop();
	}
}

void AProjectileRocket::Destroyed()
{
	//Super::Destroyed();
}