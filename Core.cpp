#include "Core.hpp"
#include "Engine.h"
#include "render.h"
#include "memory.h"
#include "vector3.h"

#define BYTEn(x, n)   (*((_BYTE*)&(x)+n))
#define BYTE1(x)   BYTEn(x,  1)         

std::ofstream porcodio2;

typedef HRESULT(__stdcall* Present_t) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(__stdcall* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef BYTE _BYTE;
typedef DWORD _DWORD;
typedef unsigned __int64 _QWORD;

ID3D11RenderTargetView* RenderTarget;
ID3D11DeviceContext* Context;
ID3D11Device* Device;
Present_t OPresent;
WNDPROC oWndProc;
HWND Window = 0;

APawn* MyPlayer = nullptr;
UWorld* World = nullptr;
AItem_Weapon_General* CurrentWeapon = nullptr;

typedef __int64(__fastcall* thookFunction)(void* address, __int64 fnc, _QWORD* original, int a);
thookFunction hookFunction = nullptr;

int Width;
int Height;

namespace Globals
{
    static bool Open = true;
    static int Tab = 0;

    static bool UnlimitedStamina = false;

    static bool ESPEnabled = false;
    static bool ESPSnaplines = false;
    static bool ESPPrivateName = false;
    static bool ESPDistance = false;
    static bool ESPBones = false;
    static bool ESPBoxes = false;
    static bool ESPWeapon = false;

    static bool NoRecoil = false;
    static bool NoSpread = false;
    static bool RapidFire = false;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static FMatrix* myMatrix = new FMatrix();

static bool IsVisible(APlayerController* PlayerController, APawn* Pawn, Vector3* ViewPoint)
{
    auto subIsVisible = reinterpret_cast<bool(__fastcall*)(APlayerController * PlayerController, APawn * Pawn, Vector3 * ViewPoint)>(LineOfSightTo);
    return subIsVisible(PlayerController, Pawn, ViewPoint);
}

static BOOL WorldToScreen(APlayerController* PlayerController, Vector3 WorldLocation, Vector2* out)
{
    auto WorldToScreen = reinterpret_cast<bool(__fastcall*)(APlayerController * PlayerController, Vector3 vWorldPos, Vector2 * vScreenPosOut, char unk)>(ProjectWorldToScreen);
    WorldToScreen(PlayerController, WorldLocation, out, (char)0);
    return true;
}

static bool GetBoneLocation(APawn* Pawn, Vector3* Output, int Id)
{
    USkeletalMeshComponent* Mesh = Pawn->Mesh;
    if (!Mesh) return false;

    auto subGetBoneMatrix = ((FMatrix * (__fastcall*)(USkeletalMeshComponent*, FMatrix*, int))(GetBoneMatrixF));
    subGetBoneMatrix(Mesh, myMatrix, Id);

    Output->x = myMatrix->M[3][0];
    Output->y = myMatrix->M[3][1];
    Output->z = myMatrix->M[3][2];

    return true;
}

void DrawSnapLines(APawn* Pawn, APlayerController* PlayerController, int8_t localid, int8_t teamid, bool isVisible)
{
    Vector3 Output3D_Root = { };
    GetBoneLocation(Pawn, &Output3D_Root, 0);

    Vector2 Output2D_Root = { };
    WorldToScreen(PlayerController, Output3D_Root, &Output2D_Root);

    RGBA LineColor = { 255, 255, 255, 255 };

    if (localid != teamid)
    {
        if (!isVisible)
            LineColor = { 255, 255, 0, 255 };
        else
            LineColor = { 255, 0, 0, 255 };

        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        if (Output2D_Root.x > 0)
        {
            DrawLine(width / 2, height, Output2D_Root.x, Output2D_Root.y, &LineColor, 0.5f);
        }
    }
}

void DrawBones(APawn* Pawn, APlayerController* PlayerController, int8_t localid, int8_t teamid, bool isVisile)
{
    Vector3 Output3D_Head7 = {};
    GetBoneLocation(Pawn, &Output3D_Head7, 7);
    Vector3 Output3D_Head6 = { };
    GetBoneLocation(Pawn, &Output3D_Head6, 6);

    Vector3 Output3D_Neck5 = { };
    GetBoneLocation(Pawn, &Output3D_Neck5, 5);

    Vector3 Output3D_Body4 = { };
    GetBoneLocation(Pawn, &Output3D_Body4, 4);
    Vector3 Output3D_Body3 = { };
    GetBoneLocation(Pawn, &Output3D_Body3, 3);
    Vector3 Output3D_Body2 = { };
    GetBoneLocation(Pawn, &Output3D_Body2, 2);
    Vector3 Output3D_Body1 = { };
    GetBoneLocation(Pawn, &Output3D_Body1, 1);

    Vector3 Output3D_LeftArm8 = { };
    GetBoneLocation(Pawn, &Output3D_LeftArm8, 8);
    Vector3 Output3D_LeftArm9 = { };
    GetBoneLocation(Pawn, &Output3D_LeftArm9, 9);
    Vector3 Output3D_LeftArm10 = { };
    GetBoneLocation(Pawn, &Output3D_LeftArm10, 10);
    Vector3 Output3D_LeftArm11 = { };
    GetBoneLocation(Pawn, &Output3D_LeftArm11, 11);

    Vector3 Output3D_RightArm27 = { };
    GetBoneLocation(Pawn, &Output3D_RightArm27, 27);
    Vector3 Output3D_RightArm28 = { };
    GetBoneLocation(Pawn, &Output3D_RightArm28, 28);
    Vector3 Output3D_RightArm29 = { };
    GetBoneLocation(Pawn, &Output3D_RightArm29, 29);
    Vector3 Output3D_RightArm30 = { };
    GetBoneLocation(Pawn, &Output3D_RightArm30, 30);

    Vector3 Output3D_LeftLeg46 = { };
    GetBoneLocation(Pawn, &Output3D_LeftLeg46, 46);
    Vector3 Output3D_LeftLeg47 = { };
    GetBoneLocation(Pawn, &Output3D_LeftLeg47, 47);
    Vector3 Output3D_LeftLeg48 = { };
    GetBoneLocation(Pawn, &Output3D_LeftLeg48, 48);
    Vector3 Output3D_LeftLeg49 = { };
    GetBoneLocation(Pawn, &Output3D_LeftLeg49, 49);

    Vector3 Output3D_RightLeg50 = { };
    GetBoneLocation(Pawn, &Output3D_RightLeg50, 50);
    Vector3 Output3D_RightLeg51 = { };
    GetBoneLocation(Pawn, &Output3D_RightLeg51, 51);
    Vector3 Output3D_RightLeg52 = { };
    GetBoneLocation(Pawn, &Output3D_RightLeg52, 52);
    Vector3 Output3D_RightLeg53 = { };
    GetBoneLocation(Pawn, &Output3D_RightLeg53, 53);
    // screen position

    Vector2 Output2D_Head7 = { };
    WorldToScreen(PlayerController, Output3D_Head7, &Output2D_Head7);
    Vector2 Output2D_Head6 = { };
    WorldToScreen(PlayerController, Output3D_Head6, &Output2D_Head6);

    Vector2 Output2D_Neck5 = { };
    WorldToScreen(PlayerController, Output3D_Neck5, &Output2D_Neck5);

    Vector2 Output2D_Body4 = { };
    WorldToScreen(PlayerController, Output3D_Body4, &Output2D_Body4);
    Vector2 Output2D_Body3 = { };
    WorldToScreen(PlayerController, Output3D_Body3, &Output2D_Body3);
    Vector2 Output2D_Body2 = { };
    WorldToScreen(PlayerController, Output3D_Body2, &Output2D_Body2);
    Vector2 Output2D_Body1 = { };
    WorldToScreen(PlayerController, Output3D_Body1, &Output2D_Body1);

    Vector2 Output2D_LeftArm8 = { };
    WorldToScreen(PlayerController, Output3D_LeftArm8, &Output2D_LeftArm8);
    Vector2 Output2D_LeftArm9 = { };
    WorldToScreen(PlayerController, Output3D_LeftArm9, &Output2D_LeftArm9);
    Vector2 Output2D_LeftArm10 = { };
    WorldToScreen(PlayerController, Output3D_LeftArm10, &Output2D_LeftArm10);
    Vector2 Output2D_LeftArm11 = { };
    WorldToScreen(PlayerController, Output3D_LeftArm11, &Output2D_LeftArm11);

    Vector2 Output2D_RightArm27 = { };
    WorldToScreen(PlayerController, Output3D_RightArm27, &Output2D_RightArm27);
    Vector2 Output2D_RightArm28 = { };
    WorldToScreen(PlayerController, Output3D_RightArm28, &Output2D_RightArm28);
    Vector2 Output2D_RightArm29 = { };
    WorldToScreen(PlayerController, Output3D_RightArm29, &Output2D_RightArm29);
    Vector2 Output2D_RightArm30 = { };
    WorldToScreen(PlayerController, Output3D_RightArm30, &Output2D_RightArm30);

    Vector2 Output2D_LeftLeg46 = { };
    WorldToScreen(PlayerController, Output3D_LeftLeg46, &Output2D_LeftLeg46);
    Vector2 Output2D_LeftLeg47 = { };
    WorldToScreen(PlayerController, Output3D_LeftLeg47, &Output2D_LeftLeg47);
    Vector2 Output2D_LeftLeg48 = { };
    WorldToScreen(PlayerController, Output3D_LeftLeg48, &Output2D_LeftLeg48);
    Vector2 Output2D_LeftLeg49 = { };
    WorldToScreen(PlayerController, Output3D_LeftLeg49, &Output2D_LeftLeg49);

    Vector2 Output2D_RightLeg50 = { };
    WorldToScreen(PlayerController, Output3D_RightLeg50, &Output2D_RightLeg50);
    Vector2 Output2D_RightLeg51 = { };
    WorldToScreen(PlayerController, Output3D_RightLeg51, &Output2D_RightLeg51);
    Vector2 Output2D_RightLeg52 = { };
    WorldToScreen(PlayerController, Output3D_RightLeg52, &Output2D_RightLeg52);
    Vector2 Output2D_RightLeg53 = { };
    WorldToScreen(PlayerController, Output3D_RightLeg53, &Output2D_RightLeg53);
    //
    RGBA LinesColor;
    if (localid != teamid)
    {
        if (!isVisile)
            LinesColor = { 255, 255, 0, 255 };
        else
            LinesColor = { 255, 0, 0, 255 };
    }
    else
        LinesColor = { 0, 255, 0, 255 };

    if (Output2D_Head7.x > 0 &&
        Output2D_Head6.x > 0 &&
        Output2D_Neck5.x > 0 &&
        Output2D_Body4.x > 0 &&
        Output2D_Body3.x > 0 &&
        Output2D_Body2.x > 0 &&
        Output2D_Body1.x > 0 &&
        Output2D_LeftArm8.x > 0 &&
        Output2D_LeftArm9.x > 0 &&
        Output2D_LeftArm10.x > 0 &&
        Output2D_LeftArm11.x > 0 &&
        Output2D_RightArm27.x > 0 &&
        Output2D_RightArm28.x > 0 &&
        Output2D_RightArm29.x > 0 &&
        Output2D_RightArm30.x > 0 &&
        Output2D_RightArm27.x > 0 &&
        Output2D_RightArm28.x > 0 &&
        Output2D_RightArm29.x > 0 &&
        Output2D_RightArm30.x > 0 &&
        Output2D_LeftLeg46.x > 0 &&
        Output2D_LeftLeg47.x > 0 &&
        Output2D_LeftLeg48.x > 0 &&
        Output2D_LeftLeg49.x > 0 &&
        Output2D_RightLeg50.x > 0 &&
        Output2D_RightLeg51.x > 0 &&
        Output2D_RightLeg52.x > 0 &&
        Output2D_RightLeg53.x > 0
        )
    {
        DrawLine(Output2D_Head7.x, Output2D_Head7.y, Output2D_Head6.x, Output2D_Head6.y, &LinesColor, 2.f);
        DrawLine(Output2D_Head6.x, Output2D_Head6.y, Output2D_Neck5.x, Output2D_Neck5.y, &LinesColor, 2.f);

        DrawLine(Output2D_Neck5.x, Output2D_Neck5.y, Output2D_Body4.x, Output2D_Body4.y, &LinesColor, 2.f);
        DrawLine(Output2D_Body4.x, Output2D_Body4.y, Output2D_Body3.x, Output2D_Body3.y, &LinesColor, 2.f);
        DrawLine(Output2D_Body3.x, Output2D_Body3.y, Output2D_Body2.x, Output2D_Body2.y, &LinesColor, 2.f);
        DrawLine(Output2D_Body2.x, Output2D_Body2.y, Output2D_Body1.x, Output2D_Body1.y, &LinesColor, 2.f);

        DrawLine(Output2D_Neck5.x, Output2D_Neck5.y, Output2D_LeftArm8.x, Output2D_LeftArm8.y, &LinesColor, 2.f);
        DrawLine(Output2D_LeftArm8.x, Output2D_LeftArm8.y, Output2D_LeftArm9.x, Output2D_LeftArm9.y, &LinesColor, 2.f);
        DrawLine(Output2D_LeftArm9.x, Output2D_LeftArm9.y, Output2D_LeftArm10.x, Output2D_LeftArm10.y, &LinesColor, 2.f);
        DrawLine(Output2D_LeftArm10.x, Output2D_LeftArm10.y, Output2D_LeftArm11.x, Output2D_LeftArm11.y, &LinesColor, 2.f);

        DrawLine(Output2D_Neck5.x, Output2D_Neck5.y, Output2D_RightArm27.x, Output2D_RightArm27.y, &LinesColor, 2.f);
        DrawLine(Output2D_RightArm27.x, Output2D_RightArm27.y, Output2D_RightArm28.x, Output2D_RightArm28.y, &LinesColor, 2.f);
        DrawLine(Output2D_RightArm28.x, Output2D_RightArm28.y, Output2D_RightArm29.x, Output2D_RightArm29.y, &LinesColor, 2.f);
        DrawLine(Output2D_RightArm29.x, Output2D_RightArm29.y, Output2D_RightArm30.x, Output2D_RightArm30.y, &LinesColor, 2.f);

        DrawLine(Output2D_Body1.x, Output2D_Body1.y, Output2D_LeftLeg46.x, Output2D_LeftLeg46.y, &LinesColor, 2.f);
        DrawLine(Output2D_LeftLeg46.x, Output2D_LeftLeg46.y, Output2D_LeftLeg47.x, Output2D_LeftLeg47.y, &LinesColor, 2.f);
        DrawLine(Output2D_LeftLeg47.x, Output2D_LeftLeg47.y, Output2D_LeftLeg48.x, Output2D_LeftLeg48.y, &LinesColor, 2.f);
        DrawLine(Output2D_LeftLeg48.x, Output2D_LeftLeg48.y, Output2D_LeftLeg49.x, Output2D_LeftLeg49.y, &LinesColor, 2.f);

        DrawLine(Output2D_Body1.x, Output2D_Body1.y, Output2D_RightLeg50.x, Output2D_RightLeg50.y, &LinesColor, 2.f);
        DrawLine(Output2D_RightLeg50.x, Output2D_RightLeg50.y, Output2D_RightLeg51.x, Output2D_RightLeg51.y, &LinesColor, 2.f);
        DrawLine(Output2D_RightLeg51.x, Output2D_RightLeg51.y, Output2D_RightLeg52.x, Output2D_RightLeg52.y, &LinesColor, 2.f);
        DrawLine(Output2D_RightLeg52.x, Output2D_RightLeg52.y, Output2D_RightLeg53.x, Output2D_RightLeg53.y, &LinesColor, 2.f);
    }
}

void DrawDistance(APawn* Pawn, APlayerController* PlayerController, int8_t localid, int8_t teamid, bool isVisile, Vector3 v1, Vector3 v2)
{
    Vector3 Output3D_Head7 = {};
    GetBoneLocation(Pawn, &Output3D_Head7, 7);

    Vector2 Output2D_Head7 = { };
    WorldToScreen(PlayerController, Output3D_Head7, &Output2D_Head7);

    RGBA TextColor;
    if (localid != teamid)
    {
        if (!isVisile)
            TextColor = { 255, 255, 0, 255 };
        else
            TextColor = { 255, 0, 0, 255 };
    }
    else
        TextColor = { 0, 255, 0, 255 };

    if (Output2D_Head7.x > 0)
    {
        std::string castDistance = std::to_string(static_cast<int>(v1.Distance(v2)));
        std::string start = "[";
        std::string end = "]";
        std::string result = start + castDistance + end;
        DrawStrokeText(Output2D_Head7.x, Output2D_Head7.y - 20.0F, &TextColor, result.c_str());
    }
}

void DrawBoxes(APawn* Pawn, APlayerController* PlayerController, int LocalId, int TeamId, bool isVisible)
{
    Vector3 Output3D_Head = { };
    GetBoneLocation(Pawn, &Output3D_Head, 6);
    // screen position
    Vector2 Output2D_Head = { };
    WorldToScreen(PlayerController, Output3D_Head, &Output2D_Head);

    Vector3 Output3D_Root = { };
    GetBoneLocation(Pawn, &Output3D_Root, 0);

    Vector2 Output2D_Root = { };
    WorldToScreen(PlayerController, Output3D_Root, &Output2D_Root);

    float BoxHeight = Output2D_Root.y - Output2D_Head.y;
    float BoxWidth = BoxHeight / 2.4f;

    RGBA BoxColor = { 255, 0, 0, 255 };

    if (LocalId != TeamId)
    {
        if (isVisible)
            BoxColor = { 255, 0, 0, 255 };
        else
            BoxColor = { 255, 255, 0, 255 };
    }
    else
        BoxColor = { 0, 255, 0, 255 };

    if (Output2D_Head.x > 0)
    {
        DrawCornerBox(Output2D_Head.x - (BoxWidth / 2.f), Output2D_Head.y, BoxWidth, BoxHeight, 2.f, &BoxColor);
    }
}

void DrawWeapon(APawn* Pawn, APlayerController* PlayerController, int8_t localid, int8_t teamid, bool isVisile, int8_t WeaponType)
{
    Vector3 Output3D_Head7 = {};
    GetBoneLocation(Pawn, &Output3D_Head7, 0);

    Vector2 Output2D_Head7 = { };
    WorldToScreen(PlayerController, Output3D_Head7, &Output2D_Head7);

    RGBA TextColor;
    if (localid != teamid)
    {
        if (!isVisile)
            TextColor = { 255, 255, 0, 255 };
        else
            TextColor = { 255, 0, 0, 255 };
    }
    else
        TextColor = { 0, 255, 0, 255 };

    if (Output2D_Head7.x > 0)
    {
        switch (WeaponType)
        {
        case 0:
            DrawStrokeText(Output2D_Head7.x, Output2D_Head7.y, &TextColor, "None");
            break;
        case 1:
            DrawStrokeText(Output2D_Head7.x, Output2D_Head7.y, &TextColor, "Rifle");
            break;
        case 2:
            DrawStrokeText(Output2D_Head7.x, Output2D_Head7.y, &TextColor, "Sniper");
            break;
        case 3:
            DrawStrokeText(Output2D_Head7.x, Output2D_Head7.y, &TextColor, "Pistol");
            break;
        case 4:
            break;
        default:
            break;
        }
    }
}

void DrawPrivateName(APawn* Pawn, APlayerController* PlayerController, int8_t localid, int8_t teamid, bool isVisile, std::string PrivateName)
{
    Vector3 Output3D_Head7 = {};
    GetBoneLocation(Pawn, &Output3D_Head7, 7);

    Vector2 Output2D_Head7 = { };
    WorldToScreen(PlayerController, Output3D_Head7, &Output2D_Head7);

    RGBA TextColor;

    if (localid != teamid)
    {
        if (!isVisile)
            TextColor = { 255, 255, 0, 255 };
        else
            TextColor = { 255, 0, 0, 255 };
    }else
        TextColor = { 0, 255, 0, 255 };

    if (Output2D_Head7.x > 0)
    {
        if (Globals::ESPDistance)
            DrawStrokeText(Output2D_Head7.x + 35.0F, Output2D_Head7.y - 20.0F, &TextColor, PrivateName.c_str());
        else
            DrawStrokeText(Output2D_Head7.x, Output2D_Head7.y - 20.0F, &TextColor, PrivateName.c_str());
    }
}

bool PtrValidation()
{
    if (World)
        if (World->OwningGameInstance && World->GameState)
            if (World->OwningGameInstance->LocalPlayers[0] && World->GameState->PlayerArray.Data[0])
                if (World->OwningGameInstance->LocalPlayers[0]->PlayerController)
                    if (World->OwningGameInstance->LocalPlayers[0]->PlayerController->AcknowledgedPawn)
                        if (World->OwningGameInstance->LocalPlayers[0]->PlayerController->AcknowledgedPawn->HealthStatsComponent)
                            if (World->OwningGameInstance->LocalPlayers[0]->PlayerController->AcknowledgedPawn->WeaponComponent)
                                if (World->OwningGameInstance->LocalPlayers[0]->PlayerController->AcknowledgedPawn->WeaponComponent->CurrentWeapon)
                                    return true;
                                else
                                    return false;
                            else
                                return false;
                        else
                            return false;
                    else
                        return false;
                else
                    return false;
            else
                return false;
        else
            return false;
    else
        return false;
}

void EnablePaste()
{
    World = *(UWorld**)(Gworld);
    
    if (PtrValidation())
    {
        UGameInstance* OwningGameInstance = World->OwningGameInstance;

        ULevel* PersistentLevel = World->PersistentLevel;

        TArray<AActor*> Actors = PersistentLevel->Actors;

        TArray<UPlayer*> LocalPlayers = OwningGameInstance->LocalPlayers;

        UPlayer* LocalPlayer = LocalPlayers[0];

        APlayerController* PlayerController = LocalPlayer->PlayerController;

        //PlayerController->GetViewportSize(Width, Height);

        MyPlayer = PlayerController->AcknowledgedPawn;

        APlayerState* PlayerState = MyPlayer->PlayerState;

        AGameStateBase* GameState = World->GameState;

        TArray<APlayerState*> PlayerArray = GameState->PlayerArray;

        CurrentWeapon = MyPlayer->WeaponComponent->CurrentWeapon;

        if(MyPlayer)
        {
            if (CurrentWeapon)
            {
                if (Globals::NoRecoil)
                {
                    CurrentWeapon->WeaponRecoil = 0.0F;
                    CurrentWeapon->WeaponRecoilAlphaPerShot = 0.0F;
                }

                if (Globals::NoSpread)
                {
                    MyPlayer->WeaponComponent->CurrentWeapon->SpreadShot = 0.0F;
                    MyPlayer->WeaponComponent->CurrentWeapon->AccuracyHip = 0.0F;
                    MyPlayer->WeaponComponent->CurrentWeapon->AccuracySight = 0.0F;
                    MyPlayer->WeaponComponent->CurrentWeapon->SpreadShot = 0.0F;
                }

                if (Globals::RapidFire)
                {
                    CurrentWeapon->TimeBetweenShots = 0.01F;
                }
            }
        }

        for (auto i = 0; i < Actors.Num(); i++) {

            if (!Actors.IsValidIndex(i)) break;

            AActor* Actor = Actors[i];

            if (!Actor || Actor == MyPlayer) continue;

            if (Actor->IsA(EnemyClass)) {

                APawn* Pawn = Actor->Instigator;

                if (!(Pawn->HealthStatsComponent->bIsAlive)) continue;

                USkeletalMeshComponent* Mesh = Pawn->Mesh;
                if (!Mesh) continue;

                APlayerState* State = Pawn->PlayerState;

                Vector3 viewPoint{};
                bool isVisible = IsVisible(PlayerController, Pawn, &viewPoint);

                FString PrivateName = Pawn->PlayerState->PlayerNamePrivate;

                if (Globals::ESPEnabled)
                {
                    if (MyPlayer->HealthStatsComponent->bIsAlive)
                    {
                        if (Globals::ESPSnaplines)
                            DrawSnapLines(Pawn, PlayerController, MyPlayer->PlayerState->TeamNum, Pawn->PlayerState->TeamNum, isVisible);

                        if (Globals::ESPDistance)
                        {
                            DrawDistance(Pawn, PlayerController, MyPlayer->PlayerState->TeamNum, Pawn->PlayerState->TeamNum, isVisible, { MyPlayer->RootComponent->RelativeLocation.X, MyPlayer->RootComponent->RelativeLocation.Y, MyPlayer->RootComponent->RelativeLocation.Z },
                                { Pawn->RootComponent->RelativeLocation.X, Pawn->RootComponent->RelativeLocation.Y, Pawn->RootComponent->RelativeLocation.Z });
                        }

                        if (Globals::ESPPrivateName)
                        {
                            DrawPrivateName(Pawn, PlayerController, MyPlayer->PlayerState->TeamNum, Pawn->PlayerState->TeamNum, isVisible, PrivateName.ToString());
                        }

                        if (Globals::ESPBones)
                            DrawBones(Pawn, PlayerController, MyPlayer->PlayerState->TeamNum, Pawn->PlayerState->TeamNum, isVisible);

                        if (Globals::ESPBoxes)
                            DrawBoxes(Pawn, PlayerController, MyPlayer->PlayerState->TeamNum, Pawn->PlayerState->TeamNum, isVisible);

                        if (Globals::ESPWeapon)
                            DrawWeapon(Pawn, PlayerController, MyPlayer->PlayerState->TeamNum, Pawn->PlayerState->TeamNum, isVisible, Pawn->WeaponComponent->CurrentWeapon->WeaponType);
                    }
                }
            }
        }
    }

}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_KEYDOWN && LOWORD(wParam) == VK_DELETE)
        Globals::Open ^= 1;

    ImGuiIO& io = ImGui::GetIO();
    POINT position;

    GetCursorPos(&position);
    ScreenToClient(Window, &position);
    io.MousePos.x = (float)position.x;
    io.MousePos.y = (float)position.y;

    if (Globals::Open)
    {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
        return true;
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT __fastcall hkPresentScene(IDXGISwapChain* pSwapChain, unsigned int SyncInterval, unsigned int Flags)
{
    static bool first = false;
    if (!first)
    {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Device)))
        {
            Device->GetImmediateContext(&Context);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            Window = sd.OutputWindow;

            ID3D11Texture2D* pBackBuffer;

            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            Device->CreateRenderTargetView(pBackBuffer, NULL, &RenderTarget);
            pBackBuffer->Release();

            oWndProc = (WNDPROC)SetWindowLongPtr(Window, GWLP_WNDPROC, (LONG_PTR)WndProc);

            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

            ImGui_ImplWin32_Init(Window);
            ImGui_ImplDX11_Init(Device, Context);

            first = true;
        }
        else
            return OPresent(pSwapChain, SyncInterval, Flags);
    }

    if (Device || Context)
    {
        ID3D11Texture2D* renderTargetTexture = nullptr;
        if (!RenderTarget)
        {
            if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&renderTargetTexture))))
            {
                Device->CreateRenderTargetView(renderTargetTexture, nullptr, &RenderTarget);
                renderTargetTexture->Release();
            }
        }
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (Globals::Open)
    {
        ImGui::GetIO().MouseDrawCursor = 1;

        ImGui::SetNextWindowSize(ImVec2(230, 300));

        auto& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_TitleBg] = ImColor(0, 0, 30, 255);
        style.Colors[ImGuiCol_TitleBgActive] = ImColor(0, 0, 30, 255);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(15, 15, 15, 50);

        style.Colors[ImGuiCol_WindowBg] = ImColor(0.0f, 0.0f, 0.10f);
        style.Colors[ImGuiCol_FrameBg] = ImColor(0.15f, 0.15f, 0.15f);

        style.Colors[ImGuiCol_Button] = ImColor(109, 6, 125);
        //style.Colors[ImGuiCol_ButtonActive] = ImColor(168, 0, 194);
        style.Colors[ImGuiCol_ButtonHovered] = ImColor(168, 0, 194);

        ImGui::Begin("Polygon");

        if (ImGui::Button("ESP", ImVec2(ImGui::GetContentRegionAvail().x / 3, 30)))
            Globals::Tab = 0;
        ImGui::SameLine();
        if (ImGui::Button("Aimbot", ImVec2(ImGui::GetContentRegionAvail().x / 2, 30)))
            Globals::Tab = 1;
        ImGui::SameLine();
        if (ImGui::Button("Misc", ImVec2(ImGui::GetContentRegionAvail().x / 1, 30)))
            Globals::Tab = 2;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        switch (Globals::Tab)
        {
        case 0:
            ImGui::Text("ESP Players");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox("Enabled", &Globals::ESPEnabled); 
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox("Snaplines", &Globals::ESPSnaplines); ImGui::Spacing();
            ImGui::Checkbox("Player Name", &Globals::ESPPrivateName); ImGui::Spacing();
            ImGui::Checkbox("Bones", &Globals::ESPBones); ImGui::Spacing();
            ImGui::Checkbox("Boxes", &Globals::ESPBoxes); ImGui::Spacing();
            ImGui::Checkbox("Distance", &Globals::ESPDistance); ImGui::Spacing();
            ImGui::Checkbox("Weapon", &Globals::ESPWeapon); ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            break;

        case 1:
            break;

        case 2:
            ImGui::Checkbox("No Recoil", &Globals::NoRecoil); ImGui::Spacing();
            ImGui::Checkbox("No Spread", &Globals::NoSpread); ImGui::Spacing();
            ImGui::Checkbox("Rapid Fire", &Globals::RapidFire); ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            break;
        }

        ImGui::End();
    }
    else
    {
        ImGui::GetIO().MouseDrawCursor = 0;
    }

    EnablePaste();

    ImGui::EndFrame();

    Context->OMSetRenderTargets(1, &RenderTarget, NULL);
    ImGui::Render();

    if (RenderTarget)
    {
        RenderTarget->Release();
        RenderTarget = nullptr;
    }

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return OPresent(pSwapChain, SyncInterval, Flags);
}

void init()
{
    if (!EngineInit()) exit(0);

    if (!GetModuleHandleA("GameOverlayRenderer64.dll"))
        exit(0);

    auto resolve_mov = [](__int64 addr) -> __int64 {
        return *(int*)(addr + 3) + addr + 7;
    };

    auto resolve_call = [](__int64 addr) -> __int64 {
        return *(int*)(addr + 1) + addr + 5;
    };

    auto present = SignatureScan("GameOverlayRenderer64.dll", "48 89 6C 24 18 48 89 74 24 20 41 56 48 83 EC 20 41 8B E8");

    static auto hookAddress = SignatureScan("GameOverlayRenderer64.dll", "48 ? ? ? ? 57 48 83 EC 30 33 C0");
    hookFunction = (thookFunction)((uintptr_t)(hookAddress));

    if (present)
        hookFunction((void*)present, (__int64)hkPresentScene, (_QWORD*)&OPresent, (__int64)1);
}
