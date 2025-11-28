#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClearWidget.generated.h"

UCLASS()
class QEST_9_API UClearWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetResultText(bool bWin, int32 MyScore, int32 OppScore);
    void SetRestartStatus(const FString& Msg);

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget)) class UTextBlock* ResultText;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* StatusText;

    UPROPERTY(meta = (BindWidget)) class UButton* RestartBtn;
    UPROPERTY(meta = (BindWidget)) class UButton* QuitBtn;

    UFUNCTION() void OnRestartClicked();
    UFUNCTION() void OnQuitClicked();
};