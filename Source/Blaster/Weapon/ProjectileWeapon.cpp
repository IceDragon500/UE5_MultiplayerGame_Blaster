// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileWeapon::AProjectileWeapon()
{
}

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if(!HasAuthority()) return; //除非在我们服务器存在的武器，否则这些都不会实现

		
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if(MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh()); //获得枪口的变换，包含枪口的坐标和旋转
		FVector ToTarget = HitTarget - SocketTransform.GetLocation(); // 得到从枪口到命中点的向量
		FRotator TargetRotation = ToTarget.Rotation();//获得命中方向的旋转
		
		if(ProjectileClass && InstigatorPawn)
		{
			//设置一个FActorSpawnParameters结构体，这里对应蓝图中的SpawnActor节点中的选项
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;
			
			UWorld* World = GetWorld();
			if(World)
			{
				World->SpawnActor<AProjectile>(
					ProjectileClass,//传入需要生成的类
					SocketTransform.GetLocation(), //传入生成类的坐标
					TargetRotation,//传入生成类的旋转
					SpawnParams//传入FActorSpawnParameters 的结构体信息
					);
			}
		}
	}
}
