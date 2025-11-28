#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NumberInsertWidget.h"
#include "ClearWidget.h"
#include "Qest9PlayerController.generated.h"

UCLASS()
class QEST_9_API AQest9PlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SetSecret(const TArray<int32>& Secret);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SubmitAttack(const TArray<int32>& Guess);

    UFUNCTION(Server, Reliable)
    void Server_RequestToggleReady();

    UFUNCTION(Client, Reliable)
    void Client_OnAttackResult(int32 AttackerIdx, const TArray<int32>& Guess, int32 S, int32 B, bool bIsNothing);

    UFUNCTION(Client, Reliable)
    void Client_RestartGame();

    UFUNCTION(Client, Reliable)
    void Client_ShowGameResult(bool bWin, int32 MyScore, int32 OppScore);

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UNumberInsertWidget> MainWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UClearWidget> ClearWidgetClass;

    class UNumberInsertWidget* MainWidget;
    class UClearWidget* ClearWidgetInstance;
};