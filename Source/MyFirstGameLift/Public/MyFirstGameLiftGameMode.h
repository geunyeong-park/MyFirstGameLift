// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameLiftServerSDK.h" // 지금 헤더에 추가하는거 나중에 좀 더 말이 되게 바꿀 예정이라고 함
#include "GameFramework/GameMode.h"
#include "MyFirstGameLiftGameMode.generated.h"

/**
 * 
 */

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
