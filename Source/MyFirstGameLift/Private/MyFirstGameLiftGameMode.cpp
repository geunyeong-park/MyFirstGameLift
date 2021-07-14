// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFirstGameLiftGameMode.h"
#include "UObject/ConstructorHelpers.h"

AMyFirstGameLiftGameMode::AMyFirstGameLiftGameMode() {
	static ConstructorHelpers::FClassFinder<APawn> Player(L"Blueprint'/Game/BluePrint/MotionControllerPawn.MotionControllerPawn_C'");
	if (Player.Succeeded()) {
		DefaultPawnClass = Player.Class;
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
}
