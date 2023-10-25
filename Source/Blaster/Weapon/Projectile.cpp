// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

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
	CollisionBox->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;// 对应蓝图中的抛射物-旋转跟踪速度：发射物每帧更新他的旋转，用来匹配他前进的方向

	//TracerComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));

}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	if(Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(Tracer, CollisionBox, FName(), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition);

		//用这个 特效不会动
		//TracerComponent = UGameplayStatics::SpawnEmitterAtLocation(this, Tracer, GetActorLocation(),GetActorRotation());
	}
	
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

