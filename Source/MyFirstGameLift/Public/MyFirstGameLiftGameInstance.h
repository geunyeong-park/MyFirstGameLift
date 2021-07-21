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

	UPROPERTY()
		FString AccessToken;

	UPROPERTY()
		FString IdToken;

	UPROPERTY()
		FString RefreshToken;

	UPROPERTY()
		FTimerHandle RetrieveNewTokensHandle;

	UFUNCTION()
		void SetCognitoTokens(FString NewAccessToken, FString NewIdToken, FString NewRefreshToken);

private:
	FHttpModule* HttpModule;

	UPROPERTY()
		FString ApiUrl;

	UFUNCTION()
		void RetrieveNewTokens();

	void OnRetrieveNewTokensResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};