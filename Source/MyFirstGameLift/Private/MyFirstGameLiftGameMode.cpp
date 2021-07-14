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
	auto InitSDKOutcome = Aws::GameLift::Server::InitSDK(); // aws ���� ����Ʈ ���� InitSDK outcome ��ȯ, ���� Ÿ�� �� auto�� ��
	UE_LOG(LogTemp, Display, TEXT("!!!!!!!!!!!!!!!!!!!!!!!!!!TryInitSDKDone!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
	if (InitSDKOutcome.IsSuccess()) {
		UE_LOG(LogTemp, Display, TEXT("!!!!!!!!!!!!!!!!!!!!!!!!!!InitSDKSuccess!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));

		// GameSessionObj ���⿡ ��ġ ������ ����ִ�, � �÷��̾ ��ġ�� �����ִ��� ���
		auto OnStartGameSession = [](Aws::GameLift::Server::Model::GameSession GameSessionObj, void* Params)
		{
			FStartGameSessionState* State = (FStartGameSessionState*)Params;

			State->Status = Aws::GameLift::Server::ActivateGameSession().IsSuccess();
		};

		// UpdateGameSessionObj ����� ������Ʈ �Ǵ� ���� ����, ���� ��� ���� �÷��̾ ���� ���ο� �÷��̾���� ������ �װ� �˷���, ������ �̷��� �ΰ� ���߿� ���� �ϸ� �׶� �� �ۼ��� ��ȹ
		auto OnUpdateGameSession = [](Aws::GameLift::Server::Model::UpdateGameSession UpdateGameSessionObj, void* Params) {
			FUpdateGameSessionState* State = (FUpdateGameSessionState*)Params;
		};

		//3���� ��Ȳ���� ȣ���
		//1. �ｺ �������ͽ� üũ �ݹ鿡�� �������μ����� UnHealthy �϶�
		//2. EC2 �ν��Ͻ����� ���� ���ͷ�Ʈ�� �߻��ϸ� �ٵ� ���� �߻����� ����
		//3. �ø� �ν��Ͻ��� �ڵ��̵� �����̵� ������ ���̸� �� �ȿ��� ���ǵ��� ���ְ� �Ǵµ� �׶� �߻�

		//�־��� �ð����� ���� Ŭ����, �÷��̾�� ������ �˸��ų�(������ �� ��������� �ٽ� �ض�) ��� �̷��� �ؾ���
		auto OnProcessTerminate = [](void* Params) {
			FProcessTerminateState* State = (FProcessTerminateState*)Params;

			auto GetTerminationTimeOutcome = Aws::GameLift::Server::GetTerminationTime();
			if (GetTerminationTimeOutcome.IsSuccess()) {
				State->TerminationTime = GetTerminationTimeOutcome.GetResult();
			}
			
			auto ProcessEndingOutcome = Aws::GameLift::Server::ProcessEnding();

			if (ProcessEndingOutcome.IsSuccess()) {
				State->Status = true;
				FGenericPlatformMisc::RequestExit(false); //�𸮾� ������ �������� ���߿� ���� ó������ �ʰ� �׳� ���ֹ�����
			}
		};

		//60�ʸ��� ȣ��Ǵµ� true�� ������ �����̶�� �� ���� ȣ���� �ȵǰų� �����ų� �ð� ���� ����� �߻��ϸ� GameLift���� OnProcessTerminate�� �߻���
		auto OnHealthCheck = [](void* Params) {
			FHealthCheckState* State = (FHealthCheckState*)Params;
			State->Status = true;

			return State->Status;
		};

		TArray<FString> CommandLineTokens;
		TArray<FString> CommandLineSwitches;
		int Port = FURL::UrlConfig.DefaultPort;

		// MyFirstGameLift.exe token -port=7777 ���⼭
		// CommandLineTokens ����� {MyFirstGameLift.exe, token, -port=7777} �̷������� ����
		//CommandLineSwitches ����� {-port=7777}�̷��� ����
		// ��ū�� ' ' �̰� ����, ����ġ�� '-' �������̳� ��� ����
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

		// �ٸ� �������� ������� ���μ��� ���� ���� �������� ���µ� �� �׷����� �𸣰ڴ���
		// ���μ��� ���� Fail�� ��ü������ �ڵ鸵 �ϰ�ʹٸ� �̰� ����϶���, �ٵ� ���⼭�� �ϴ� ���ϰڴ���
		auto ProcessReadyOutcome = Aws::GameLift::Server::ProcessReady(*Params);
	}

#endif
}
