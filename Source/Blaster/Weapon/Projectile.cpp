// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

#include "NiagaraFunctionLibrary.h"
#include "Blaster/Blaster.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; //子弹可复制

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

	

	//TracerComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));

}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	if(Tracer)
	{
		//绑定特效到碰撞盒子上，这样子弹在飞行的时候就能看到特效
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
			);

		//用这个 特效不会动
		//TracerComponent = UGameplayStatics::SpawnEmitterAtLocation(this, Tracer, GetActorLocation(),GetActorRotation());
	}

	if(HasAuthority())
	{
		//将重写的OnHit事件与CollisionBox进行绑定
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}
	//CollisionBox->IgnoreActorWhenMoving()
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}

void AProjectile::SpawnTrailSystem()
{
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
}

void AProjectile::ExplodeDamage()
{
	
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
				DamageInnerRadius,   //最小半径 最小半径内的伤害就是满额伤害
				DamageOuterRadius,   //最大半径 最大半径到最小半径中间是线性递减的伤害
				1.f,     //衰减方式,
				UDamageType::StaticClass(),  //伤害类型
				TArray<AActor*>(),  //被忽略的actor，创建一个数组并且添加任何你想忽略的参与者，因为我们这里是都可以受到伤害，所以传入了一个空数组
				this,  //创造这个伤害的actor，这里是火箭弹，所以是这个子弹类创建了这个伤害，所以是this
				FiringController   //负责造成伤害的控制器（例如，投掷手榴弹的玩家）
				);
		}
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&ThisClass::DestroyTimerFinished,
		DestroyTime
		);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}
