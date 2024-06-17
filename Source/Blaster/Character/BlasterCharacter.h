// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Blaster/BlaseComponents/CombatComponent.h"
#include "Blaster/BlaseComponents/BuffComponent.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "GameFramework/Character.h"
#include "Blaster/Interface/InteractWithCrosshairsInterface.h"
//#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Components/TimelineComponent.h"
#include "BlasterCharacter.generated.h"

/*
 * 主要的角色类
 */

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AWeapon;
class UWidgetComponent;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//AActor::PostInitializeComponents: Called after the actor’s components have been initialized, only during gameplay and some editor previews
	//在角色的组件初始化后调用，仅在游戏和某些编辑器预览时调用
	//用这个来初始化CombatComponent相关的变量
	virtual void PostInitializeComponents() override;
	//播放开火动画
	void PlayFireMontage(bool bAiming);
	//播放换弹动画
	void PlayReloadMontage();
	//播放死亡动画
	void PlayElimMontage();
	//播放投掷手雷的动画
	void PlayThrowGrenadeMontage();
	//用来播放受击动画
	void PlayHitReactMontage();

	
	virtual void OnRep_ReplicatedMovement() override;
	
	void Elim();//被淘汰之后的逻辑
	
	UFUNCTION(NetMulticast, Reliable)//被淘汰之后的逻辑
	void MulticastElim();
	
	virtual void Destroyed() override;
	
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;
	
	virtual void Jump() override;//跳跃
	void SetOverlappingWeapon(AWeapon* Weapon);
	
	//返回是否已经装备武器
	bool IsWeaponEquipped();

	//返回是否在瞄准
	bool IsAiming();

	FORCEINLINE float GetAO_Yaw() const { return  AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMotage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachGrenade() const { return AttachedGrenade; }

	FORCEINLINE UBuffComponent* GetBuff() const { return BuffComponent; }
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	//更新显示血量的HUD
	void UpdateHUDHealth();
	//更新显示护盾的HUD
	void UpdateHUDShield();

	void UpdateHUDAmmo();

	void SpawnDefaultWeapon();
	
protected:
	virtual void BeginPlay() override;

	void RotateInPlace(float DeltaTime);
/*
 * 增强输入
 */
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|增强输入")
	UInputMappingContext* InputContext;//增强输入
	
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|增强输入")
	UInputAction* MovementAction;//增强输入-移动
	void Move(const FInputActionValue& Value);
	
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|增强输入")
	UInputAction* LookAction;//增强输入-鼠标
	void Look(const FInputActionValue& Value);
	
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|增强输入")
	UInputAction* JumpAction;//增强输入-跳
	
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|增强输入")
	UInputAction* AttackAction;//增强输入-鼠标左键攻击 
	void FireButtonPressed(const FInputActionValue& Value);
	void FireButtonReleased(const FInputActionValue& Value);
	
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|增强输入")
	UInputAction* ReloadKeyAction;//增强输入-R键功能
	void ReloadButtonPressed(const FInputActionValue& Value);
	
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|增强输入")
	UInputAction* PickupKeyAction;//增强输入-E键功能
	void PickupKeyPressed(const FInputActionValue& Value);
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();
	
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|增强输入")
	UInputAction* ThrowGrenadeKeyAction;//增强输入-T键功能
	void ThrowGrenadePressed(const FInputActionValue& Value);
	
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|增强输入")
	UInputAction* Crouching;//增强输入-蹲
	void CrouchKeyPressed(const FInputActionValue& Value);
	
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|增强输入")
	UInputAction* Aiming;//增强输入-瞄准
	void AimButtonPressed(const FInputActionValue& Value);
	void AimButtonReleased(const FInputActionValue& Value);
	
	float CalculateSpeed();//获取速度
	void CalculateAO_Pitch();//获取旋转

	
	void AimOffset(float DeltaTime);//用来计算瞄准偏移
	void SimProxiesTurn();//模拟转向

	//受到伤害
	//收到伤害的方法
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatroController, AActor* DamageCasuer);

	//初始化分数相关的操作
	//Poll for any relelvant classes and initialize our hud
	//轮询任何相关类并初始化hud
	void PollInit();

private:
	//创建一个OnRep的方法，给需要复制的变量指定这个方法
	//是一个接收到服务器replicate后需要执行的函数，replicate过程是单向的，所以On_RepXXX只能再客户端执行
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	

	//用来处理原地转身
	void TurnInPlace(float DeltaTime);

	//用来处理靠墙时，隐藏角色模型
	void HideCameraIfCharacterClose();
	
	//摄像机靠墙阈值
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	
	UPROPERTY(VisibleAnywhere, Category = "BlasterPlayer");
	ABlasterPlayerController* BlasterPlayerController;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;
	
	UPROPERTY(VisibleAnywhere, Category = "BlasterPlayer|摄像机")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "BlasterPlayer|摄像机")
	UCameraComponent* FollowCamera;

	//创建拾取提示组件
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	//使用ReplicatedUsing = OnRep_OverlappingWeapon，表示我们在这个变量上调用OnRep_OverlappingWeapon这个方法
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	//创建战斗组件CombatComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BlasterPlayer", meta=(AllowPrivateAccess = "true"))
	UCombatComponent* Combat;

	//创建Buff组件
	UPROPERTY(VisibleAnywhere, Category = "BlasterPlayer", meta=(AllowPrivateAccess = "true"))
	UBuffComponent* BuffComponent;
	
	float AO_Yaw;//瞄准偏移Yaw
	float InterpAO_Yaw;//用来计算转身的Yaw
	float AO_Pitch;//瞄准偏移Pitch
	FRotator  StartingAimRotation;//用来计算左手相较于右手的位置变化

	ETurningInPlace TurningInPlace;//转身的状态

	//开火的动画
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|战斗")
	UAnimMontage* FireWeaponMontage;

	//受击动画
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|战斗")
	UAnimMontage* HitReactMontage;
	
	//死亡动画
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|战斗")
	UAnimMontage* ElimMontage;

	//换弹动画
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|战斗")
	UAnimMontage* ReloadMontage;

	//投掷手雷动画
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|战斗")
	UAnimMontage* ThrowGrenadeMontage;

	//是否旋转根骨骼
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;//上次运动复制后的时间

	/**
	 * 角色健康值
	 */
	UPROPERTY(ReplicatedUsing= OnRep_Health, EditAnywhere, Category= "BlasterPlayer|角色状态")
	float Health = 100.f;//当前血量,将其绑定至OnRep_Health方法，进行网络复制
	
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|角色状态")
	float MaxHealth = 100.f;//最大生命值
	
	UFUNCTION()
	void OnRep_Health(float LastHealth);//我们需要在血量值变化的时候，进行复制

	/**
	 * 角色护盾值
	 */
	UPROPERTY(ReplicatedUsing= OnRep_Shield, EditAnywhere, Category= "BlasterPlayer|角色状态")
	float Shield = 12.f;//当前护盾值
	UPROPERTY(EditAnywhere, Category= "BlasterPlayer|角色状态")
	float MaxShield = 100.f;//最大护盾值
	UFUNCTION()
	void OnRep_Shield(float LastShield);

	//是否被淘汰
	bool bElimmed = false;

	//设置复活的定时器
	FTimerHandle ElimTimer;
	//复活的时间间隔
	UPROPERTY(EditAnywhere, Category="BlasterPlayer|角色状态")
	float ElimDelay = 3.f;

	//定时器到时间之后调用的方法
	void ElimTimerFinished();

	/*
	 *使用时间轴控制角色溶解特效
	 */
	//时间轴组件
	UPROPERTY(VisibleAnywhere, Category="BlasterPlayer|角色淘汰")
	UTimelineComponent* DissolveTimeline;
	//时间轴中使用的曲线类型
	FOnTimelineFloat DissolveTrack;
	//定义曲线
	UPROPERTY(EditAnywhere, Category="BlasterPlayer|角色淘汰")
	UCurveFloat* DissolveCurve;
	//定义动态材质
	//我们可以在运行时更改的动态实例Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category="BlasterPlayer|角色淘汰")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	//蓝图上设置的材料实例，与动态材料一起使用material instance set on the Blueprint, used with the dynamic material
	UPROPERTY(EditAnywhere, Category="BlasterPlayer|角色淘汰")
	UMaterialInstance* DissolveMaterialInstance;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();
	
	/*
	 *Elim Bot
	 */

	//角色被淘汰时的特效
	UPROPERTY(EditAnywhere, Category="BlasterPlayer|角色淘汰")
	UParticleSystem* ElimBotEffect;

	//特效组件
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;
	
	UPROPERTY(EditAnywhere, Category="BlasterPlayer|角色淘汰")
	USoundCue* ElimBotSound;

/**
 * Grenade手雷
 */

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/***
	 * Default Weapon默认武器
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;
	
};
