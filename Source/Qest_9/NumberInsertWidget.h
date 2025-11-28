#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Qest9GameState.h"
#include "NumberInsertWidget.generated.h"

UCLASS()
class QEST_9_API UNumberInsertWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    void UpdateScoreboard(int32 S, int32 B, int32 O);
    void AddHistoryLog(const FString& Msg);
    void SetStatusText(const FString& Status);
protected:
    UPROPERTY(meta = (BindWidget)) class UTextBlock* StatusText;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* RoundText;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* ScoreText_VS;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* ScoreText_S;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* ScoreText_B;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* ScoreText_O;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TurnText;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* PlayerIdText;
    UPROPERTY(meta = (BindWidget)) class UBorder* TurnBorder;
    UPROPERTY(meta = (BindWidget)) class UEditableTextBox* InputBox;
    UPROPERTY(meta = (BindWidget)) class UScrollBox* HistoryScrollBox;
    UPROPERTY(meta = (BindWidget)) class UButton* RandomBtn;
    UPROPERTY(meta = (BindWidget)) class UButton* SubmitBtn;
    UFUNCTION() void OnSubmitClicked();
    UFUNCTION() void OnRandomClicked();
    UFUNCTION() void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
    int32 GetMyPlayerIndex();
private:
    int32 CachedPlayerIndex = -1;
    EGamePhase LastPhase = EGamePhase::Waiting;
};