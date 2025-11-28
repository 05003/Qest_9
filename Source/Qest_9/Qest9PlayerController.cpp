#include "Qest9PlayerController.h"
#include "Qest9GameMode.h"
#include "Qest9GameState.h"

void AQest9PlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (IsLocalController() && MainWidgetClass)
    {
        Client_RestartGame();
    }
}

void AQest9PlayerController::Client_RestartGame_Implementation()
{
    if (ClearWidgetInstance && ClearWidgetInstance->IsValidLowLevel())
    {
        ClearWidgetInstance->RemoveFromParent();
        ClearWidgetInstance = nullptr;
    }

    if (MainWidget && MainWidget->IsValidLowLevel())
    {
        MainWidget->RemoveFromParent();
        MainWidget = nullptr;
    }

    // MainWidgetClass가 유효한지 확인
    if (!MainWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("MainWidgetClass is nullptr! Cannot create widget."));
        return;
    }

    MainWidget = CreateWidget<UNumberInsertWidget>(GetWorld(), MainWidgetClass);
    if (MainWidget)
    {
        {
            MainWidget->AddToViewport();
            bShowMouseCursor = true;

            FInputModeGameAndUI InputMode;
            InputMode.SetWidgetToFocus(MainWidget->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            SetInputMode(InputMode);
        }
    }
}

bool AQest9PlayerController::Server_SubmitAttack_Validate(const TArray<int32>& Guess) { return Guess.Num() == 3; }
void AQest9PlayerController::Server_SubmitAttack_Implementation(const TArray<int32>& Guess)
{
    AQest9GameMode* GM = GetWorld()->GetAuthGameMode<AQest9GameMode>();
    if (GM) GM->ProcessAttack(this, Guess);
}

bool AQest9PlayerController::Server_SetSecret_Validate(const TArray<int32>& Secret) { return Secret.Num() == 3; }
void AQest9PlayerController::Server_SetSecret_Implementation(const TArray<int32>& Secret)
{
    AQest9GameMode* GM = GetWorld()->GetAuthGameMode<AQest9GameMode>();
    if (GM) GM->SetPlayerSecret(this, Secret);
}

void AQest9PlayerController::Server_RequestToggleReady_Implementation()
{
    AQest9GameMode* GM = GetWorld()->GetAuthGameMode<AQest9GameMode>();
    if (GM) GM->RequestToggleReady(this);
}

void AQest9PlayerController::Client_OnAttackResult_Implementation(int32 AttackerIdx, const TArray<int32>& Guess, int32 S, int32 B, bool bIsNothing)
{
    if (MainWidget)
    {
        AQest9GameState* GS = GetWorld()->GetGameState<AQest9GameState>();
        FString PStr = (AttackerIdx == 0) ? TEXT("P1") : TEXT("P2");
        FString ResStr;
        if (S == 3)
        {
            ResStr = TEXT("RUN");
        }
        else if (bIsNothing)
        {
            ResStr = TEXT("OUT");
        }
        else
        {
            ResStr = FString::Printf(TEXT("%dS%dB"), S, B);
        }
        FString Log = FString::Printf(TEXT("[%s] %d%d%d -> %s"), *PStr, Guess[0], Guess[1], Guess[2], *ResStr);
        MainWidget->AddHistoryLog(Log);
        if (GS)
        {
            MainWidget->UpdateScoreboard(GS->AccStrikes, GS->AccBalls, GS->CurrentOuts);
        }
    }
}

void AQest9PlayerController::Client_ShowGameResult_Implementation(bool bWin, int32 MyScore, int32 OppScore)
{
    if (MainWidget) MainWidget->RemoveFromParent();
    if (ClearWidgetClass)
    {
        ClearWidgetInstance = CreateWidget<UClearWidget>(this, ClearWidgetClass);
        if (ClearWidgetInstance)
        {
            ClearWidgetInstance->AddToViewport();
            ClearWidgetInstance->SetResultText(bWin, MyScore, OppScore);
            bShowMouseCursor = true;
            SetInputMode(FInputModeUIOnly());
        }
    }
}