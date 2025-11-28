#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Qest9GameState.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    Waiting,
    Setup,   // Defender sets secret
    Battle,  // Attacker guesses
    Finished
};

UCLASS()
class QEST_9_API AQest9GameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AQest9GameState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(Replicated, BlueprintReadOnly)
    EGamePhase CurrentPhase;

    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 CurrentAttackerIndex; // 0 or 1

    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 CurrentRound;

    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 P1Score;
    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 P2Score;

    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 CurrentOuts;
    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 AccStrikes;
    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 AccBalls;

    // Restart Handshake Flags
    UPROPERTY(Replicated, BlueprintReadOnly)
    bool bP1Ready;
    UPROPERTY(Replicated, BlueprintReadOnly)
    bool bP2Ready;
};