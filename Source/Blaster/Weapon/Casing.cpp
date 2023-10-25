// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"

#include "Kismet/GameplayStatics.h"

ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);

	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	//开启物理效果
	CasingMesh->SetSimulatePhysics(true);
	//开启重力
	CasingMesh->SetEnableGravity(true);
	//模拟生成命中事件 
	CasingMesh->SetNotifyRigidBodyCollision(true);

	ShellEjectionImpulse = 10.f;
}


void ACasing::BeginPlay()
{
	Super::BeginPlay();

	CasingMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);

	//添加一个冲量
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if(ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}

	Destroy();
}

