#include "MyFirstGameLiftGameState.h"
#include "Net/UnrealNetwork.h"

void AMyFirstGameLiftGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyFirstGameLiftGameState, LatestEvent);
	DOREPLIFETIME(AMyFirstGameLiftGameState, WinningTeam);
}