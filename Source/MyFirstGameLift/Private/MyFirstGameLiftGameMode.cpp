#include "MyFirstGameLiftGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "MyFirstGameLiftHUD.h"
#include "MyFirstGameLiftPlayerState.h"
#include "MyFirstGameLiftGameState.h"
#include "TextReaderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Json.h"
#include "JsonUtilities.h"

AMyFirstGameLiftGameMode::AMyFirstGameLiftGameMode()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(L"Blueprint'/Game/BluePrint/MotionControllerPawn.MotionControllerPawn_C'");
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
		HUDClass = AMyFirstGameLiftHUD::StaticClass();
		PlayerStateClass = AMyFirstGameLiftPlayerState::StaticClass();
		GameStateClass = AMyFirstGameLiftGameState::StaticClass();
	}

	UTextReaderComponent* TextReader = CreateDefaultSubobject<UTextReaderComponent>(TEXT("TextReaderComp"));
	ApiUrl = TextReader->ReadFile("Urls/ApiUrl.txt");

	HttpModule = &FHttpModule::Get();

	RemainingGameTime = 240;
	GameSessionActivated = false;

	WaitingForPlayersToJoin = false;
	TimeSpentWaitingForPlayersToJoin = 0;
}

void AMyFirstGameLiftGameMode::BeginPlay() {
	Super::BeginPlay();

#if WITH_GAMELIFT
	auto InitSDKOutcome = Aws::GameLift::Server::InitSDK();

	if (InitSDKOutcome.IsSuccess()) {
		auto OnStartGameSession = [](Aws::GameLift::Server::Model::GameSession GameSessionObj, void* Params)
		{
			FStartGameSessionState* State = (FStartGameSessionState*)Params;

			State->Status = Aws::GameLift::Server::ActivateGameSession().IsSuccess();

			FString MatchmakerData = GameSessionObj.GetMatchmakerData();

			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(MatchmakerData);

			if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
				State->MatchmakingConfigurationArn = JsonObject->GetStringField("matchmakingConfigurationArn");

				TArray<TSharedPtr<FJsonValue>> Teams = JsonObject->GetArrayField("teams");
				for (TSharedPtr<FJsonValue> Team : Teams) {
					TSharedPtr<FJsonObject> TeamObj = Team->AsObject();
					FString TeamName = TeamObj->GetStringField("name");

					TArray<TSharedPtr<FJsonValue>> Players = TeamObj->GetArrayField("players");

					for (TSharedPtr<FJsonValue> Player : Players) {
						TSharedPtr<FJsonObject> PlayerObj = Player->AsObject();
						FString PlayerId = PlayerObj->GetStringField("playerId");

						TSharedPtr<FJsonObject> Attributes = PlayerObj->GetObjectField("attributes");
						TSharedPtr<FJsonObject> Skill = Attributes->GetObjectField("skill");
						FString SkillValue = Skill->GetStringField("valueAttribute");
						auto SkillAttributeValue = new Aws::GameLift::Server::Model::AttributeValue(FCString::Atod(*SkillValue));

						Aws::GameLift::Server::Model::Player AwsPlayerObj;

						AwsPlayerObj.SetPlayerId(TCHAR_TO_ANSI(*PlayerId));
						AwsPlayerObj.SetTeam(TCHAR_TO_ANSI(*TeamName));
						AwsPlayerObj.AddPlayerAttribute("skill", *SkillAttributeValue);

						State->PlayerIdToPlayer.Add(PlayerId, AwsPlayerObj);
					}
				}
			}
		};

		auto OnUpdateGameSession = [](Aws::GameLift::Server::Model::UpdateGameSession UpdateGameSessionObj, void* Params)
		{
			FUpdateGameSessionState* State = (FUpdateGameSessionState*)Params;

			auto Reason = UpdateGameSessionObj.GetUpdateReason();

			if (Reason == Aws::GameLift::Server::Model::UpdateReason::MATCHMAKING_DATA_UPDATED) {
				State->Reason = EUpdateReason::MATCHMAKING_DATA_UPDATED;

				auto GameSessionObj = UpdateGameSessionObj.GetGameSession();
				FString MatchmakerData = GameSessionObj.GetMatchmakerData();

				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(MatchmakerData);

				if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
					TArray<TSharedPtr<FJsonValue>> Teams = JsonObject->GetArrayField("teams");
					for (TSharedPtr<FJsonValue> Team : Teams) {
						TSharedPtr<FJsonObject> TeamObj = Team->AsObject();
						FString TeamName = TeamObj->GetStringField("name");

						TArray<TSharedPtr<FJsonValue>> Players = TeamObj->GetArrayField("players");

						for (TSharedPtr<FJsonValue> Player : Players) {
							TSharedPtr<FJsonObject> PlayerObj = Player->AsObject();
							FString PlayerId = PlayerObj->GetStringField("playerId");

							TSharedPtr<FJsonObject> Attributes = PlayerObj->GetObjectField("attributes");
							TSharedPtr<FJsonObject> Skill = Attributes->GetObjectField("skill");
							FString SkillValue = Skill->GetStringField("valueAttribute");
							auto SkillAttributeValue = new Aws::GameLift::Server::Model::AttributeValue(FCString::Atod(*SkillValue));

							Aws::GameLift::Server::Model::Player AwsPlayerObj;

							AwsPlayerObj.SetPlayerId(TCHAR_TO_ANSI(*PlayerId));
							AwsPlayerObj.SetTeam(TCHAR_TO_ANSI(*TeamName));
							AwsPlayerObj.AddPlayerAttribute("skill", *SkillAttributeValue);

							State->PlayerIdToPlayer.Add(PlayerId, AwsPlayerObj);
						}
					}
				}
			}
			else if (Reason == Aws::GameLift::Server::Model::UpdateReason::BACKFILL_CANCELLED) {
				State->Reason = EUpdateReason::BACKFILL_CANCELLED;
			}
			else if (Reason == Aws::GameLift::Server::Model::UpdateReason::BACKFILL_FAILED) {
				State->Reason = EUpdateReason::BACKFILL_FAILED;
			}
			else if (Reason == Aws::GameLift::Server::Model::UpdateReason::BACKFILL_TIMED_OUT) {
				State->Reason = EUpdateReason::BACKFILL_TIMED_OUT;
			}
		};

		auto OnProcessTerminate = [](void* Params)
		{
			FProcessTerminateState* State = (FProcessTerminateState*)Params;

			auto GetTerminationTimeOutcome = Aws::GameLift::Server::GetTerminationTime();
			if (GetTerminationTimeOutcome.IsSuccess()) {
				State->TerminationTime = GetTerminationTimeOutcome.GetResult();
			}

			State->Status = true;
		};

		auto OnHealthCheck = [](void* Params)
		{
			FHealthCheckState* State = (FHealthCheckState*)Params;
			State->Status = true;

			return State->Status;
		};

		TArray<FString> CommandLineTokens;
		TArray<FString> CommandLineSwitches;
		int Port = FURL::UrlConfig.DefaultPort;

		// MyFirstGameLiftServer.exe token -port=7777
		FCommandLine::Parse(FCommandLine::Get(), CommandLineTokens, CommandLineSwitches);

		for (FString Str : CommandLineSwitches) {
			FString Key;
			FString Value;

			if (Str.Split("=", &Key, &Value)) {
				if (Key.Equals("port")) {
					Port = FCString::Atoi(*Value);
				}
				else if (Key.Equals("password")) {
					ServerPassword = Value;
				}
			}
		}

		const char* LogFile = "aLogFile.txt";
		const char** LogFiles = &LogFile;
		auto LogParams = new Aws::GameLift::Server::LogParameters(LogFiles, 1);

		auto Params = new Aws::GameLift::Server::ProcessParameters(
			OnStartGameSession,
			&StartGameSessionState,
			OnUpdateGameSession,
			&UpdateGameSessionState,
			OnProcessTerminate,
			&ProcessTerminateState,
			OnHealthCheck,
			&HealthCheckState,
			Port,
			*LogParams
		);

		auto ProcessReadyOutcome = Aws::GameLift::Server::ProcessReady(*Params);
	}
#endif
	GetWorldTimerManager().SetTimer(HandleGameSessionUpdateHandle, this, &AMyFirstGameLiftGameMode::HandleGameSessionUpdate, 1.0f, true, 5.0f);
	GetWorldTimerManager().SetTimer(HandleProcessTerminationHandle, this, &AMyFirstGameLiftGameMode::HandleProcessTermination, 1.0f, true, 5.0f);
	/*if (GameState != nullptr) {
		AMyFirstGameLiftGameState* MyFirstGameLiftGameState = Cast<AMyFirstGameLiftGameState>(GameState);
		if (MyFirstGameLiftGameState != nullptr) {
			MyFirstGameLiftGameState->LatestEvent = "GameEnded";
			MyFirstGameLiftGameState->WinningTeam = "cowboys";
		}
	}*/
}

void AMyFirstGameLiftGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) {
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
#if WITH_GAMELIFT
	if (Options.Len() > 0) {
		const FString& PlayerSessionId = UGameplayStatics::ParseOption(Options, "PlayerSessionId");
		const FString& PlayerId = UGameplayStatics::ParseOption(Options, "PlayerId");

		if (PlayerSessionId.Len() > 0 && PlayerId.Len() > 0) {
			Aws::GameLift::Server::Model::DescribePlayerSessionsRequest DescribePlayerSessionsRequest;
			DescribePlayerSessionsRequest.SetPlayerSessionId(TCHAR_TO_ANSI(*PlayerSessionId));

			auto DescribePlayerSessionsOutcome = Aws::GameLift::Server::DescribePlayerSessions(DescribePlayerSessionsRequest);
			if (DescribePlayerSessionsOutcome.IsSuccess()) {
				auto DescribePlayerSessionsResult = DescribePlayerSessionsOutcome.GetResult();
				int Count = 1;
				auto PlayerSessions = DescribePlayerSessionsResult.GetPlayerSessions(Count);
				if (PlayerSessions != nullptr) {
					auto PlayerSession = PlayerSessions[0];
					FString ExpectedPlayerId = PlayerSession.GetPlayerId();
					auto PlayerStatus = PlayerSession.GetStatus();

					if (ExpectedPlayerId.Equals(PlayerId) && PlayerStatus == Aws::GameLift::Server::Model::PlayerSessionStatus::RESERVED) {
						auto AcceptPlayerSessionOutcome = Aws::GameLift::Server::AcceptPlayerSession(TCHAR_TO_ANSI(*PlayerSessionId));

						if (!AcceptPlayerSessionOutcome.IsSuccess()) {
							ErrorMessage = "Unauthorized";
						}
					}
					else {
						ErrorMessage = "Unauthorized";
					}
				}
				else {
					ErrorMessage = "Unauthorized";
				}
			}
			else {
				ErrorMessage = "Unauthorized";
			}
		}
		else {
			ErrorMessage = "Unauthorized";
		}
	}
	else {
		ErrorMessage = "Unauthorized";
	}
#endif
}

void AMyFirstGameLiftGameMode::Logout(AController* Exiting) {
#if WITH_GAMELIFT
	if (LatestBackfillTicketId.Len() > 0) {
		auto GameSessionIdOutcome = Aws::GameLift::Server::GetGameSessionId();
		if (GameSessionIdOutcome.IsSuccess()) {
			FString GameSessionId = GameSessionIdOutcome.GetResult();
			FString MatchmakingConfigurationArn = StartGameSessionState.MatchmakingConfigurationArn;
			StopBackfillRequest(GameSessionId, MatchmakingConfigurationArn, LatestBackfillTicketId);
		}
	}
	if (Exiting != nullptr) {
		APlayerState* PlayerState = Exiting->PlayerState;
		if (PlayerState != nullptr) {
			AMyFirstGameLiftPlayerState* MyFirstGameLiftPlayerState = Cast<AMyFirstGameLiftPlayerState>(PlayerState);
			const FString& PlayerSessionId = MyFirstGameLiftPlayerState->PlayerSessionId;
			if (PlayerSessionId.Len() > 0) {
				Aws::GameLift::Server::RemovePlayerSession(TCHAR_TO_ANSI(*PlayerSessionId));
			}
		}
	}
#endif
	Super::Logout(Exiting);
}

FString AMyFirstGameLiftGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) {
	FString InitializedString = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	/*if (NewPlayerController != nullptr) {
		APlayerState* PlayerState = NewPlayerController->PlayerState;
		if (PlayerState != nullptr) {
			AMyFirstGameLiftPlayerState* MyFirstGameLiftPlayerState = Cast<AMyFirstGameLiftPlayerState>(PlayerState);
			if (MyFirstGameLiftPlayerState != nullptr) {
				if (FMath::RandRange(0, 1) == 0) {
					MyFirstGameLiftPlayerState->Team = "cowboys";
				}
				else {
					MyFirstGameLiftPlayerState->Team = "aliens";
				}
			}
		}
	}*/
#if WITH_GAMELIFT
	const FString& PlayerSessionId = UGameplayStatics::ParseOption(Options, "PlayerSessionId");
	const FString& PlayerId = UGameplayStatics::ParseOption(Options, "PlayerId");

	if (NewPlayerController != nullptr) {
		APlayerState* PlayerState = NewPlayerController->PlayerState;
		if (PlayerState != nullptr) {
			AMyFirstGameLiftPlayerState* MyFirstGameLiftPlayerState = Cast<AMyFirstGameLiftPlayerState>(PlayerState);
			if (MyFirstGameLiftPlayerState != nullptr) {
				MyFirstGameLiftPlayerState->PlayerSessionId = *PlayerSessionId;
				MyFirstGameLiftPlayerState->MatchmakingPlayerId = *PlayerId;

				if (UpdateGameSessionState.PlayerIdToPlayer.Num() > 0) {
					if (UpdateGameSessionState.PlayerIdToPlayer.Contains(PlayerId)) {
						auto PlayerObj = UpdateGameSessionState.PlayerIdToPlayer.Find(PlayerId);
						FString Team = PlayerObj->GetTeam();
						MyFirstGameLiftPlayerState->Team = *Team;
					}
				}
				else if (StartGameSessionState.PlayerIdToPlayer.Num() > 0) {
					if (StartGameSessionState.PlayerIdToPlayer.Contains(PlayerId)) {
						auto PlayerObj = StartGameSessionState.PlayerIdToPlayer.Find(PlayerId);
						FString Team = PlayerObj->GetTeam();
						MyFirstGameLiftPlayerState->Team = *Team;
					}
				}
			}
		}
	}
#endif
	return InitializedString;
}

void AMyFirstGameLiftGameMode::CountDownUntilGameOver() {
	if (GameState != nullptr) {
		AMyFirstGameLiftGameState* MyFirstGameLiftGameState = Cast<AMyFirstGameLiftGameState>(GameState);
		if (MyFirstGameLiftGameState != nullptr) {
			MyFirstGameLiftGameState->LatestEvent = FString::FromInt(RemainingGameTime) + " seconds until the game is over";
		}
	}

	if (RemainingGameTime > 0) {
		RemainingGameTime--;
	}
	else {
		GetWorldTimerManager().ClearTimer(CountDownUntilGameOverHandle);
	}
}

void AMyFirstGameLiftGameMode::EndGame() {
	GetWorldTimerManager().ClearTimer(CountDownUntilGameOverHandle);
	GetWorldTimerManager().ClearTimer(EndGameHandle);
	GetWorldTimerManager().ClearTimer(PickAWinningTeamHandle);
	GetWorldTimerManager().ClearTimer(HandleProcessTerminationHandle);
	GetWorldTimerManager().ClearTimer(HandleGameSessionUpdateHandle);
	GetWorldTimerManager().ClearTimer(SuspendBackfillHandle);

#if WITH_GAMELIFT
	Aws::GameLift::Server::TerminateGameSession();
	Aws::GameLift::Server::ProcessEnding();
	FGenericPlatformMisc::RequestExit(false);
#endif
}

void AMyFirstGameLiftGameMode::PickAWinningTeam() {
	GetWorldTimerManager().ClearTimer(CountDownUntilGameOverHandle);

#if WITH_GAMELIFT
	if (GameState != nullptr) {
		AMyFirstGameLiftGameState* MyFirstGameLiftGameState = Cast<AMyFirstGameLiftGameState>(GameState);
		if (MyFirstGameLiftGameState != nullptr) {
			MyFirstGameLiftGameState->LatestEvent = "GameEnded";

			if (FMath::RandRange(0, 1) == 0) {
				MyFirstGameLiftGameState->WinningTeam = "cowboys";
			}
			else {
				MyFirstGameLiftGameState->WinningTeam = "aliens";
			}

			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
			RequestObj->SetStringField("winningTeam", MyFirstGameLiftGameState->WinningTeam);

			auto GetGameSessionIdOutcome = Aws::GameLift::Server::GetGameSessionId();
			if (GetGameSessionIdOutcome.IsSuccess()) {
				RequestObj->SetStringField("gameSessionId", GetGameSessionIdOutcome.GetResult());

				FString RequestBody;
				TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
				if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
					TSharedRef<IHttpRequest> RecordMatchResultRequest = HttpModule->CreateRequest();
					RecordMatchResultRequest->OnProcessRequestComplete().BindUObject(this, &AMyFirstGameLiftGameMode::OnRecordMatchResultResponseReceived);
					RecordMatchResultRequest->SetURL(ApiUrl + "/recordmatchresult");
					RecordMatchResultRequest->SetVerb("POST");
					RecordMatchResultRequest->SetHeader("Authorization", ServerPassword);
					RecordMatchResultRequest->SetHeader("Content-Type", "application/json");
					RecordMatchResultRequest->SetContentAsString(RequestBody);
					RecordMatchResultRequest->ProcessRequest();
				}
				else {
					GetWorldTimerManager().SetTimer(EndGameHandle, this, &AMyFirstGameLiftGameMode::EndGame, 1.0f, false, 5.0f);
				}
			}
			else {
				GetWorldTimerManager().SetTimer(EndGameHandle, this, &AMyFirstGameLiftGameMode::EndGame, 1.0f, false, 5.0f);
			}
		}
		else {
			GetWorldTimerManager().SetTimer(EndGameHandle, this, &AMyFirstGameLiftGameMode::EndGame, 1.0f, false, 5.0f);
		}
	}
	else {
		GetWorldTimerManager().SetTimer(EndGameHandle, this, &AMyFirstGameLiftGameMode::EndGame, 1.0f, false, 5.0f);
	}
#endif
}

void AMyFirstGameLiftGameMode::HandleProcessTermination() {
	if (ProcessTerminateState.Status) {
		GetWorldTimerManager().ClearTimer(CountDownUntilGameOverHandle);
		GetWorldTimerManager().ClearTimer(HandleProcessTerminationHandle);
		GetWorldTimerManager().ClearTimer(HandleGameSessionUpdateHandle);
		GetWorldTimerManager().ClearTimer(SuspendBackfillHandle);

#if WITH_GAMELIFT
		if (LatestBackfillTicketId.Len() > 0) {
			auto GameSessionIdOutcome = Aws::GameLift::Server::GetGameSessionId();
			if (GameSessionIdOutcome.IsSuccess()) {
				FString GameSessionArn = FString(GameSessionIdOutcome.GetResult());
				FString MatchmakingConfigurationArn = StartGameSessionState.MatchmakingConfigurationArn;
				StopBackfillRequest(GameSessionArn, MatchmakingConfigurationArn, LatestBackfillTicketId);
			}
		}
#endif

		FString ProcessInterruptionMessage;

		if (ProcessTerminateState.TerminationTime <= 0L) {
			ProcessInterruptionMessage = "Server process could shut down at any time";
		}
		else {
			long TimeLeft = (long)(ProcessTerminateState.TerminationTime - FDateTime::Now().ToUnixTimestamp());
			ProcessInterruptionMessage = FString::Printf(TEXT("Server process scheduled to terminate in %ld seconds"), TimeLeft);
		}

		if (GameState != nullptr) {
			AMyFirstGameLiftGameState* MyFirstGameLiftGameState = Cast<AMyFirstGameLiftGameState>(GameState);
			if (MyFirstGameLiftGameState != nullptr) {
				MyFirstGameLiftGameState->LatestEvent = ProcessInterruptionMessage;
			}
		}

		GetWorldTimerManager().SetTimer(EndGameHandle, this, &AMyFirstGameLiftGameMode::EndGame, 1.0f, false, 10.0f);
	}
}

void AMyFirstGameLiftGameMode::HandleGameSessionUpdate() {
#if WITH_GAMELIFT
	if (!GameSessionActivated) {
		if (StartGameSessionState.Status) {
			GameSessionActivated = true;

			ExpectedPlayers = StartGameSessionState.PlayerIdToPlayer;

			WaitingForPlayersToJoin = true;

			GetWorldTimerManager().SetTimer(PickAWinningTeamHandle, this, &AMyFirstGameLiftGameMode::PickAWinningTeam, 1.0f, false, (float)RemainingGameTime);
			GetWorldTimerManager().SetTimer(SuspendBackfillHandle, this, &AMyFirstGameLiftGameMode::SuspendBackfill, 1.0f, false, (float)(RemainingGameTime - 60));
			GetWorldTimerManager().SetTimer(CountDownUntilGameOverHandle, this, &AMyFirstGameLiftGameMode::CountDownUntilGameOver, 1.0f, true, 0.0f);
		}
	}
	else if (WaitingForPlayersToJoin) {
		if (TimeSpentWaitingForPlayersToJoin < 60) {
			auto GameSessionIdOutcome = Aws::GameLift::Server::GetGameSessionId();
			if (GameSessionIdOutcome.IsSuccess()) {
				FString GameSessionId = FString(GameSessionIdOutcome.GetResult());

				Aws::GameLift::Server::Model::DescribePlayerSessionsRequest DescribePlayerSessionsRequest;
				DescribePlayerSessionsRequest.SetGameSessionId(TCHAR_TO_ANSI(*GameSessionId));
				DescribePlayerSessionsRequest.SetPlayerSessionStatusFilter("RESERVED");

				auto DescribePlayerSessionsOutcome = Aws::GameLift::Server::DescribePlayerSessions(DescribePlayerSessionsRequest);
				if (DescribePlayerSessionsOutcome.IsSuccess()) {
					auto DescribePlayerSessionsResult = DescribePlayerSessionsOutcome.GetResult();
					int Count = DescribePlayerSessionsResult.GetPlayerSessionsCount();
					if (Count == 0) {
						UpdateGameSessionState.Reason = EUpdateReason::BACKFILL_COMPLETED;

						WaitingForPlayersToJoin = false;
						TimeSpentWaitingForPlayersToJoin = 0;
					}
					else {
						TimeSpentWaitingForPlayersToJoin++;
					}
				}
				else {
					TimeSpentWaitingForPlayersToJoin++;
				}
			}
			else {
				TimeSpentWaitingForPlayersToJoin++;
			}
		}
		else {
			UpdateGameSessionState.Reason = EUpdateReason::BACKFILL_COMPLETED;

			WaitingForPlayersToJoin = false;
			TimeSpentWaitingForPlayersToJoin = 0;
		}
	}
	else if (UpdateGameSessionState.Reason == EUpdateReason::MATCHMAKING_DATA_UPDATED) {
		LatestBackfillTicketId = "";
		ExpectedPlayers = UpdateGameSessionState.PlayerIdToPlayer;

		WaitingForPlayersToJoin = true;
	}
	else if (UpdateGameSessionState.Reason == EUpdateReason::BACKFILL_CANCELLED || UpdateGameSessionState.Reason == EUpdateReason::BACKFILL_COMPLETED
		|| UpdateGameSessionState.Reason == EUpdateReason::BACKFILL_FAILED || UpdateGameSessionState.Reason == EUpdateReason::BACKFILL_TIMED_OUT) {
		LatestBackfillTicketId = "";

		TArray<APlayerState*> PlayerStates = GetWorld()->GetGameState()->PlayerArray;

		TMap<FString, Aws::GameLift::Server::Model::Player> ConnectedPlayers;
		for (APlayerState* PlayerState : PlayerStates) {
			if (PlayerState != nullptr) {
				AMyFirstGameLiftPlayerState* MyFirstGameLiftPlayerState = Cast<AMyFirstGameLiftPlayerState>(PlayerState);
				if (MyFirstGameLiftPlayerState != nullptr) {
					auto PlayerObj = ExpectedPlayers.Find(MyFirstGameLiftPlayerState->MatchmakingPlayerId);
					if (PlayerObj != nullptr) {
						ConnectedPlayers.Add(MyFirstGameLiftPlayerState->MatchmakingPlayerId, *PlayerObj);
					}
				}
			}
		}

		if (ConnectedPlayers.Num() == 0) {
			EndGame();
		}
		else if (ConnectedPlayers.Num() < 4) {
			auto GameSessionIdOutcome = Aws::GameLift::Server::GetGameSessionId();
			if (GameSessionIdOutcome.IsSuccess()) {
				FString GameSessionId = FString(GameSessionIdOutcome.GetResult());
				FString MatchmakingConfigurationArn = StartGameSessionState.MatchmakingConfigurationArn;
				LatestBackfillTicketId = CreateBackfillRequest(GameSessionId, MatchmakingConfigurationArn, ConnectedPlayers);
				if (LatestBackfillTicketId.Len() > 0) {
					UpdateGameSessionState.Reason = EUpdateReason::BACKFILL_INITIATED;
				}
			}
		}
	}
#endif
}

void AMyFirstGameLiftGameMode::OnRecordMatchResultResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
	GetWorldTimerManager().SetTimer(EndGameHandle, this, &AMyFirstGameLiftGameMode::EndGame, 1.0f, false, 5.0f);
}

void AMyFirstGameLiftGameMode::SuspendBackfill() {
	GetWorldTimerManager().ClearTimer(HandleGameSessionUpdateHandle);
#if WITH_GAMELIFT
	if (LatestBackfillTicketId.Len() > 0) {
		auto GameSessionIdOutcome = Aws::GameLift::Server::GetGameSessionId();
		if (GameSessionIdOutcome.IsSuccess()) {
			FString GameSessionId = GameSessionIdOutcome.GetResult();
			FString MatchmakingConfigurationArn = StartGameSessionState.MatchmakingConfigurationArn;
			if (!StopBackfillRequest(GameSessionId, MatchmakingConfigurationArn, LatestBackfillTicketId)) {
				GetWorldTimerManager().SetTimer(SuspendBackfillHandle, this, &AMyFirstGameLiftGameMode::SuspendBackfill, 1.0f, false, 1.0f);
			}
		}
		else {
			GetWorldTimerManager().SetTimer(SuspendBackfillHandle, this, &AMyFirstGameLiftGameMode::SuspendBackfill, 1.0f, false, 1.0f);
		}
	}
#endif
}

FString AMyFirstGameLiftGameMode::CreateBackfillRequest(FString GameSessionArn, FString MatchmakingConfigurationArn, TMap<FString, Aws::GameLift::Server::Model::Player> Players) {
#if WITH_GAMELIFT
	Aws::GameLift::Server::Model::StartMatchBackfillRequest StartMatchBackfillRequest;
	StartMatchBackfillRequest.SetGameSessionArn(TCHAR_TO_ANSI(*GameSessionArn));
	StartMatchBackfillRequest.SetMatchmakingConfigurationArn(TCHAR_TO_ANSI(*MatchmakingConfigurationArn));

	for (auto& Elem : Players) {
		auto PlayerObj = Elem.Value;
		StartMatchBackfillRequest.AddPlayer(PlayerObj);
	}

	auto StartMatchBackfillOutcome = Aws::GameLift::Server::StartMatchBackfill(StartMatchBackfillRequest);
	if (StartMatchBackfillOutcome.IsSuccess()) {
		return StartMatchBackfillOutcome.GetResult().GetTicketId();
	}
	else {
		return "";
	}
#endif
	return "";
}

bool AMyFirstGameLiftGameMode::StopBackfillRequest(FString GameSessionArn, FString MatchmakingConfigurationArn, FString TicketId) {
#if WITH_GAMELIFT
	Aws::GameLift::Server::Model::StopMatchBackfillRequest StopMatchBackfillRequest;
	StopMatchBackfillRequest.SetGameSessionArn(TCHAR_TO_ANSI(*GameSessionArn));
	StopMatchBackfillRequest.SetMatchmakingConfigurationArn(TCHAR_TO_ANSI(*MatchmakingConfigurationArn));
	StopMatchBackfillRequest.SetTicketId(TCHAR_TO_ANSI(*TicketId));

	auto StopMatchBackfillOutcome = Aws::GameLift::Server::StopMatchBackfill(StopMatchBackfillRequest);

	return StopMatchBackfillOutcome.IsSuccess();
#endif
	return false;
}

/*
// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFirstGameLiftGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "MyFirstGameLiftHUD.h"

AMyFirstGameLiftGameMode::AMyFirstGameLiftGameMode() {
	static ConstructorHelpers::FClassFinder<APawn> Player(L"Blueprint'/Game/BluePrint/MotionControllerPawn.MotionControllerPawn_C'");
	if (Player.Succeeded()) {
		DefaultPawnClass = Player.Class;
		HUDClass = AMyFirstGameLiftHUD::StaticClass();
	}
}

void AMyFirstGameLiftGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Display, TEXT("!!!!!!!!!!!!!!!!!!!!!!!!!!GameModeBeginPlay!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
#if WITH_GAMELIFT
	UE_LOG(LogTemp, Display, TEXT("!!!!!!!!!!!!!!!!!!!!!!!!!!TryInitSDK!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
	auto InitSDKOutcome = Aws::GameLift::Server::InitSDK(); // aws 게임 리프트 서버 InitSDK outcome 반환, 변수 타입 길어서 auto로 함
	UE_LOG(LogTemp, Display, TEXT("!!!!!!!!!!!!!!!!!!!!!!!!!!TryInitSDKDone!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
	if (InitSDKOutcome.IsSuccess()) {
		UE_LOG(LogTemp, Display, TEXT("!!!!!!!!!!!!!!!!!!!!!!!!!!InitSDKSuccess!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));

		// GameSessionObj 여기에 매치 정보도 들어있다, 어떤 플레이어가 매치에 속해있는지 등등
		auto OnStartGameSession = [](Aws::GameLift::Server::Model::GameSession GameSessionObj, void* Params)
		{
			FStartGameSessionState* State = (FStartGameSessionState*)Params;

			State->Status = Aws::GameLift::Server::ActivateGameSession().IsSuccess();
		};

		// UpdateGameSessionObj 여기는 업데이트 되는 세션 정보, 예를 들면 기존 플레이어에 더해 새로운 플레이어들이 들어오면 그거 알려줌, 지금은 이렇게 두고 나중에 백필 하면 그때 더 작성할 계획
		auto OnUpdateGameSession = [](Aws::GameLift::Server::Model::UpdateGameSession UpdateGameSessionObj, void* Params) {
			FUpdateGameSessionState* State = (FUpdateGameSessionState*)Params;
		};

		//3가지 상황에서 호출됨
		//1. 헬스 스테이터스 체크 콜백에서 서버프로세스가 UnHealthy 일때
		//2. EC2 인스턴스에서 스팟 인터럽트가 발생하면 근데 거의 발생하지 않음
		//3. 플릿 인스턴스가 자동이든 수동이든 개수를 줄이면 그 안에든 세션들을 없애게 되는데 그때 발생

		//주어진 시간동안 게임 클린업, 플레이어에게 뭔가를 알리거나(세션이 걍 종료됐으니 다시 해라) 결과 이런거 해야함
		auto OnProcessTerminate = [](void* Params) {
			FProcessTerminateState* State = (FProcessTerminateState*)Params;

			auto GetTerminationTimeOutcome = Aws::GameLift::Server::GetTerminationTime();
			if (GetTerminationTimeOutcome.IsSuccess()) {
				State->TerminationTime = GetTerminationTimeOutcome.GetResult();
			}
			
			auto ProcessEndingOutcome = Aws::GameLift::Server::ProcessEnding();

			if (ProcessEndingOutcome.IsSuccess()) {
				State->Status = true;
				FGenericPlatformMisc::RequestExit(false); //언리얼 서버가 없어지는 도중에 뭔가 처리하지 않고 그냥 없애버리기
			}
		};

		//60초마다 호출되는데 true는 서버가 정상이라는 뜻 만약 호출이 안되거나 깨지거나 시간 지연 등등이 발생하면 GameLift에서 OnProcessTerminate가 발생함
		auto OnHealthCheck = [](void* Params) {
			FHealthCheckState* State = (FHealthCheckState*)Params;
			State->Status = true;

			return State->Status;
		};

		TArray<FString> CommandLineTokens;
		TArray<FString> CommandLineSwitches;
		int Port = FURL::UrlConfig.DefaultPort;

		// MyFirstGameLift.exe token -port=7777 여기서
		// CommandLineTokens 여기는 {MyFirstGameLift.exe, token, -port=7777} 이런식으로 들어가고
		//CommandLineSwitches 여기는 {-port=7777}이렇게 들어간다
		// 토큰은 ' ' 이거 기준, 스위치는 '-' 하이픈이나 대시 기준
		FCommandLine::Parse(FCommandLine::Get(), CommandLineTokens, CommandLineSwitches);

		for (FString Str : CommandLineSwitches) {
			FString Key;
			FString Value;

			if (Str.Split("=", &Key, &Value)) {
				if (Key.Equals("port")) {
					Port = FCString::Atoi(*Value);
				}
			}
		}

		const char* LogFile = "aLogFile.txt";
		const char** LogFiles = &LogFile;
		auto LogParams = new Aws::GameLift::Server::LogParameters(LogFiles, 1);

		auto Params = new Aws::GameLift::Server::ProcessParameters(
			OnStartGameSession,
			&StartGameSessionState,
			OnUpdateGameSession,
			&UpdateGameSessionState,
			OnProcessTerminate,
			&ProcessTerminateState,
			OnHealthCheck,
			&HealthCheckState,
			Port,
			*LogParams
		);

		// 다른 예제에서 사람들이 프로세스 레디를 센션 성공으로 쓰는데 왜 그러는지 모르겠다함
		// 프로세스 레디 Fail을 구체적으로 핸들링 하고싶다면 이걸 사용하라함, 근데 여기서는 일단 안하겠다함
		auto ProcessReadyOutcome = Aws::GameLift::Server::ProcessReady(*Params);
	}

#endif
}*/
