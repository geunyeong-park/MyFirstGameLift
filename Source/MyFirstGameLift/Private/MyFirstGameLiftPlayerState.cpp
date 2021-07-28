#include "MyFirstGameLiftPlayerState.h"
#include "Net/UnrealNetwork.h"

void AMyFirstGameLiftPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyFirstGameLiftPlayerState, Team);
}