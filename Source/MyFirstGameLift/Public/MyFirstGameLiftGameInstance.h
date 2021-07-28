#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "MyFirstGameLiftGameInstance.generated.h"

/**
 *
 */
UCLASS()
class MYFIRSTGAMELIFT_API UMyFirstGameLiftGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMyFirstGameLiftGameInstance();

	virtual void Shutdown() override;

	virtual void Init() override;

	UPROPERTY()
		FString AccessToken;

	UPROPERTY()
		FString IdToken;

	UPROPERTY()
		FString RefreshToken;

	UPROPERTY()
		FString MatchmakingTicketId;

	UPROPERTY()
		FTimerHandle RetrieveNewTokensHandle;

	UPROPERTY()
		FTimerHandle GetResponseTimeHandle;

	TDoubleLinkedList<float> PlayerLatencies;

	UFUNCTION()
		void SetCognitoTokens(FString NewAccessToken, FString NewIdToken, FString NewRefreshToken);

private:
	FHttpModule* HttpModule;

	UPROPERTY()
		FString ApiUrl;

	UPROPERTY()
		FString RegionCode;

	UFUNCTION()
		void RetrieveNewTokens();

	UFUNCTION()
		void GetResponseTime();

	void OnRetrieveNewTokensResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnGetResponseTimeResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};