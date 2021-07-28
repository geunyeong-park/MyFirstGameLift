#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyFirstGameLiftPlayerState.generated.h"

/**
 *
 */
UCLASS()
class MYFIRSTGAMELIFT_API AMyFirstGameLiftPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	UPROPERTY()
		FString PlayerSessionId;

	UPROPERTY()
		FString MatchmakingPlayerId;

	UPROPERTY(Replicated)
		FString Team;
};