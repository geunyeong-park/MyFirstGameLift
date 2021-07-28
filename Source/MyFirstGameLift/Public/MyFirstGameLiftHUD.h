#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyFirstGameLiftHUD.generated.h"

class UUserWidget;
/**
 *
 */
UCLASS()
class MYFIRSTGAMELIFT_API AMyFirstGameLiftHUD : public AHUD
{
	GENERATED_BODY()

public:
	AMyFirstGameLiftHUD();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
		TSubclassOf<UUserWidget> GameWidgetClass;
};