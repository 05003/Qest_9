#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"

// [Guide] 정적 함수들을 모아둔 헬퍼 클래스
class QEST_9_API Qest9Helper
{
public:
    // NetMode(실행 환경)를 문자열로 변환
    static FString GetNetModeString(const AActor* Context)
    {
        if (!Context) return TEXT("Invalid");

        // [공식문서 참고] ENetMode 열거형 처리
        switch (Context->GetNetMode())
        {
        case NM_Client:          return TEXT("Client");
        case NM_DedicatedServer: return TEXT("DedicatedServer");
        case NM_ListenServer:    return TEXT("ListenServer");
        case NM_Standalone:      return TEXT("Standalone");
        default:                 return TEXT("Unknown");
        }
    }

    // Role(권한)을 문자열로 변환
    static FString GetRoleString(const AActor* Context)
    {
        if (!Context) return TEXT("Invalid");

        // [공식문서 참고] ENetRole 열거형 처리
        switch (Context->GetLocalRole())
        {
        case ROLE_None:             return TEXT("None");
        case ROLE_SimulatedProxy:   return TEXT("SimulatedProxy"); // 다른 클라이언트
        case ROLE_AutonomousProxy:  return TEXT("AutonomousProxy"); // 내 클라이언트
        case ROLE_Authority:        return TEXT("Authority");       // 서버
        default:                    return TEXT("Unknown");
        }
    }

    // 통합 로그 출력 함수
    static void PrintLog(const AActor* Context, const FString& Message, float Time = 5.f, FColor Color = FColor::Cyan)
    {
        if (!Context) return;

        FString NetMode = GetNetModeString(Context);
        FString Role = GetRoleString(Context);

        // 최종 출력 형태: [Client][AutonomousProxy] 메시지
        FString FinalLog = FString::Printf(TEXT("[%s][%s] %s"), *NetMode, *Role, *Message);

        // 1. 화면 출력 (Dedicated Server가 아닐 때만)
        if (GEngine && Context->GetNetMode() != NM_DedicatedServer)
        {
            GEngine->AddOnScreenDebugMessage(-1, Time, Color, FinalLog);
        }

        // 2. 로그 파일 출력 (항상 기록)
        UE_LOG(LogTemp, Warning, TEXT("%s"), *FinalLog);
    }
};