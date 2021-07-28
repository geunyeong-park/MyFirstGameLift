#include "MyFirstGameLiftHUD.h"
#include "Blueprint/UserWidget.h"

AMyFirstGameLiftHUD::AMyFirstGameLiftHUD() {
	static ConstructorHelpers::FClassFinder<UUserWidget> GameObj(TEXT("/Game/UI/Widgets/UI_Game"));
	GameWidgetClass = GameObj.Class;
}

void AMyFirstGameLiftHUD::BeginPlay() {
	Super::BeginPlay();

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController != nullptr) {
		PlayerController->bShowMouseCursor = false;
	}

	if (GameWidgetClass != nullptr) {
		UUserWidget* GameWidget = CreateWidget<UUserWidget>(GetWorld(), GameWidgetClass);
		if (GameWidget != nullptr) {
			GameWidget->AddToViewport();
		}
	}
}