#include "ClearWidget.h"
#include "Qest9PlayerController.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void UClearWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (RestartBtn) RestartBtn->OnClicked.AddDynamic(this, &UClearWidget::OnRestartClicked);
    if (QuitBtn) QuitBtn->OnClicked.AddDynamic(this, &UClearWidget::OnQuitClicked);
}

void UClearWidget::SetResultText(bool bWin, int32 MyScore, int32 OppScore)
{
    if (ResultText) {
        ResultText->SetText(bWin ? FText::FromString(TEXT("VICTORY!")) : FText::FromString(TEXT("DEFEAT...")));
        ResultText->SetColorAndOpacity(bWin ? FLinearColor::Green : FLinearColor::Red);
    }
}

void UClearWidget::SetRestartStatus(const FString& Msg)
{
    if (StatusText) StatusText->SetText(FText::FromString(Msg));
}

void UClearWidget::OnRestartClicked()
{
    // 내 컨트롤러에게 재시작 의사 전달
    if (auto* PC = GetOwningPlayer<AQest9PlayerController>())
    {
        PC->Server_RequestToggleReady();
        if (StatusText) StatusText->SetText(FText::FromString(TEXT("Waiting for opponent..."))); // 대기 중
        RestartBtn->SetIsEnabled(false);
    }
}

void UClearWidget::OnQuitClicked()
{
    // 게임 완전 종료
    UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}