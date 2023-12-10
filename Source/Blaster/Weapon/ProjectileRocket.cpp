// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	if(TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		TrailSystem,  //粒子系统
		GetRootComponent(), //附加对象，这里选择当前这个actor的root
		FName(), //附加对象的FName 这里没有，就留空
		GetActorLocation(), //位置
		GetActorRotation(), //旋转
		EAttachLocation::KeepWorldPosition,  //
		false  //是否自动销毁，这里选false，我们想手动控制其销毁
			);
	}
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
	
	APawn* FiringPawn =	GetInstigator();
	if(FiringPawn && HasAuthority())
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
	
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &ThisClass::DestroyTimerFinished, DestroyTime);

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
	if(RocketMesh)
	{
		//设置模型可见为不可见
		RocketMesh->SetVisibility(false);
	}
	if(CollisionBox)
	{
		//设置碰撞为关闭碰撞
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if(TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		//设置拖尾的特效发生器停止
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();//GetSystemInstance()不可用
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

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}