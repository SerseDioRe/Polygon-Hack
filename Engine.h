#include <Windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

uintptr_t SignatureScan(const char* module, const char* pattern);

struct FNameEntryHandle {
	uint32_t Block = 0;
	uint32_t Offset = 0;

	FNameEntryHandle(uint32_t block, uint32_t offset) : Block(block), Offset(offset) {};
	FNameEntryHandle(uint32_t id) : Block(id >> 16), Offset(id & 65535) {};
	operator uint32_t() const { return (Block << 16 | Offset); }
};

struct FNameEntry {
	uint16_t bIsWide : 1;
	uint16_t LowercaseProbeHash : 5;
	uint16_t Len : 10;
	union
	{
		char AnsiName[1024];
		wchar_t	WideName[1024];
	};

	std::string String();
};

struct FNamePool
{
	BYTE Lock[8];
	uint32_t CurrentBlock;
	uint32_t CurrentByteCursor;
	BYTE* Blocks[8192];

	FNameEntry* GetEntry(FNameEntryHandle handle) const;
};

struct FName {
	uint32_t Index;
	uint32_t Number;

	std::string GetName();
};


struct UObject {
	void** VFTable;
	uint32_t ObjectFlags;
	uint32_t InternalIndex;
	struct UClass* ClassPrivate;
	FName NamePrivate;
	UObject* OuterPrivate;

	std::string GetName();
	std::string GetFullName();
	bool IsA(void* cmp);
	void ProcessEvent(void* fn, void* parms);
};

// Class CoreUObject.Field
// Size: 0x30 (Inherited: 0x28)
struct UField : UObject {
	char UnknownData_28[0x8]; // 0x28(0x08)
};

// Class CoreUObject.Struct
// Size: 0xb0 (Inherited: 0x30)
struct UStruct : UField {
	char pad_30[0x10]; // 0x30(0x10)
	UStruct* SuperStruct; // 0x40(0x8)
	char UnknownData_48[0x68]; // 0x48(0x80)
};

// Class CoreUObject.Class
// Size: 0x230 (Inherited: 0xb0)
struct UClass : UStruct {
	char UnknownData_B0[0x180]; // 0xb0(0x180)
};

struct TUObjectArray {
	BYTE** Objects;
	BYTE* PreAllocatedObjects;
	uint32_t MaxElements;
	uint32_t NumElements;
	uint32_t MaxChunks;
	uint32_t NumChunks;

	UObject* GetObjectPtr(uint32_t id) const;
	UObject* FindObject(const char* name) const;
};

template<class T>
struct TArray
{
	friend struct FString;

public:
	inline int Num() const
	{
		return Count;
	};

	inline T& operator[](int i)
	{
		return Data[i];
	};

	inline bool IsValidIndex(int i) const
	{
		return i < Num();
	}

public:
	T* Data;
	int32_t Count;
	int32_t Max;
};

struct FString : private TArray<wchar_t>
{
	inline FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? std::wcslen(other) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};

	inline bool IsValid() const
	{
		return Data != nullptr;
	}

	inline const wchar_t* c_str() const
	{
		return Data;
	}

	std::string ToString() const
	{
		auto length = std::wcslen(Data);

		std::string str(length, '\0');

		std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

		return str;
	}
};

struct FLinearColor
{
	float R;
	float G;
	float B;
	float A;
};

struct FMatrix {
	float M[4][4];
};

struct FVector2D
{
	float X;
	float Y;
};

struct FVector {
	float X;
	float Y;
	float Z;
};

struct FRotator {
	float Pitch;
	float Yaw;
	float Roll;
};

// Class Engine.Actor 
struct AActor : UObject {
	char pad_0000[0x30]; // 0x28 (0x30) 
	BYTE bTearOff; // 0x58 (0x01)
	char pad_0001[0xBF]; // 0x59 (0xBF) 
	class APawn* Instigator; // 0x118 (0x08) 
	char pad_0002[0x10]; // 0x120 (0x10)
	class USceneComponent* RootComponent; // 0x130 (0x08)
};

// Class Engine.Controller 
struct AController : AActor {
	char pad_0000[0xF0]; // 0x138 (0xF0)
	class APlayerState* PlayerState; // 0x228(0x08)
	bool LineOfSightTo(AActor* Other);
	void SetControlRotation(FRotator NewRotation);
};

// Class Engine.PlayerController
struct APlayerController : AController {
	char pad_0000[0x68]; // 0x230 (0x68)
	class UPlayer* Player; // 0x298 (0x08)
	class APawn* AcknowledgedPawn; // 0x2a0 (0x08)
	bool ProjectWorldLocationToScreen(FVector& WorldLocation, FVector2D& ScreenLocation);
	void GetViewportSize(INT& X, INT& Y);
};

// ScriptStruct Engine.TimerHandle
// Size: 0x08 (Inherited: 0x00)
struct FTimerHandle {
	uint64_t Handle; // 0x00(0x08)
};

// Class Engine.Pawn 
struct APawn : AActor {
	char pad_0000[0x108]; // 0x138 (0x108)
	class APlayerState* PlayerState; //0x0240
	char pad_0248[16]; //0x0248
	class AController* Controller; //0x0258
	char pad_0260[32]; //0x0260
	class USkeletalMeshComponent* Mesh; //0x0280
	char pad_0288[0x308]; //0x0288
	class UHealthStatsComponent* HealthStatsComponent; //0x0590
	class UWeaponComponent* WeaponComponent; //0x0598
};

class UHealthStatsComponent
{
public:
	char pad_0000[0xE0];    // 0x000
	int8_t Health;         // 0x0E0
	bool bIsAlive;          // 0x0E1
	bool bHealthProtection; // 0x0E2
	char pad_0001[0x1];     // 0x0E3
	float Stamina;          // 0x0E4
};

class UWeaponComponent
{
public:
	char pad_0000[0x120];                         // 0x000
	class AItem_Weapon_General* CurrentWeapon;    // 0x120
	class AItem_Weapon_General* PrimaryWeapon;    // 0x128
	class AItem_Weapon_General* SecondaryWeapon;  // 0x130
};

class AItem_Weapon_General
{
public:
	char pad_0000[712]; //0x0000
	int8_t WeaponType; //0x02C8
	int8_t WeaponShootingType; //0x02C9
	char pad_02CA[2]; //0x02CA
	int32_t WeaponDamage; //0x02CC
	char pad_02D0[8]; //0x02D0
	int32_t MaxMagazineAmmo; //0x02D8
	int32_t MaxStockAmmo; //0x02DC
	float TimeBetweenShots; //0x02E0
	float WeaponRecoil; //0x02E4
	float WeaponRecoilAlphaPerShot; //0x02E8
	float AccuracyHip; //0x02EC
	float AccuracySight; //0x02F0
	float SpreadShot; //0x02F4
	float WeaponUsability; // 0x2f8
	char pad_02F8[28]; //0x02FC
	int32_t LevelRequired; //0x0318
	bool bIsPremium; //0x031C
	bool bIsAvailable; //0x031D
	char pad_0316[146]; //0x0316
	int32_t CurrentMagazineAmmo; //0x03B0
	uint16_t CurrentStockAmmo; //0x03B4
	char pad_03AE[2]; //0x03B6
	float CurrentSpread; //0x03B8
}; //Size: 0x0580

// Class Engine.Level
struct ULevel {
	char pad_0000[0x98]; // 0x00 (0x98)
	TArray<AActor*> Actors; // 0x98 (0x10)
	char pad_0001[0x10]; // 0xA8 (0x10)
	class UWorld* OwningWorld; // 0xB8 (0x08)
};

// Class Engine.GameInstance
struct UGameInstance {
	char pad_0000[0x38]; // 0x0 (0x38)
	TArray<class UPlayer*> LocalPlayers; // 0x38(0x10)
};

// Class Engine.Player
struct UPlayer {
	char pad_0000[0x30]; // 0x0 (0x30)
	class APlayerController* PlayerController; // 0x30(0x08)
	char pad_0001[0x38]; // 0x38 (0x38)
	class UGameViewportClient* ViewportClient; // 0x70 (0x08)
};

// Class Engine.PlayerState
struct APlayerState {
	char pad_0000[0x280]; // 0x0 (0x280)
	class APawn* PawnPrivate; // 0x280 (0x08)
	char pad_0001[0x78]; // 0x228 (0x78)
	FString PlayerNamePrivate; // 0x300 (0x10)
	char pad_0002[0x6C]; // 0x310 (0x28)
	int8_t TeamNum; // 0x37C (0x01)
};

// Class Engine.SkinnedMeshComponent
struct USkeletalMeshComponent : UObject {
	char pad_0000[0x00]; // 0x28
	FName GetBoneName(INT BoneIndex);
	FVector GetBoneMatrix(INT index);
};

// Class Engine.SceneComponent
struct USceneComponent {
	char pad_0000[0x11C]; // 0x0 (0x11C)
	struct FVector RelativeLocation; // 0x11C(0x0C)
	struct FRotator RelativeRotation; // 0x128(0x0C)
};

// Class Engine.World 
struct UWorld {
	char pad_0000[0x30]; //0x0000
	class ULevel* PersistentLevel; //0x0030
	char pad_0038[0xE8]; //0x0038
	class AGameStateBase* GameState; //0x0120
	char pad_0128[0x58]; //0x0128
	class UGameInstance* OwningGameInstance; //0x0180
};

class AGameStateBase
{
public:
	char pad_0000[0x238]; //0x0000
	TArray<class APlayerState*> PlayerArray; // 0x238(0x10)
}; //Size: 0x0488


// Class Engine.Canvas
struct Canvas : UObject {
	char pad_0000[0x00]; // 0x28
	void K2_DrawLine(FVector2D ScreenPositionA, FVector2D ScreenPositionB, FLOAT Thickness, FLinearColor Color);
	void K2_DrawText(FString RenderText, FVector2D ScreenPosition, FVector2D Scale, FLinearColor RenderColor, float Kerning, FLinearColor ShadowColor, FVector2D ShadowOffset, bool bCentreX, bool bCentreY, bool bOutlined, FLinearColor OutlineColor);
};

// Class Engine.GameViewportClient
struct UGameViewportClient : UObject {
	char pad_0000[0x00]; // 0x28 
};

enum BoneFNames {
	Root = 0,
	pelvis = 1,
	spine_01 = 2,
	spine_02 = 3,
	spine_03 = 4,
	clavicle_l = 5,
	upperarm_l = 6,
	lowerarm_l = 7,
	hand_l = 8,
	index_01_l = 9,
	index_02_l = 10,
	index_03_l = 11,
	middle_01_l = 12,
	middle_02_l = 13,
	middle_03_l = 14,
	pinky_01_l = 15,
	pinky_02_l = 16,
	pinky_03_l = 17,
	ring_01_l = 18,
	ring_02_l = 19,
	ring_03_l = 20,
	thumb_01_l = 21,
	thumb_02_l = 22,
	thumb_03_l = 23,
	lowerarm_twist_01_l = 24,
	upperarm_twist_01_l = 25,
	clavicle_r = 26,
	upperarm_r = 27,
	lowerarm_r = 28,
	hand_r = 29,
	index_01_r = 30,
	index_02_r = 31,
	index_03_r = 32,
	middle_01_r = 33,
	middle_02_r = 34,
	middle_03_r = 35,
	pinky_01_r = 36,
	pinky_02_r = 37,
	pinky_03_r = 38,
	ring_01_r = 39,
	ring_02_r = 40,
	ring_03_r = 41,
	thumb_01_r = 42,
	thumb_02_r = 43,
	thumb_03_r = 44,
	lowerarm_twist_01_r = 45,
	upperarm_twist_01_r = 46,
	neck_01 = 47,
	head = 48,
	thigh_l = 49,
	calf_l = 50,
	calf_twist_01_l = 51,
	foot_l = 52,
	ball_l = 53,
	thigh_twist_01_l = 54,
	thigh_r = 55,
	calf_r = 56,
	calf_twist_01_r = 57,
	foot_r = 58,
	ball_r = 59,
	thigh_twist_01_r = 60,
	ik_foot_root = 61,
	ik_foot_l = 62,
	ik_foot_r = 63,
	ik_hand_root = 64,
	ik_hand_gun = 65,
	ik_hand_l = 66,
	ik_hand_r = 67,
	knee_target_l = 68,
	knee_target_r = 69,
	RHS_ik_hand_gun = 70,
};

extern FNamePool* NamePoolData;
extern TUObjectArray* ObjObjects;
extern UObject* WorldToScreenUFunc;
extern UObject* GetViewportSizeUFunc;
extern UObject* GetBoneNameUFunc;
extern UObject* K2_DrawLineUFunc;
extern UObject* K2_DrawTextUFunc;
extern UObject* SetControlRotationUFunc;
extern UObject* LineOfSightToUFunc;
extern UObject* EnemyClass;
extern uintptr_t GetBoneMatrixF;
extern uintptr_t Gworld; 
extern uintptr_t ProjectWorldToScreen;
extern uintptr_t LineOfSightTo;
extern void(*OPostRender)(UGameViewportClient* UGameViewportClient, Canvas* Canvas);

bool EngineInit();
FVector2D GetBone(USkeletalMeshComponent* Mesh, INT index, APlayerController* PlayerController);
void GetAllBoneNames(USkeletalMeshComponent* Mesh);
