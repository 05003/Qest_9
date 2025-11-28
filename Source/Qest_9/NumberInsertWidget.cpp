#include "NumberInsertWidget.h"
#include "Qest9GameState.h"
#include "Qest9PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"

void UNumberInsertWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (InputBox)
    {
        InputBox->OnTextCommitted.AddDynamic(this, &UNumberInsertWidget::OnTextCommitted);
        InputBox->SetKeyboardFocus();
    }
    if (RandomBtn) RandomBtn->OnClicked.AddDynamic(this, &UNumberInsertWidget::OnRandomClicked);
    if (SubmitBtn) SubmitBtn->OnClicked.AddDynamic(this, &UNumberInsertWidget::OnSubmitClicked);

    CachedPlayerIndex = -1;
    LastPhase = EGamePhase::Waiting;
}

void UNumberInsertWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    AQest9GameState* GS = GetWorld()->GetGameState<AQest9GameState>();
    if (!GS || GS->CurrentPhase == EGamePhase::Finished) return;

    if (RoundText) RoundText->SetText(FText::FromString(FString::Printf(TEXT("ROUND %d/3"), GS->CurrentRound)));
    if (ScoreText_VS) ScoreText_VS->SetText(FText::FromString(FString::Printf(TEXT("%d : %d"), GS->P1Score, GS->P2Score)));

    int32 MyIdx = GetMyPlayerIndex();
    if (MyIdx == -1) return;

    bool bIsMyTurn = (GS->CurrentAttackerIndex == MyIdx);
    bool bSetupPhase = (GS->CurrentPhase == EGamePhase::Setup);
    bool bBattlePhase = (GS->CurrentPhase == EGamePhase::Battle);

    // Player ID + 색상
    if (PlayerIdText)
    {
        if (MyIdx == 0)
        {
            PlayerIdText->SetText(FText::FromString(TEXT("PLAYER 1 (HOST)")));
            PlayerIdText->SetColorAndOpacity(FLinearColor(0.4f, 0.8f, 1.0f, 1.0f));
        }
        else
        {
            PlayerIdText->SetText(FText::FromString(TEXT("PLAYER 2 (CLIENT)")));
            PlayerIdText->SetColorAndOpacity(FLinearColor(1.0f, 0.4f, 0.4f, 1.0f));
        }
    }

    // 턴 헤더
    if (TurnText && TurnBorder)
    {
        if (bSetupPhase)
        {
            int32 DefenderIdx = (GS->CurrentAttackerIndex + 1) % 2;
            if (MyIdx == DefenderIdx)
            {
                TurnText->SetText(FText::FromString(TEXT("YOUR TURN: SETUP SECRET")));
                TurnBorder->SetBrushColor(FLinearColor(0.0f, 0.2f, 0.4f, 1.0f));
                if (InputBox) InputBox->SetIsEnabled(true);
            }
            else
            {
                TurnText->SetText(FText::FromString(TEXT("OPPONENT: SETUP SECRET")));
                TurnBorder->SetBrushColor(FLinearColor(0.2f, 0.0f, 0.0f, 1.0f));
                if (InputBox) InputBox->SetIsEnabled(false);
            }
        }
        else if (bBattlePhase)
        {
            if (bIsMyTurn)
            {
                TurnText->SetText(FText::FromString(TEXT("YOUR TURN: ATTACK")));
                TurnBorder->SetBrushColor(FLinearColor(0.0f, 0.4f, 0.2f, 1.0f));
                if (InputBox) InputBox->SetIsEnabled(true);
            }
            else
            {
                TurnText->SetText(FText::FromString(TEXT("OPPONENT: ATTACK")));
                TurnBorder->SetBrushColor(FLinearColor(0.2f, 0.0f, 0.0f, 1.0f));
                if (InputBox) InputBox->SetIsEnabled(false);
            }
        }
    }

    UpdateScoreboard(GS->AccStrikes, GS->AccBalls, GS->CurrentOuts);
}

int32 UNumberInsertWidget::GetMyPlayerIndex()
{
    if (CachedPlayerIndex != -1) return CachedPlayerIndex;
    AQest9GameState* GS = GetWorld()->GetGameState<AQest9GameState>();

    // 이 코드가 없으면 두 클라이언트 모두 Player 2로 잘못 인식됨
    if (!GS || GS->PlayerArray.Num() < 2)
        return -1;
    // 1. IndexOfByPredicate: 여기는 포인터(TObjectPtr)로 들어옴 -> 화살표(->) 사용
    int32 Index = GS->PlayerArray.IndexOfByPredicate([this](const auto& PS)
        {
            return PS && GetOwningPlayer() && PS == GetOwningPlayer()->PlayerState;
        });
    if (Index != INDEX_NONE)
    {
        auto SortedPlayers = GS->PlayerArray;

        // 2. Sort: 여기는 객체(Reference)로 들어옴 -> 점(.) 사용
        SortedPlayers.Sort([](const auto& A, const auto& B)
            {
                // FIX [Build]: 'A && B' 제거, '->' 대신 '.' 사용
                return A.GetPlayerId() < B.GetPlayerId();
            });
        for (int32 i = 0; i < SortedPlayers.Num(); ++i)
        {
            // SortedPlayers[i]는 포인터이므로 '->' 사용 가능 (루프 내)
            if (SortedPlayers[i] == GetOwningPlayer()->PlayerState)
            {
                CachedPlayerIndex = i;
                return i;
            }
        }
    }
    return -1;
}

void UNumberInsertWidget::OnSubmitClicked()
{
    if (!InputBox) return;
    FString InputStr = InputBox->GetText().ToString();
    if (InputStr.Len() != 3 || !InputStr.IsNumeric())
    {
        SetStatusText(TEXT("Enter 3 Digits"));
        return;
    }
    TArray<int32> Arr;
    TSet<int32> UniqueSet;
    for (int32 i = 0; i < 3; i++)
    {
        int32 Val = (int32)InputStr[i] - 48;
        if (Val == 0) { SetStatusText(TEXT("0 Not Allowed")); return; }
        Arr.Add(Val);
        UniqueSet.Add(Val);
    }
    if (UniqueSet.Num() != 3)
    {
        SetStatusText(TEXT("No Duplicates!"));
        return;
    }
    AQest9PlayerController* PC = GetOwningPlayer<AQest9PlayerController>();
    AQest9GameState* GS = GetWorld()->GetGameState<AQest9GameState>();
    if (PC && GS)
    {
        int32 MyIdx = GetMyPlayerIndex();
        int32 DefIdx = (GS->CurrentAttackerIndex + 1) % 2;
        if (GS->CurrentPhase == EGamePhase::Setup && MyIdx == DefIdx)
        {
            PC->Server_SetSecret(Arr);
            InputBox->SetText(FText::GetEmpty());
            SetStatusText(TEXT("Secret Set!"));
        }
        else if (GS->CurrentPhase == EGamePhase::Battle && MyIdx == GS->CurrentAttackerIndex)
        {
            PC->Server_SubmitAttack(Arr);
            InputBox->SetText(FText::GetEmpty());
            InputBox->SetKeyboardFocus();
        }
        else
        {
            SetStatusText(TEXT("Not Your Turn"));
        }
    }
}

void UNumberInsertWidget::OnRandomClicked()
{
    TArray<int32> Cands = { 1,2,3,4,5,6,7,8,9 };
    for (int32 i = Cands.Num() - 1; i > 0; --i)
    {
        Cands.Swap(i, FMath::RandRange(0, i));
    }
    FString RndStr = FString::Printf(TEXT("%d%d%d"), Cands[0], Cands[1], Cands[2]);
    if (InputBox) InputBox->SetText(FText::FromString(RndStr));
}

void UNumberInsertWidget::OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter) OnSubmitClicked();
}

void UNumberInsertWidget::AddHistoryLog(const FString& Msg)
{
    if (!HistoryScrollBox) return;
    while (HistoryScrollBox->GetChildrenCount() > 20)
    {
        HistoryScrollBox->RemoveChildAt(0);
    }
    UTextBlock* NewLog = NewObject<UTextBlock>(HistoryScrollBox);
    NewLog->SetText(FText::FromString(Msg));
    NewLog->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.f));
    HistoryScrollBox->AddChild(NewLog);
    HistoryScrollBox->ScrollToEnd();
}

void UNumberInsertWidget::UpdateScoreboard(int32 S, int32 B, int32 O)
{
    auto MakeDots = [](int32 Count, int32 Max) -> FString
        {
            FString OutStr = TEXT("");
            for (int32 i = 0; i < Max; i++) OutStr += (i < Count) ? TEXT("O ") : TEXT("- ");
            return OutStr;
        };
    if (ScoreText_S) ScoreText_S->SetText(FText::FromString(MakeDots(S, 3)));
    if (ScoreText_B) ScoreText_B->SetText(FText::FromString(MakeDots(B, 4)));
    if (ScoreText_O) ScoreText_O->SetText(FText::FromString(MakeDots(O, 3)));
}

void UNumberInsertWidget::SetStatusText(const FString& Status)
{
    if (StatusText) StatusText->SetText(FText::FromString(Status));
}