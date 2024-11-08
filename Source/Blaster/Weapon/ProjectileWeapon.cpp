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
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	UWorld* World = GetWorld();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if(MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh()); //获得枪口的变换，包含枪口的坐标和旋转
		FVector ToTarget = HitTarget - SocketTransform.GetLocation(); // 得到从枪口到命中点的向量
		FRotator TargetRotation = ToTarget.Rotation();//获得命中方向的旋转

		//设置一个FActorSpawnParameters结构体，这里对应蓝图中的SpawnActor节点中的选项
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;
		if(bUseServerSideRewind)
		{
			if(InstigatorPawn->HasAuthority())//server
			{
				if(InstigatorPawn->IsLocallyControlled()) //server, host - use replicated projectile服务器、主机 - 使用复制射弹
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUserServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;
				}
				else // server, not locally controlled - spawn non-replicated projectile, no SSR服务器，非本地控制 - 产生非复制弹丸，无 SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUserServerSideRewind = false;
				}
			}
			else // client, using SSR
			{
				if(InstigatorPawn->IsLocallyControlled())// client, locally controlled - spawn non-replicated projectile, use SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUserServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					SpawnedProjectile->Damage = Damage;
				}
				else // client, not locally conftrolled - spawn non-replicated projectile, no SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUserServerSideRewind = false;
				}
			}
		}
		else // weapon not using SSR
		{
			if(InstigatorPawn->HasAuthority())
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUserServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;
			}
		}
	}
}
