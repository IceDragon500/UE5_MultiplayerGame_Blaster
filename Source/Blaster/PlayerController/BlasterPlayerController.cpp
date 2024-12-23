// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "EnhancedInputComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Components/Image.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/BlasterTypes/Announcement.h"


void ABlasterPlayerController::BroadcastElie(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void ABlasterPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if(Attacker && Victim && Self)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if(BlasterHUD)
		{
			if(Attacker == Self && Victim != Self)//自己击杀了他人
			{
				BlasterHUD->AddElimAnnouncement(TEXT("你"), Victim->GetPlayerName());
				return;
			}
			if(Victim == Self && Attacker != Self)//自己被击杀了
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), TEXT("你"));
				return;
			}
			if(Attacker == Victim && Attacker == Self)//自己自杀
			{
				BlasterHUD->AddElimAnnouncement(TEXT("你"), TEXT("自己"));
				return;
			}
			if(Attacker == Victim && Attacker != Self)//别个自杀
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), TEXT("自己"));
				return;
			}
			BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());//第三者看A杀了B
		}
	}
}

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	ServerCheckMatchState();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerController, MatchState);
	DOREPLIFETIME(ABlasterPlayerController, bShowTeamScores);
}

void ABlasterPlayerController::HideTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScoreText &&
		BlasterHUD->CharacterOverlay->BlueTeamScoreText;
	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->RedTeamScoreText->SetText(FText());
		BlasterHUD->CharacterOverlay->BlueTeamScoreText->SetText(FText());
	}
}

void ABlasterPlayerController::InitTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScoreText &&
		BlasterHUD->CharacterOverlay->BlueTeamScoreText;
	if(bHUDValid)
	{
		FString Zero("0");
		BlasterHUD->CharacterOverlay->RedTeamScoreText->SetText(FText::FromString(Zero));
		BlasterHUD->CharacterOverlay->BlueTeamScoreText->SetText(FText::FromString(Zero));
	}
}

void ABlasterPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScoreText;
	if(bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		BlasterHUD->CharacterOverlay->RedTeamScoreText->SetText(FText::FromString(ScoreText));
	}
}

void ABlasterPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->BlueTeamScoreText;
	if(bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		BlasterHUD->CharacterOverlay->BlueTeamScoreText->SetText(FText::FromString(ScoreText));
	}
}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
	CheckPing(DeltaSeconds);
}

void ABlasterPlayerController::CheckPing(float DeltaTime)
{
	//如果累加时间超过20秒CheckPingFrequency，则检测一下ping，如果ping达标 就播放wifi图标的动画，需要先将PingAnimationRunningTime置为0，方便进行累加
	//
	HighPingRunningTime += DeltaTime;
	PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
	if(HighPingRunningTime > CheckPingFrequency)
	{
		
		if(PlayerState)
		{
			//if(PlayerState->GetPing() * 4 > HighPingThreshold) //ping是被压缩的，被除以了4，所以这里要乘4 。另外这里使用了GetPing()会提示警告，因为新版本不推荐使用这个函数了
			//我们可以用GetPingInMilliseconds()  或者 GetCompressedPing()
			//这里我使用GetPingInMilliseconds()
			if(PlayerState->GetPingInMilliseconds() > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0;
	}
	if(PlayerState) ShowPingDate(PlayerState->GetPingInMilliseconds());
	
	bool bHighPingAnimationPlaying =
		BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HighPingAnimation && BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation);
	//如果正在播放动画 IsAnimationPlaying = true 则开始累加PingAnimationRunningTime，如果超过设定的5秒HighPingDration
	//也就是说 动画播放了5秒，执行动画关闭StopHighPingWaring
	if(bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if(PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHightPing)
{
	HighPingDelegate.Broadcast(bHightPing);
}

void ABlasterPlayerController::ShowPingDate(float Ping)
{
	//先判断BlasterHUD是否为空，如果为空则创建，如果不为空，则不改变
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	//因为这里需要直接使用到CharacterOverlay中的具体指针变量，所以都需要检查一下是否为空
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->PingDate;
	if(bHUDValid)
	{

		FString PingText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Ping));
		//设置文字，这里是FText类型，需要FString转换为FText
		BlasterHUD->CharacterOverlay->PingDate->SetText(FText::FromString(PingText));
	}
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlasterPlayerController::HighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HighPingImage &&
				BlasterHUD->CharacterOverlay->HighPingAnimation;
	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation, 0.f, 5.f);
	}
}

void ABlasterPlayerController::StopHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HighPingImage &&
				BlasterHUD->CharacterOverlay->HighPingAnimation;
	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if(BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);
		}
		
	}
}

void ABlasterPlayerController::OnRep_ShowTeamScores()
{
	if(bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

FString ABlasterPlayerController::GetInfoText(const TArray<class ABlasterPlayerState*>& PlayerStates)
{
	ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if(BlasterPlayerState == nullptr) return FString();
	FString InfoText;
	
	if(PlayerStates.Num() == 0 )
	{
		InfoText = Announcement::ThereIsNoWinner;
	}
	else if(PlayerStates.Num() == 1 && PlayerStates[0] == BlasterPlayerState)
	{
		InfoText = Announcement::YouAreTheWinner;
	}
	else if(PlayerStates.Num() == 1)
	{
		InfoText = FString::Printf(TEXT("赢家是: \n%s"), *PlayerStates[0]->GetPlayerName());
	}
	else if(PlayerStates.Num() > 1)
	{
		InfoText = Announcement::PlayersTiedForTheWin;
		InfoText.Append(FString(TEXT("\n")));
		for(auto TiedPlayer : PlayerStates)
		{
			InfoText.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}
	
	return InfoText;
}

FString ABlasterPlayerController::GetTeamsInfoText(class ABlasterGameState* BlasterGameState)
{
	if(BlasterGameState == nullptr) return FString();
	FString InfoText;

	const int32 RedTeamScore = BlasterGameState->GetRedTeamScore();
	const int32 BlueTeamScore = BlasterGameState->GetBlueTeamScore();

	if(RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoText = Announcement::ThereIsNoWinner;
	}
	else if(RedTeamScore == BlueTeamScore)
	{
		InfoText =FString::Printf(TEXT("%s\n"),*Announcement::TeamsTiedForTheWin);
		InfoText.Append(Announcement::RedTeam);
		InfoText.Append(TEXT("\n"));
		InfoText.Append(Announcement::BlueTeam);
	}
	else if(RedTeamScore > BlueTeamScore)
	{
		InfoText = Announcement::RedTeamWin;
		InfoText.Append(TEXT("\n"));
		InfoText.Append(FString::Printf(TEXT("%s: %d\n"),*Announcement::RedTeam, RedTeamScore));
		InfoText.Append(FString::Printf(TEXT("%s: %d\n"),*Announcement::BlueTeam, BlueTeamScore));
	}
	else if(BlueTeamScore > RedTeamScore)
	{
		InfoText = Announcement::BlueTeamWin;
		InfoText.Append(TEXT("\n"));
		InfoText.Append(FString::Printf(TEXT("%s: %d\n"),*Announcement::BlueTeam, BlueTeamScore));
		InfoText.Append(FString::Printf(TEXT("%s: %d\n"),*Announcement::RedTeam, RedTeamScore));
	}

	return InfoText;
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime,MatchTime, CooldownTime, LevelStartingTime);
	}
}

void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float StatingTime, float Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StatingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	
	if(BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncement();
	}	
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if(BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
		//SetHUDShield(BlasterCharacter->GetShield(), BlasterCharacter->GetMaxShield());
		//BlasterCharacter->UpdateHUDAmmo();
	}
}


void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if(InputComponent == nullptr) return;
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	if(EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(ReturnMenu, ETriggerEvent::Triggered, this, &ABlasterPlayerController::ReturnKeyPressed);
	}
	
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	//先判断BlasterHUD是否为空，如果为空则创建，如果不为空，则不改变
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	/*if(BlasterHUD == nullptr)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}*/

	//因为这里需要直接使用到CharacterOverlay中的具体指针变量，所以都需要检查一下是否为空
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HealthBar && BlasterHUD->CharacterOverlay->HealthText;
	if(bHUDValid)
	{
		//取得当前血量百分比
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		//设置血条上的文字，为了避免出现小数点，用FMath::CeilToInt进行四舍五入
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		//设置文字，这里是FText类型，需要FString转换为FText
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	//这里逻辑过程与设置血量是一致的
	if(BlasterHUD == nullptr)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}
	
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->ShieldBar && BlasterHUD->CharacterOverlay->ShieldText;
	if(bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->ScoreAmount;
	if(bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->DefeatsAmount;
	if(bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->WeaponAmmoAmount;
	if(bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->CarriedAmmoAmount;
	if(bHUDValid)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->MatchCountdownText;
	if(bHUDValid)
	{
		if(CountdownTime < 0.f)
		{
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->Announcement && BlasterHUD->Announcement->WarmupTime;
	if(bHUDValid)
	{
		if(CountdownTime < 0.f)
		{
			BlasterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDGrenades(int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->GrenadesText;
	if(bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if(MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if(MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if(MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	
	/*
	*在这句话中，FMath::CeilToInt()的作用是把MatchTime - GetWorld()->GetTimeSeconds()的结果进行向上取整。也就是说如果结果的小数部分大于或等于0.5，则会返回下一个整数。
	*假设MatchTime表示总的游戏时长，并且GetWorld()->GetTimeSeconds()返回的是当前游戏中的实时时间。那么SecondsLeft就会存储游戏中还剩多少秒结束。
	*具体的值取决于MatchTime和GetWorld()->GetTimeSeconds()的具体数值。比如如果MatchTime是90秒，而当前时间为60秒，那么SecondsLeft就是30秒。如果MatchTime是180秒，当前时间为60秒，那么SecondsLeft就是120秒。
	 */
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	/*
	if(HasAuthority())
	{
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) :BlasterGameMode;
		if(BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	*/
	
	if(CountdownInt != SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown )
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if(MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::PollInit()
{
	if(CharacterOverlay == nullptr)
	{
		if(BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if(CharacterOverlay)
			{
				if(bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if(bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if(bInitializeScore) SetHUDScore(HUDScore);
				if(bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if(bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if(bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
				if(BlasterCharacter && BlasterCharacter->GetCombat())
				{
					if(bInitializeGrenades) SetHUDGrenades(BlasterCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();//获取游戏开始的时间，从0秒开始
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABlasterPlayerController::GetServerTime()
{
	if(HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	else
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
	
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;
	
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if(HasAuthority()) bShowTeamScores = bTeamsMatch;
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		//进入游戏开始阶段后，把角色主界面添加到屏幕，然后将等待界面设置为隐藏
		if(BlasterHUD->CharacterOverlay == nullptr) BlasterHUD->AddCharacterOverlay();
		if(BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if(!HasAuthority()) return;
		if(bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		//游戏结束后进入冷却阶段，这个时候移除界面内容  然后把等待界面显示出来
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = BlasterHUD->Announcement &&
			BlasterHUD->Announcement->AnnouncementText &&
			BlasterHUD->Announcement->InfoText;
		
		if(bHUDValid)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			
			if(BlasterGameState && BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> TopPlayer = BlasterGameState->TopScoringPlayers;
				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(BlasterGameState) : GetInfoText(TopPlayer);
				
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
			
			
		}
	}
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if(BlasterCharacter)
	{
		BlasterCharacter->bDisableGameplay = true;
		//教程下面还调用了CombatComponent里面的FireButtonPressed的false逻辑，我觉得没有必要
		//BlasterCharacter->GetCombat->FireButtonPressed(false);
		//然后他还禁用了角色的瞄准偏移  还是觉得没有必要
	}
}

void ABlasterPlayerController::ReturnKeyPressed(const FInputActionValue& Value)
{
	if(ReturnToMainMenuWidget == nullptr) return;
	if(ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if(ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if(bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("按下了按钮"));
	//show the return to main menu widget
	//显示返回到主菜单小部件

	
	
}