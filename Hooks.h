#pragma once
#include <atomic>
#include <d3d9.h>
#include <d3dx9math.h>
#include <vector>

#if _DEBUG
#include "Log.h"
#define printf_s(...) asi_log::Log(__VA_ARGS__)
#endif

#define GLOBAL_D3DDEVICE 0x00982BDC

inline LPDIRECT3DDEVICE9 GetGameDevice()
{
    return *(LPDIRECT3DDEVICE9*)GLOBAL_D3DDEVICE;
}

inline void SetGameDevice(LPDIRECT3DDEVICE9 device)
{
    *(LPDIRECT3DDEVICE9*)GLOBAL_D3DDEVICE = device;
}

inline LPDIRECT3DDEVICE9& g_Device = *(LPDIRECT3DDEVICE9*)GLOBAL_D3DDEVICE;

typedef HRESULT (WINAPI*PresentFn)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);
static PresentFn RealPresent = nullptr;

constexpr uintptr_t GAMEFLOWSTATUS_ADDR = 0x00925E90;
constexpr uintptr_t FASTMEM_ADDR = 0x00925B30;
constexpr uintptr_t NISINSTANCE_ADDR = 0x009885C8;
constexpr uintptr_t WORLDPRELITSHADER_OBJ_ADDR = 0x93DEBC;
constexpr uintptr_t CURRENTSHADER_OBJ_ADDR = 0x00982C80;
constexpr uintptr_t SHADER_TABLE_ADDRESS = 0x0093DE78;
constexpr uintptr_t WORLDSHADER_TABLE_ADDRESS = 0x008F9B60;
constexpr uintptr_t EViewsBase = 0x009195E0; // Replace old guess 0x008FAF30

#define FRAMECOUNTER_ADDR 0x00982B78
#define eFrameCounter *(uint32_t*)FRAMECOUNTER_ADDR

namespace std
{
    template <>
    struct hash<std::pair<void*, int>>
    {
        size_t operator()(const std::pair<void*, int>& p) const
        {
            return hash<void*>()(p.first) ^ (hash<int>()(p.second) << 1);
        }
    };
}

// typedef void(__thiscall* ApplyGraphicsSettingsFn)(void* thisptr);
typedef void (__fastcall*ApplyGraphicsSettingsFn)(void* ecx, void* edx, void* arg1);
extern ApplyGraphicsSettingsFn ApplyGraphicsSettingsOriginal; // ✅ extern = DECLARATION ONLY

typedef int (__thiscall*ApplyGraphicsManagerMain_t)(void* thisptr);
extern ApplyGraphicsManagerMain_t ApplyGraphicsManagerMainOriginal;

inline ID3DXEffect* g_SlotRetainedFx[64] = {};

extern bool g_WaitingForReset;
extern int g_ApplyGraphicsTriggerDelay;
extern void* g_ApplyGraphicsSettingsThis;
void __fastcall HookApplyGraphicsSettings(void* manager, void*, void* vtObject);

using IVisualTreatment_ResetFn = void(__thiscall*)(void* thisPtr);
// ✅ Header declaration only:
extern IVisualTreatment_ResetFn IVisualTreatment_Reset;


// NFSMW_XenonEffects imports

class bNode
{
    void* Prev;
    void* Next;
};

template <class T>
class bTNode : bNode
{
};

struct RenderState
{
    uint32_t _bf_0;
};

class TextureInfoPlatInfo : public bTNode<TextureInfoPlatInfo>
{
public:
    RenderState mRenderState;
    uint32_t type;

private:
    uint16_t Pad0;

public:
    uint16_t PunchThruValue;
    uint32_t format;
    LPDIRECT3DBASETEXTURE9 pD3DTexture;
    void* pActiveBucket;
};

class TextureInfoPlatInterface
{
public:
    TextureInfoPlatInfo* PlatInfo;
};

class TextureInfo : public TextureInfoPlatInterface, public bTNode<TextureInfo>
{
public:
    char DebugName[24];
    uint32_t NameHash;
    uint32_t ClassNameHash;

private:
    uint32_t Padding0;

public:
    uint32_t ImagePlacement;
    uint32_t PalettePlacement;
    uint32_t ImageSize;
    uint32_t PaletteSize;
    uint32_t BaseImageSize;
    uint16_t Width;
    uint16_t Height;
    uint8_t ShiftWidth;
    uint8_t ShiftHeight;
    uint8_t ImageCompressionType;
    uint8_t PaletteCompressionType;
    uint16_t NumPaletteEntries;
    uint8_t NumMipMapLevels;
    uint8_t TilableUV;
    uint8_t BiasLevel;
    uint8_t RenderingOrder;
    uint8_t ScrollType;
    uint8_t UsedFlag;
    uint8_t ApplyAlphaSorting;
    uint8_t AlphaUsageType;
    uint8_t AlphaBlendType;
    uint8_t Flags;
    uint8_t MipmapBiasType;

private:
    uint8_t Padding1;

public:
    uint16_t ScrollTimeStep;
    uint16_t ScrollSpeedS;
    uint16_t ScrollSpeedT;
    uint16_t OffsetS;
    uint16_t OffsetT;
    uint16_t ScaleS;
    uint16_t ScaleT;
    void* pTexturePack;
    void* ImageData;
    void* PaletteData;

private:
    uint32_t Padding2[2];
};


inline void*(__thiscall*FastMem_Alloc)(void* FastMem, unsigned int bytes, char* kind) = (void*(__thiscall*)(
    void*, unsigned int, char*))0x005D29D0;
inline void* (__thiscall*FastMem_Free)(void* FastMem, void* ptr, unsigned int bytes, char* kind) = (void* (__thiscall*)(
    void*, void*, unsigned int, char*))0x005D0370;
inline void* (__cdecl*FastMem_CoreAlloc)(uint32_t size, char* debug_line) = (void* (__cdecl*)(uint32_t, char*))
    0x00465A70;
inline void (__stdcall*__CxxThrowException)(int arg1, int arg2) = (void (__stdcall*)(int, int))0x007C56B0;
inline void* (__thiscall*Attrib_Instance_MW)(void* Attrib, void* AttribCollection, unsigned int unk, void* ucomlist) = (
    void* (__thiscall*)(void*, void*, unsigned int, void*))0x00452380;
inline void*(__cdecl*Attrib_DefaultDataArea)(unsigned int size) = (void*(__cdecl*)(unsigned int))0x006269B0;
inline void* (__thiscall*Attrib_Instance_Get)(void* AttribCollection, unsigned int unk, unsigned int hash) = (void* (
    __thiscall*)(void*, unsigned int, unsigned int))0x004546C0;
inline void* (__thiscall*Attrib_Attribute_GetLength)(void* AttribCollection) = (void* (__thiscall*)(void*))0x00452D40;
inline void* (__thiscall*Attrib_Dtor)(void* AttribCollection) = (void* (__thiscall*)(void*))0x00452BD0;
inline void* (__thiscall*Attrib_Instance_GetAttributePointer)(void* AttribCollection, unsigned int hash,
                                                              unsigned int unk) = (void* (__thiscall*)(
    void*, unsigned int, unsigned int))0x00454810;
inline void* (__thiscall*Attrib_RefSpec_GetCollection)(void* Attrib) = (void* (__thiscall*)(void*))0x004560D0;
inline void* (__thiscall*Attrib_Instance_Dtor)(void* AttribInstance) = (void* (__thiscall*)(void*))0x0045A430;
inline void* (__thiscall*Attrib_Instance_Refspec)(void* AttribCollection, void* refspec, unsigned int unk,
                                                  void* ucomlist) = (void* (__thiscall*)(
    void*, void*, unsigned int, void*))0x00456CB0;
inline void* (__cdecl*Attrib_FindCollection)(uint32_t param1, uint32_t param2) = (void* (__cdecl*)(uint32_t, uint32_t))
    0x00455FD0;
inline float (__cdecl*bRandom_Float_Int)(float range, int unk) = (float (__cdecl*)(float, int))0x0045D9E0;
inline int (__cdecl*bRandom_Int_Int)(int range, uint32_t* unk) = (int(__cdecl*)(int, uint32_t*))0x0045D9A0;
inline unsigned int (__cdecl*bStringHash)(const char* str) = (unsigned int(__cdecl*)(const char*))0x00460BF0;
inline TextureInfo*(__cdecl*GetTextureInfo)(unsigned int name_hash, int return_default_texture_if_not_found,
                                            int include_unloaded_textures) = (TextureInfo*(__cdecl*)(
    unsigned int, int, int))0x00503400;
inline void (__thiscall*EmitterSystem_UpdateParticles)(void* EmitterSystem, float dt) = (void (__thiscall
    *)(void*, float))0x00508C30;
inline void (__thiscall*EmitterSystem_Render)(void* EmitterSystem, void* eView) = (void(__thiscall*)(void*, void*))
    0x00503D00;
inline void (__stdcall*sub_7286D0)() = (void(__stdcall*)())0x007286D0;
inline void (__stdcall*sub_739600)() = (void(__stdcall*)())0x739600;
inline void (__stdcall*sub_6CFCE0)() = (void(__stdcall*)())0x6CFCE0;
inline void (__cdecl*ParticleSetTransform)(D3DXMATRIX* worldmatrix, uint32_t EVIEW_ID) = (void(__cdecl
    *)(D3DXMATRIX*, uint32_t))0x6C8000;
inline void (__cdecl*GameSetTexture)(void* TextureInfo, uint32_t unk) = (void(__cdecl*)(void*, uint32_t))0x006C68B0;
inline void* (*CreateResourceFile)(char* filename, int ResFileType, int unk1, int unk2, int unk3) = (void* (*)(
    char*, int, int, int, int))0x0065FD30;
inline void (__thiscall*ResourceFile_BeginLoading)(void* ResourceFile, void* callback, void* unk) = (void(__thiscall
    *)(void*, void*, void*))0x006616F0;
inline void (*ServiceResourceLoading)() = (void(*)())0x006626B0;
inline uint32_t (__stdcall*sub_6DFAF0)() = (uint32_t(__stdcall*)())0x6DFAF0;
inline uint32_t (*Attrib_StringHash32)(const char* k) = (uint32_t(*)(const char*))0x004519D0;
