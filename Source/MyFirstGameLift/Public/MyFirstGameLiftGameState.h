#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyFirstGameLiftGameState.generated.h"

/**
 *
 */
UCLASS()
class MYFIRSTGAMELIFT_API AMyFirstGameLiftGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	UPROPERTY(Replicated)
		FString LatestEvent;

	UPROPERTY(Replicated)
		FString WinningTeam;
};