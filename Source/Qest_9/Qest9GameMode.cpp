#include "Qest9GameMode.h"
#include "Qest9GameState.h"
#include "Qest9PlayerController.h"
#include "GameFramework/PlayerState.h"

AQest9GameMode::AQest9GameMode()
{
    GameStateClass = AQest9GameState::StaticClass();
    PlayerControllerClass = AQest9PlayerController::StaticClass();
}

int32 AQest9GameMode::GetSortedPlayerIndex(const APlayerController* PC, const UWorld* World)
{
    if (!PC || !World) return -1;
    AQest9GameState* GS = World->GetGameState<AQest9GameState>();
    if (!GS || GS->PlayerArray.Num() < 2) return -1;
    auto SortedPlayers = GS->PlayerArray;
    SortedPlayers.Sort([](const auto& A, const auto& B)
        {
            return A.GetPlayerId() < B.GetPlayerId();
        });
    for (int32 i = 0; i < SortedPlayers.Num(); ++i)
    {
        if (SortedPlayers[i] == PC->PlayerState) return i;
    }
    return -1;
}

void AQest9GameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    AQest9GameState* GS = GetGameState<AQest9GameState>();
    if (GetNumPlayers() == 2 && GS && GS->CurrentPhase == EGamePhase::Waiting)
    {
        GS->CurrentPhase = EGamePhase::Setup;
    }
}

void AQest9GameMode::SetPlayerSecret(AQest9PlayerController* PC, const TArray<int32>& Secret)
{
    AQest9GameState* GS = GetGameState<AQest9GameState>();
    if (!GS) return;
    int32 PIndex = GetSortedPlayerIndex(PC, GetWorld());
    if (PIndex == -1) return;
    PlayerSecrets.Add(PIndex, Secret);
    int32 DefenderIdx = (GS->CurrentAttackerIndex + 1) % 2;
    if (PlayerSecrets.Contains(DefenderIdx))
    {
        GS->CurrentPhase = EGamePhase::Battle;
    }
}

void AQest9GameMode::ProcessAttack(AQest9PlayerController* Attacker, const TArray<int32>& Guess)
{
    AQest9GameState* GS = GetGameState<AQest9GameState>();
    if (!GS || GS->CurrentPhase != EGamePhase::Battle) return;

    int32 AttackerIdx = GetSortedPlayerIndex(Attacker, GetWorld());
    if (AttackerIdx == -1 || AttackerIdx != GS->CurrentAttackerIndex) return;

    int32 DefenderIndex = (AttackerIdx + 1) % 2;
    if (!PlayerSecrets.Contains(DefenderIndex)) return;

    const TArray<int32>& TargetSecret = PlayerSecrets[DefenderIndex];

    int32 TurnS = 0, TurnB = 0;
    for (int32 i = 0; i < 3; i++)
    {
        for (int32 j = 0; j < 3; j++)
        {
            if (Guess[i] == TargetSecret[j])
            {
                (i == j ? TurnS++ : TurnB++);
            }
        }
    }

    bool bIsNothing = (TurnS == 0 && TurnB == 0);

    // 모든 클라이언트에 결과 전송
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (AQest9PlayerController* PC = Cast<AQest9PlayerController>(It->Get()))
        {
            PC->Client_OnAttackResult(AttackerIdx, Guess, TurnS, TurnB, bIsNothing);
        }
    }

    // ===== 핵심 로직 시작 =====

    // 1. 이번 턴에 정확히 3스트라이크 → 정답! (홈런 + 점수 + 다음 이닝)
    if (TurnS == 3)
    {
        if (AttackerIdx == 0) GS->P1Score++;
        else GS->P2Score++;

        // 승리 체크
        if (GS->P1Score >= 3 || GS->P2Score >= 3)
        {
            GS->CurrentPhase = EGamePhase::Finished;
            bool bP1Wins = (GS->P1Score > GS->P2Score);

            for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
            {
                if (AQest9PlayerController* PC = Cast<AQest9PlayerController>(It->Get()))
                {
                    int32 MyIdx = GetSortedPlayerIndex(PC, GetWorld());
                    if (MyIdx == -1) continue;
                    bool bIWin = (MyIdx == 0) ? bP1Wins : !bP1Wins;
                    int32 MyScore = (MyIdx == 0) ? GS->P1Score : GS->P2Score;
                    int32 OppScore = (MyIdx == 0) ? GS->P2Score : GS->P1Score;
                    PC->Client_ShowGameResult(bIWin, MyScore, OppScore);
                }
            }
            return;
        }

        // 정답 맞췄으니 다음 이닝으로 (아웃 안 줌!)
        PlayerSecrets.Empty();
        GS->AccStrikes = 0;
        GS->AccBalls = 0;
        GS->CurrentPhase = EGamePhase::Setup;
        return;
    }

    // 2. 정답이 아닌 경우에만 누적 계산
    GS->AccStrikes += TurnS;
    GS->AccBalls += TurnB;

    // 3. 누적 3스트라이크 → 아웃 + 초기화
    if (GS->AccStrikes >= 3)
    {
        GS->CurrentOuts++;
        GS->AccStrikes = 0;
        GS->AccBalls = 0;
    }
    // 4. 4볼 → 아웃 + 초기화
    else if (GS->AccBalls >= 4)
    {
        GS->CurrentOuts++;
        GS->AccStrikes = 0;
        GS->AccBalls = 0;
    }
    // 5. 아무것도 아닌 경우 → 아웃
    else if (bIsNothing)
    {
        GS->CurrentOuts++;
    }

    // 6. 3아웃 → 공수교대
    if (GS->CurrentOuts >= 3)
    {
        SwapTurn();
    }
}

void AQest9GameMode::SwapTurn()
{
    AQest9GameState* GS = GetGameState<AQest9GameState>();
    if (!GS) return;

    GS->CurrentAttackerIndex = (GS->CurrentAttackerIndex + 1) % 2;

    // 라운드 증가 (P2 공격 끝날 때)
    if (GS->CurrentAttackerIndex == 0)
    {
        GS->CurrentRound++;
        if (GS->CurrentRound > 3)
        {
            GS->CurrentPhase = EGamePhase::Finished;
            // 여기서도 승리 판정 추가 (필요시)
            return;
        }
    }

    GS->CurrentOuts = 0;
    GS->AccStrikes = 0;
    GS->AccBalls = 0;
    PlayerSecrets.Empty();
    GS->CurrentPhase = EGamePhase::Setup;
}

void AQest9GameMode::RequestToggleReady(AQest9PlayerController* Requestor)
{
    AQest9GameState* GS = GetGameState<AQest9GameState>();
    if (!GS) return;
    int32 PIndex = GetSortedPlayerIndex(Requestor, GetWorld());
    if (PIndex == 0) GS->bP1Ready = true;
    else if (PIndex == 1) GS->bP2Ready = true;

    if (GS->bP1Ready && GS->bP2Ready)
    {
        ResetMatch();
    }
}

void AQest9GameMode::ResetMatch()
{
    AQest9GameState* GS = GetGameState<AQest9GameState>();
    if (!GS) return;
    GS->P1Score = 0; GS->P2Score = 0;
    GS->CurrentRound = 1; GS->CurrentOuts = 0;
    GS->AccStrikes = 0; GS->AccBalls = 0;
    GS->CurrentPhase = EGamePhase::Setup;
    GS->CurrentAttackerIndex = 0;
    GS->bP1Ready = false; GS->bP2Ready = false;
    PlayerSecrets.Empty();
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        if (AQest9PlayerController* PC = Cast<AQest9PlayerController>(It->Get()))
            PC->Client_RestartGame();
}