// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;// 对应蓝图中的抛射物-旋转跟踪速度：发射物每帧更新他的旋转，用来匹配他前进的方向
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR	
void AProjectileBullet::PostEditChangeProperty(struct FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName =Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if(PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if(ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif


void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{

	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if(OwnerCharacter)
	{
		ABlasterPlayerController* OwnerControll = Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
		if(OwnerControll)
		{
			if(OwnerCharacter->HasAuthority() && !bUserServerSideRewind)
			{
				UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerControll, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}

			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			if(bUserServerSideRewind && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() && HitCharacter->IsLocallyControlled())
			{
				OwnerCharacter->GetLagCompensation()->ProjectileServerScoreRequest(
					HitCharacter,
					TraceStart,
					InitialVelocity,
					OwnerControll->GetServerTime() - OwnerControll->SingleTripTime
					);
			}
		}
	}
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();
	/*
	//这是输入参数结构体，用于设置预测抛射物路径的条件和参数
	FPredictProjectilePathParams PathParams;
	
	// bTraceComplex 如果设置为true，则使用复杂的碰撞检测。复杂碰撞检测会考虑模型的实际形状，而不是简单的碰撞体（如球体、盒体等），但这会增加性能开销。
	// LaunchVelocity (FVector)：抛射物的初始速度向量。
	// ObjectTypes 指定哪些类型的物体应该被包含在碰撞检测中。你可以选择特定的碰撞通道或对象类型。
	// ProjectileRadius (float)：抛射物的半径，用于碰撞检测。
	// SimFrequency (int32)：模拟频率，决定每秒计算多少次路径点。
	// StartLocation (FVector)：抛射物的起始位置。
	// TraceChannel 指定用于碰撞检测的碰撞通道。不同的通道可以用来区分不同类型的碰撞。
	// ActorsToIgnore 一个数组，包含了在碰撞检测时应该忽略的角色
	// bTraceWithChannel 如果设置为true，则使用TraceChannel来进行碰撞检测
	// bTraceWithCollision  如果设置为true，则进行碰撞检测。这与bTracePath类似，但bTraceWithCollision可能更通用。
	// DrawDebugTime
	// DrawDebugType
	// MaxSimTime 最大模拟的时间
	// OverrideGravityZ


	PathParams.bTraceWithChannel = true;
	PathParams.bTraceWithCollision = true;
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::None;
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	PathParams.MaxSimTime = 4.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.SimFrequency = 30.f;
	PathParams.StartLocation = GetActorLocation();
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	PathParams.ActorsToIgnore.Add(this);


	//这是输出结果结构体，包含了路径预测的结果信息：
	FPredictProjectilePathResult PathResult;

	// PathData (TArray<FPredictProjectilePathPointData>)：一个数组，包含了沿路径的每个点的详细信息，包括位置、速度、时间等。
	// HitResult (FHitResult)：如果路径与物体发生碰撞，这里包含了碰撞的详细信息，如碰撞点、法线、碰撞的物体等。
	// LastTraceDestination (FVector)：如果没有发生碰撞，这是路径的终点位置。
	// FPredictProjectilePathPointData 则包含了每个路径点的具体信息：
	//
	// Location (FVector)：路径点的空间位置。
	// Velocity (FVector)：路径点时的速度。
	// Time (float)：到达这个点所需的时间。
	// Distance (float)：从起点到这个点之间的距离。

	//通过这种方式，你可以预测抛射物的路径，这在游戏中用于瞄准系统、射击游戏的弹道预览等场景中非常有用。记得在你的代码中处理可能的错误和边界情况
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
	*/
}
