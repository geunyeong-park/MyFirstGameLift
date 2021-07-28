#include "MyFirstGameLiftWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"
#include "MyFirstGameLiftPlayerState.h"
#include "MyFirstGameLiftGameState.h"
#include "MyFirstGameLiftGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UMyFirstGameLiftWidget::NativeConstruct() {
	Super::NativeConstruct();

	TeamNameTextBlock = (UTextBlock*)GetWidgetFromName(TEXT("TextBlock_TeamName"));
	TeammateCountTextBlock = (UTextBlock*)GetWidgetFromName(TEXT("TextBlock_TeammateCount"));
	EventTextBlock = (UTextBlock*)GetWidgetFromName(TEXT("TextBlock_Event"));
	PingTextBlock = (UTextBlock*)GetWidgetFromName(TEXT("TextBlock_Ping"));

	GetWorld()->GetTimerManager().SetTimer(SetTeammateCountHandle, this, &UMyFirstGameLiftWidget::SetTeammateCount, 1.0f, true, 1.0f);
	GetWorld()->GetTimerManager().SetTimer(SetLatestEventHandle, this, &UMyFirstGameLiftWidget::SetLatestEvent, 1.0f, true, 1.0f);
	GetWorld()->GetTimerManager().SetTimer(SetAveragePlayerLatencyHandle, this, &UMyFirstGameLiftWidget::SetAveragePlayerLatency, 1.0f, true, 1.0f);
}

void UMyFirstGameLiftWidget::NativeDestruct() {
	GetWorld()->GetTimerManager().ClearTimer(SetTeammateCountHandle);
	GetWorld()->GetTimerManager().ClearTimer(SetLatestEventHandle);
	GetWorld()->GetTimerManager().ClearTimer(SetAveragePlayerLatencyHandle);
	Super::NativeDestruct();
}

void UMyFirstGameLiftWidget::SetTeammateCount() {
	FString OwningPlayerTeam;
	APlayerState* OwningPlayerState = GetOwningPlayerState();

	if (OwningPlayerState != nullptr) {
		AMyFirstGameLiftPlayerState* OwningMyFirstGameLiftPlayerState = Cast<AMyFirstGameLiftPlayerState>(OwningPlayerState);
		if (OwningMyFirstGameLiftPlayerState != nullptr) {
			OwningPlayerTeam = OwningMyFirstGameLiftPlayerState->Team;
			TeamNameTextBlock->SetText(FText::FromString("Team Name: " + OwningPlayerTeam));
		}
	}

	if (OwningPlayerTeam.Len() > 0) {
		TArray<APlayerState*> PlayerStates = GetWorld()->GetGameState()->PlayerArray;

		int TeammateCount = 0;

		for (APlayerState* PlayerState : PlayerStates) {
			if (PlayerState != nullptr) {
				AMyFirstGameLiftPlayerState* MyFirstGameLiftPlayerState = Cast<AMyFirstGameLiftPlayerState>(PlayerState);
				if (MyFirstGameLiftPlayerState != nullptr && MyFirstGameLiftPlayerState->Team.Equals(OwningPlayerTeam)) {
					TeammateCount++;
				}
			}
		}

		TeammateCountTextBlock->SetText(FText::FromString("Teammate Count: " + FString::FromInt(TeammateCount)));
	}
}

void UMyFirstGameLiftWidget::SetLatestEvent() {
	FString LatestEvent;
	FString WinningTeam;
	AGameStateBase* GameState = GetWorld()->GetGameState();

	if (GameState != nullptr) {
		AMyFirstGameLiftGameState* MyFirstGameLiftGameState = Cast<AMyFirstGameLiftGameState>(GameState);
		if (MyFirstGameLiftGameState != nullptr) {
			LatestEvent = MyFirstGameLiftGameState->LatestEvent;
			WinningTeam = MyFirstGameLiftGameState->WinningTeam;
		}
	}

	if (LatestEvent.Len() > 0) {
		if (LatestEvent.Equals("GameEnded")) {
			FString OwningPlayerTeam;
			APlayerState* OwningPlayerState = GetOwningPlayerState();

			if (OwningPlayerState != nullptr) {
				AMyFirstGameLiftPlayerState* OwningMyFirstGameLiftPlayerState = Cast<AMyFirstGameLiftPlayerState>(OwningPlayerState);
				if (OwningMyFirstGameLiftPlayerState != nullptr) {
					OwningPlayerTeam = OwningMyFirstGameLiftPlayerState->Team;
				}
			}

			if (WinningTeam.Len() > 0 && OwningPlayerTeam.Len() > 0) {
				FString GameOverMessage = "You and the " + OwningPlayerTeam;
				if (OwningPlayerTeam.Equals(WinningTeam)) {
					EventTextBlock->SetText(FText::FromString(GameOverMessage + " won!"));
				}
				else {
					EventTextBlock->SetText(FText::FromString(GameOverMessage + " lost :("));
				}
			}
		}
		else {
			EventTextBlock->SetText(FText::FromString(LatestEvent));
		}
	}
}

void UMyFirstGameLiftWidget::SetAveragePlayerLatency() {
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance != nullptr) {
		UMyFirstGameLiftGameInstance* MyFirstGameLiftGameInstance = Cast<UMyFirstGameLiftGameInstance>(GameInstance);
		if (MyFirstGameLiftGameInstance != nullptr) {
			float TotalPlayerLatency = 0.0f;
			for (float PlayerLatency : MyFirstGameLiftGameInstance->PlayerLatencies) {
				TotalPlayerLatency += PlayerLatency;
			}

			float AveragePlayerLatency = 60.0f;

			if (TotalPlayerLatency > 0) {
				AveragePlayerLatency = TotalPlayerLatency / MyFirstGameLiftGameInstance->PlayerLatencies.Num();

				FString PingString = "Ping: " + FString::FromInt(FMath::RoundToInt(AveragePlayerLatency)) + "ms";
				PingTextBlock->SetText(FText::FromString(PingString));
			}
		}
	}
}