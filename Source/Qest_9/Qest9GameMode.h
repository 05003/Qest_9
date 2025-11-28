#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Qest9GameMode.generated.h"

UCLASS()
class QEST_9_API AQest9GameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AQest9GameMode();
    virtual void PostLogin(APlayerController* NewPlayer) override;

    void ProcessAttack(class AQest9PlayerController* Attacker, const TArray<int32>& Guess);
    void SetPlayerSecret(class AQest9PlayerController* PC, const TArray<int32>& Secret);
    void RequestToggleReady(class AQest9PlayerController* Requestor);

    // Helper to get sorted index (0 or 1)
    static int32 GetSortedPlayerIndex(const APlayerController* PC, const UWorld* World);

protected:
    void SwapTurn();
    void ResetMatch();
    TMap<int32, TArray<int32>> PlayerSecrets;
};