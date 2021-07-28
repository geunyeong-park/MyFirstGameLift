#pragma once

#include "CoreMinimal.h"
#include "GameLiftServerSDK.h"
#include "GameFramework/GameModeBase.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "MyFirstGameLiftGameMode.generated.h"

UENUM()
enum class EUpdateReason : uint8
{
	NO_UPDATE_RECEIVED,
	BACKFILL_INITIATED,
	MATCHMAKING_DATA_UPDATED,
	BACKFILL_FAILED,
	BACKFILL_TIMED_OUT,
	BACKFILL_CANCELLED,
	BACKFILL_COMPLETED
};

USTRUCT()
struct FStartGameSessionState
{
	GENERATED_BODY();

	UPROPERTY()
		bool Status;

	UPROPERTY()
		FString MatchmakingConfigurationArn;

	TMap<FString, Aws::GameLift::Server::Model::Player> PlayerIdToPlayer;

	FStartGameSessionState() {
		Status = false;
	}
};

USTRUCT()
struct FUpdateGameSessionState
{
	GENERATED_BODY();

	UPROPERTY()
		EUpdateReason Reason;

	TMap<FString, Aws::GameLift::Server::Model::Player> PlayerIdToPlayer;

	FUpdateGameSessionState() {
		Reason = EUpdateReason::NO_UPDATE_RECEIVED;
	}
};

USTRUCT()
struct FProcessTerminateState
{
	GENERATED_BODY();

	UPROPERTY()
		bool Status;

	long TerminationTime;

	FProcessTerminateState() {
		Status = false;
		TerminationTime = 0L;
	}
};

USTRUCT()
struct FHealthCheckState
{
	GENERATED_BODY();

	UPROPERTY()
		bool Status;

	FHealthCheckState() {
		Status = false;
	}
};

UCLASS(minimalapi)
class AMyFirstGameLiftGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyFirstGameLiftGameMode();

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void Logout(AController* Exiting) override;

public:
	UPROPERTY()
		FTimerHandle CountDownUntilGameOverHandle;

	UPROPERTY()
		FTimerHandle EndGameHandle;

	UPROPERTY()
		FTimerHandle PickAWinningTeamHandle;

	UPROPERTY()
		FTimerHandle HandleProcessTerminationHandle;

	UPROPERTY()
		FTimerHandle HandleGameSessionUpdateHandle;

	UPROPERTY()
		FTimerHandle SuspendBackfillHandle;

protected:
	virtual void BeginPlay() override;

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;

private:
	FHttpModule* HttpModule;

	UPROPERTY()
		FStartGameSessionState StartGameSessionState;

	UPROPERTY()
		FUpdateGameSessionState UpdateGameSessionState;

	UPROPERTY()
		FProcessTerminateState ProcessTerminateState;

	UPROPERTY()
		FHealthCheckState HealthCheckState;

	UPROPERTY()
		FString ApiUrl;

	UPROPERTY()
		FString ServerPassword;

	UPROPERTY()
		int RemainingGameTime;

	UPROPERTY()
		bool GameSessionActivated;

	UPROPERTY()
		FString LatestBackfillTicketId;

	UPROPERTY()
		bool WaitingForPlayersToJoin;

	UPROPERTY()
		int TimeSpentWaitingForPlayersToJoin;

	TMap<FString, Aws::GameLift::Server::Model::Player> ExpectedPlayers;

	UFUNCTION()
		void CountDownUntilGameOver();

	UFUNCTION()
		void EndGame();

	UFUNCTION()
		void PickAWinningTeam();

	UFUNCTION()
		void HandleProcessTermination();

	UFUNCTION()
		void HandleGameSessionUpdate();

	UFUNCTION()
		void SuspendBackfill();

	FString CreateBackfillRequest(FString GameSessionArn, FString MatchmakingConfigurationArn, TMap<FString, Aws::GameLift::Server::Model::Player> Players);
	bool StopBackfillRequest(FString GameSessionArn, FString MatchmakingConfigurationArn, FString TicketId);
	void OnRecordMatchResultResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};

/* Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameLiftServerSDK.h" // 지금 헤더에 추가하는거 나중에 좀 더 말이 되게 바꿀 예정이라고 함
#include "GameFramework/GameMode.h"
#include "MyFirstGameLiftGameMode.generated.h"


USTRUCT() //이거 안 써도 되지만 언리얼 엔진에서 편하게 쓰려고 작성함
struct FStartGameSessionState { // 콜백에 쓰임 게임리프트랑 언리얼 서버의 중간 브릿지 역할, 예를 들어 게임리프트 서버가 게임 세션을 성공적으로 시작했는지 알고 싶을 때 사용
	GENERATED_BODY();

	UPROPERTY()
		bool Status;

	FStartGameSessionState() {
		Status = false;
	}
};

USTRUCT()
struct FUpdateGameSessionState { // 멤버변수 더 자세하게 안 쓰는 이유는 지금 베이직 하는중이라 나중에 다룬다 함
	GENERATED_BODY();

	FUpdateGameSessionState() {

	}
};

USTRUCT()
struct FProcessTerminateState {
	GENERATED_BODY();

	UPROPERTY()
		bool Status;

	long TerminationTime; //long 타입은 U프로퍼티로 쓸 수 없음, 나중에 Log 반환하는 함수 반환값 저장으로 쓴다 함

	FProcessTerminateState() {
		Status = false;
	}
};

USTRUCT()
struct FHealthCheckState {
	GENERATED_BODY();

	UPROPERTY()
		bool Status;

	FHealthCheckState() {
		Status = false;
	}
};


UCLASS()
class MYFIRSTGAMELIFT_API AMyFirstGameLiftGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AMyFirstGameLiftGameMode();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
		FStartGameSessionState StartGameSessionState;

	UPROPERTY()
		FUpdateGameSessionState UpdateGameSessionState;

	UPROPERTY()
		FProcessTerminateState ProcessTerminateState;

	UPROPERTY()
		FHealthCheckState HealthCheckState;
};
*/