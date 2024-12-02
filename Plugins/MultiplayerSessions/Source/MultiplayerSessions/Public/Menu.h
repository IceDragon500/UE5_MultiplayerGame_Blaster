// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Menu.generated.h"



class UButton;
class UMultiplayerSessionsSubsystem;
/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	


public:

	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));

protected:

	//重写UUserWidget中的Initialize()方法
	virtual bool Initialize() override;

	virtual void NativeDestruct() override;//教程中提示，UE5.1以上，使用NativeDestruct()；视频中是UE5.0，使用的是virtual void OnLeveRemovedFromWorld() override;

	//
	//Callbacks for the custom delegates on the MultiplayerSessionSubsystem
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful);

	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
private:

	//Host按钮按下的功能
	UFUNCTION()
	void HostButtonClicked();

	//Join按钮按下的功能
	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();


	//声明button变量，对应UMG中的两个按钮，名称需要一致
	UPROPERTY(meta = (BinWidget))
	UButton* HostButton;

	//声明button变量，对应UMG中的两个按钮，名称需要一致
	UPROPERTY(meta = (BinWidget))
	UButton* JoinButtom;

	//The subsystem designed to handle all online session functionality
	//处理所有在线会话功能的子系统
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	int32 NumPublicConnections = 4;
	
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	FString MatchType = TEXT("FreeForAll");
	
	FString PathToLobby = TEXT("");

};
