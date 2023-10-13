// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AWeapon;
class UWidgetComponent;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


public:
	//跳跃
	virtual void Jump() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetOverlappingWeapon(AWeapon* Weapon);

protected:
	//增强输入
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputMappingContext* InputContext;

	//增强输入-移动
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputAction* MovementAction;
	void Move(const FInputActionValue& Value);

	
	//增强输入-鼠标
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputAction* LookAction;
	void Look(const FInputActionValue& Value);

	//增强输入-跳
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputAction* JumpAction;

	//增强输入-鼠标左键攻击
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputAction* AttackAction;
	void AttackKeyPressed(const FInputActionValue& Value);

	//增强输入-E键功能
	UPROPERTY(EditAnywhere, Category= "增强输入")
	UInputAction* EKeyAction;
	void EKeyPressed(const FInputActionValue& Value);

private:
	//创建一个OnRep的方法，给需要复制的变量指定这个方法
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	//使用ReplicatedUsing = OnRep_OverlappingWeapon，表示我们在这个变量上调用OnRep_OverlappingWeapon这个方法
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;
	
};
