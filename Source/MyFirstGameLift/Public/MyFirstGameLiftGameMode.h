// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameLiftServerSDK.h" // ���� ����� �߰��ϴ°� ���߿� �� �� ���� �ǰ� �ٲ� �����̶�� ��
#include "GameFramework/GameMode.h"
#include "MyFirstGameLiftGameMode.generated.h"

/**
 * 
 */

USTRUCT() //�̰� �� �ᵵ ������ �𸮾� �������� ���ϰ� ������ �ۼ���
struct FStartGameSessionState { // �ݹ鿡 ���� ���Ӹ���Ʈ�� �𸮾� ������ �߰� �긴�� ����, ���� ��� ���Ӹ���Ʈ ������ ���� ������ ���������� �����ߴ��� �˰� ���� �� ���
	GENERATED_BODY();

	UPROPERTY()
		bool Status;

	FStartGameSessionState() {
		Status = false;
	}
};

USTRUCT() 
struct FUpdateGameSessionState { // ������� �� �ڼ��ϰ� �� ���� ������ ���� ������ �ϴ����̶� ���߿� �ٷ�� ��
	GENERATED_BODY();

	FUpdateGameSessionState() {

	}
};

USTRUCT()
struct FProcessTerminateState {
	GENERATED_BODY();

	UPROPERTY()
		bool Status;

	long TerminationTime; //long Ÿ���� U������Ƽ�� �� �� ����, ���߿� Log ��ȯ�ϴ� �Լ� ��ȯ�� �������� ���� ��

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
