#include "Qest9GameState.h"
#include "Net/UnrealNetwork.h"

AQest9GameState::AQest9GameState()
{
    CurrentPhase = EGamePhase::Waiting;
    CurrentRound = 1;
    P1Score = 0;
    P2Score = 0;
    CurrentOuts = 0;
    AccStrikes = 0;
    AccBalls = 0;
    bP1Ready = false;
    bP2Ready = false;
}

void AQest9GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AQest9GameState, CurrentPhase);
    DOREPLIFETIME(AQest9GameState, CurrentAttackerIndex);
    DOREPLIFETIME(AQest9GameState, CurrentRound);
    DOREPLIFETIME(AQest9GameState, P1Score);
    DOREPLIFETIME(AQest9GameState, P2Score);
    DOREPLIFETIME(AQest9GameState, CurrentOuts);
    DOREPLIFETIME(AQest9GameState, AccStrikes);
    DOREPLIFETIME(AQest9GameState, AccBalls);
    DOREPLIFETIME(AQest9GameState, bP1Ready);
    DOREPLIFETIME(AQest9GameState, bP2Ready);
}