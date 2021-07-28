#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyFirstGameLiftWidget.generated.h"

class UTextBlock;
/**
 *
 */
UCLASS()
class MYFIRSTGAMELIFT_API UMyFirstGameLiftWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY()
		FTimerHandle SetTeammateCountHandle;

	UPROPERTY()
		FTimerHandle SetLatestEventHandle;

	UPROPERTY()
		FTimerHandle SetAveragePlayerLatencyHandle;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY()
		UTextBlock* TeamNameTextBlock;

	UPROPERTY()
		UTextBlock* TeammateCountTextBlock;

	UPROPERTY()
		UTextBlock* EventTextBlock;

	UPROPERTY()
		UTextBlock* PingTextBlock;

	UFUNCTION()
		void SetTeammateCount();

	UFUNCTION()
		void SetLatestEvent();

	UFUNCTION()
		void SetAveragePlayerLatency();
};