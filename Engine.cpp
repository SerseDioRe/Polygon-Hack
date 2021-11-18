#include "engine.h"
#include "Util.h"
#include <Psapi.h>

uintptr_t SignatureScan(const char* module, const char* pattern)
{
	uintptr_t moduleAdress = 0;
	moduleAdress = (uintptr_t)GetModuleHandleA(module);

	static auto patternToByte = [](const char* pattern)
	{
		auto       bytes = std::vector<int>{};
		const auto start = const_cast<char*>(pattern);
		const auto end = const_cast<char*>(pattern) + strlen(pattern);

		for (auto current = start; current < end; ++current)
		{
			if (*current == '?')
			{
				++current;
				if (*current == '?')
					++current;
				bytes.push_back(-1);
			}
			else { bytes.push_back(strtoul(current, &current, 16)); }
		}
		return bytes;
	};

	const auto dosHeader = (PIMAGE_DOS_HEADER)moduleAdress;
	const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)moduleAdress + dosHeader->e_lfanew);

	const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
	auto       patternBytes = patternToByte(pattern);
	const auto scanBytes = reinterpret_cast<std::uint8_t*>(moduleAdress);

	const auto s = patternBytes.size();
	const auto d = patternBytes.data();

	for (auto i = 0ul; i < sizeOfImage - s; ++i)
	{
		bool found = true;
		for (auto j = 0ul; j < s; ++j)
		{
			if (scanBytes[i + j] != d[j] && d[j] != -1)
			{
				found = false;
				break;
			}
		}
		if (found) { return reinterpret_cast<uintptr_t>(&scanBytes[i]); }
	}
	return NULL;
}

std::string FNameEntry::String()
{
	if (bIsWide) { return std::string(); }
	return { AnsiName, Len };
}

FNameEntry* FNamePool::GetEntry(FNameEntryHandle handle) const
{
	return reinterpret_cast<FNameEntry*>(Blocks[handle.Block] + 2 * static_cast<uint64_t>(handle.Offset));
}

std::string FName::GetName()
{
	auto entry = NamePoolData->GetEntry(Index);
	auto name = entry->String();
	if (Number > 0)
	{
		name += '_' + std::to_string(Number);
	}
	auto pos = name.rfind('/');
	if (pos != std::string::npos)
	{
		name = name.substr(pos + 1);
	}
	return name;
}

std::string UObject::GetName()
{
	return NamePrivate.GetName();
}

std::string UObject::GetFullName()
{
	std::string name;
	for (auto outer = OuterPrivate; outer; outer = outer->OuterPrivate) { name = outer->GetName() + "." + name; }
	name = ClassPrivate->GetName() + " " + name + this->GetName();
	return name;
}

bool UObject::IsA(void* cmp)
{
	for (auto super = ClassPrivate; super; super = static_cast<UClass*>(super->SuperStruct)) { if (super == cmp) { return true; } }
	return false;
}

UObject* TUObjectArray::GetObjectPtr(uint32_t id) const
{
	if (id >= NumElements) return nullptr;
	uint64_t chunkIndex = id / 65536;
	if (chunkIndex >= NumChunks) return nullptr;
	auto chunk = Objects[chunkIndex];
	if (!chunk) return nullptr;
	uint32_t withinChunkIndex = id % 65536 * 24;
	auto item = *reinterpret_cast<UObject**>(chunk + withinChunkIndex);
	return item;
}

UObject* TUObjectArray::FindObject(const char* name) const
{
	for (auto i = 0u; i < NumElements; i++)
	{
		auto object = GetObjectPtr(i);
		if (object && object->GetFullName() == name) { return object; }
	}
	return nullptr;
}

void UObject::ProcessEvent(void* UFunction, void* Params)
{
	auto vtable = *reinterpret_cast<void***>(this);
	reinterpret_cast<void(*)(void*, void*, void*)>(vtable[68])(this, UFunction, Params);
}

FNamePool* NamePoolData = nullptr;
TUObjectArray* ObjObjects = nullptr;

UObject* WorldToScreenUFunc;
UObject* GetViewportSizeUFunc;
UObject* GetPlayerNameUFunc;
UObject* GetBoneNameUFunc;
UObject* SetControlRotationUFunc;
UObject* K2_DrawLineUFunc;
UObject* K2_DrawTextUFunc;
UObject* LineOfSightToUFunc;
UObject* EnemyClass;
UObject* Font;
uintptr_t Gworld;
uintptr_t GetBoneMatrixF;
uintptr_t ProjectWorldToScreen;
uintptr_t LineOfSightTo;

void(*OPostRender)(UGameViewportClient* UGameViewportClient, Canvas* Canvas) = nullptr;

bool APlayerController::ProjectWorldLocationToScreen(FVector& WorldLocation, FVector2D& ScreenLocation)
{
	struct {
		FVector WorldLocation;
		FVector2D ScreenLocation;
		bool bPlayerViewportRelative;
		bool ReturnValue;
	} Parameters;

	Parameters.WorldLocation = WorldLocation;
	Parameters.ScreenLocation = ScreenLocation;
	Parameters.bPlayerViewportRelative = FALSE;

	ProcessEvent(WorldToScreenUFunc, &Parameters);

	ScreenLocation = Parameters.ScreenLocation;

	return Parameters.ReturnValue;
}

bool AController::LineOfSightTo(AActor* Other)
{
	struct {
		AActor* Other;
		FVector ViewPoint;
		bool bAlternateChecks;
		bool ReturnValue;
	} Parameters;

	Parameters.Other = Other;
	Parameters.ViewPoint = FVector{ 0, 0, 0 };
	Parameters.bAlternateChecks = FALSE;

	ProcessEvent(LineOfSightToUFunc, &Parameters);

	return Parameters.ReturnValue;
}

void APlayerController::GetViewportSize(INT& X, INT& Y)
{
	struct {
		INT X;
		INT Y;
	} Parameters;

	Parameters.X = X;
	Parameters.Y = Y;

	ProcessEvent(GetViewportSizeUFunc, &Parameters);

	X = Parameters.X;
	Y = Parameters.Y;
}

void AController::SetControlRotation(FRotator NewRotation) {
	struct {
		FRotator NewRotation;
	} Parameters;

	Parameters.NewRotation = NewRotation;

	ProcessEvent(SetControlRotationUFunc, &Parameters);
}

void Canvas::K2_DrawLine(FVector2D ScreenPositionA, FVector2D ScreenPositionB, FLOAT Thickness, FLinearColor Color)
{
	struct {
		FVector2D ScreenPositionA;
		FVector2D ScreenPositionB;
		FLOAT Thickness;
		FLinearColor Color;
	} Parameters;

	Parameters.ScreenPositionA = ScreenPositionA;
	Parameters.ScreenPositionB = ScreenPositionB;
	Parameters.Thickness = Thickness;
	Parameters.Color = Color;

	ProcessEvent(K2_DrawLineUFunc, &Parameters);
}

void Canvas::K2_DrawText(FString RenderText, FVector2D ScreenPosition, FVector2D Scale, FLinearColor RenderColor, float Kerning, FLinearColor ShadowColor, FVector2D ShadowOffset, bool bCentreX, bool bCentreY, bool bOutlined, FLinearColor OutlineColor)
{
	struct {
		UObject* RenderFont; //UFont* 
		FString RenderText;
		FVector2D ScreenPosition;
		FVector2D Scale;
		FLinearColor RenderColor;
		float Kerning;
		FLinearColor ShadowColor;
		FVector2D ShadowOffset;
		bool bCentreX;
		bool bCentreY;
		bool bOutlined;
		FLinearColor OutlineColor;
	} Parameters;

	Parameters.RenderFont = Font;
	Parameters.RenderText = RenderText;
	Parameters.ScreenPosition = ScreenPosition;
	Parameters.Scale = Scale;
	Parameters.RenderColor = RenderColor;
	Parameters.Kerning = Kerning;
	Parameters.ShadowColor = ShadowColor;
	Parameters.ShadowOffset = ShadowOffset;
	Parameters.bCentreX = bCentreX;
	Parameters.bCentreY = bCentreY;
	Parameters.bOutlined = bOutlined;

	ProcessEvent(K2_DrawTextUFunc, &Parameters);
}

FVector USkeletalMeshComponent::GetBoneMatrix(INT index) {

	auto GetBoneMatrix = reinterpret_cast<FMatrix * (*)(USkeletalMeshComponent*, FMatrix*, INT)>(GetBoneMatrixF);

	FMatrix matrix;
	GetBoneMatrix(this, &matrix, index);

	return FVector({ matrix.M[3][0], matrix.M[3][1], matrix.M[3][2] });
}

FName USkeletalMeshComponent::GetBoneName(INT index) {
	struct {
		INT index;
		FName ReturnValue;
	} Parameters;

	Parameters.index = index;

	ProcessEvent(GetBoneNameUFunc, &Parameters);

	return Parameters.ReturnValue;
}

FVector2D GetBone(USkeletalMeshComponent* Mesh, INT index, APlayerController* PlayerController) {

	FVector WorldLocation = Mesh->GetBoneMatrix(index);

	FVector2D ScreenLocation;

	if (PlayerController->ProjectWorldLocationToScreen(WorldLocation, ScreenLocation)) return ScreenLocation;

	return { 0,0 };
}

void GetAllBoneNames(USkeletalMeshComponent* Mesh) {

	for (int i = 0; i < 100; i++) {
		std::string BoneName = Mesh->GetBoneName(i).GetName();
		if (BoneName.find("None") == std::string::npos)
		{
			// print it out or whatever.
		}
	}
}

bool EngineInit()
{
	auto main = GetModuleHandleA(nullptr);

	auto resolve_mov = [](__int64 addr) -> __int64 {
		return *(int*)(addr + 3) + addr + 7;
	};

	auto resolve_call = [](__int64 addr) -> __int64 {
		return *(int*)(addr + 1) + addr + 5;
	};

	static byte objSig[] = { 0x48, 0x8d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x39, 0x44, 0x24, 0x68 };
	ObjObjects = reinterpret_cast<decltype(ObjObjects)>(FindPointer(main, objSig, sizeof(objSig), 0));
	if (!ObjObjects)
		return false;

	static byte poolSig[] = { 0x48, 0x8D, 0x35, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x16 };
	NamePoolData = reinterpret_cast<decltype(NamePoolData)>(FindPointer(main, poolSig, sizeof(poolSig), 0));
	if (!NamePoolData) 
		return false;

	Gworld = resolve_mov(SignatureScan("POLYGON-Win64-Shipping.exe", "48 8B 1D ?? ?? ?? ?? 48 85 DB 74 ?? 41 B0 ??"));

	ProjectWorldToScreen = SignatureScan("POLYGON-Win64-Shipping.exe", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 41 0F B6 E9");

	LineOfSightTo = SignatureScan("POLYGON-Win64-Shipping.exe", "40 55 53 56 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 49 8B F8");

	GetBoneMatrixF = resolve_call(SignatureScan("POLYGON-Win64-Shipping.exe", "E8 ? ? ? ? 0F 10 00 0F 11 87 ? ? ? ? 0F 10 48 ? 0F 11 8F ? ? ? ? 0F 10 40 ? 0F 11 87 ? ? ? ? F3 0F 10 87 ? ? ? ?"));
	if (!GetBoneMatrixF)
		return false;

	WorldToScreenUFunc = ObjObjects->FindObject("Function Engine.PlayerController.ProjectWorldLocationToScreen");

	GetViewportSizeUFunc = ObjObjects->FindObject("Function Engine.PlayerController.GetViewportSize");

	K2_DrawLineUFunc = ObjObjects->FindObject("Function Engine.Canvas.K2_DrawLine");

	LineOfSightToUFunc = ObjObjects->FindObject("Function Engine.Controller.LineOfSightTo");

	K2_DrawTextUFunc = ObjObjects->FindObject("Function Engine.Canvas.K2_DrawText");

	GetBoneNameUFunc = ObjObjects->FindObject("Function Engine.SkinnedMeshComponent.GetBoneName");

	SetControlRotationUFunc = ObjObjects->FindObject("Function Engine.Controller.SetControlRotation");
	
	EnemyClass = ObjObjects->FindObject("Class POLYGON.PG_Character");
	
	Font = ObjObjects->FindObject("Font Roboto.Roboto");

	return true;
}
