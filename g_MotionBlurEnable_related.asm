==========================================================================================
FUNCTION 0x6C1780 sub_6C1780  (end 0x6C1985)
==========================================================================================
0x006C1780  mov     eax, dword_901840
0x006C1785  cmp     eax, g_CarEnvironmentMapEnable
0x006C178B  jge     short loc_6C1792
0x006C178D  mov     g_CarEnvironmentMapEnable, eax
0x006C1792  mov     eax, dword_901844
0x006C1797  cmp     eax, g_CarEnvironmentMapUpdateData
0x006C179D  jge     short loc_6C17A4
0x006C179F  mov     g_CarEnvironmentMapUpdateData, eax
0x006C17A4  mov     eax, dword_901848
0x006C17A9  cmp     eax, dword_9017C0
0x006C17AF  jge     short loc_6C17B6
0x006C17B1  mov     dword_9017C0, eax
0x006C17B6  mov     eax, dword_90184C
0x006C17BB  cmp     eax, dword_9017C4
0x006C17C1  jge     short loc_6C17C8
0x006C17C3  mov     dword_9017C4, eax
0x006C17C8  mov     eax, dword_901850
0x006C17CD  cmp     eax, dword_9017C8
0x006C17D3  jge     short loc_6C17DA
0x006C17D5  mov     dword_9017C8, eax
0x006C17DA  mov     eax, dword_901854
0x006C17DF  cmp     eax, dword_9017CC
0x006C17E5  jge     short loc_6C17EC
0x006C17E7  mov     dword_9017CC, eax
0x006C17EC  mov     eax, dword_901858
0x006C17F1  cmp     eax, dword_9017D0
0x006C17F7  jge     short loc_6C17FE
0x006C17F9  mov     dword_9017D0, eax
0x006C17FE  mov     eax, dword_90185C
0x006C1803  cmp     eax, g_RoadReflectionEnable
0x006C1809  jge     short loc_6C1810
0x006C180B  mov     g_RoadReflectionEnable, eax
0x006C1810  mov     eax, dword_901860
0x006C1815  cmp     eax, dword_9017D8
0x006C181B  jge     short loc_6C1822
0x006C181D  mov     dword_9017D8, eax
0x006C1822  mov     eax, dword_901864
0x006C1827  cmp     eax, g_MotionBlurEnable
0x006C182D  jge     short loc_6C1834
0x006C182F  mov     g_MotionBlurEnable, eax
0x006C1834  mov     eax, dword_901868
0x006C1839  cmp     eax, g_X360EffectsEnable
0x006C183F  jge     short loc_6C1846
0x006C1841  mov     g_X360EffectsEnable, eax
0x006C1846  mov     eax, dword_90186C
0x006C184B  cmp     eax, dword_9017E4; g_LightGlowEnable
0x006C1851  jge     short loc_6C1858
0x006C1853  mov     dword_9017E4, eax; g_LightGlowEnable
0x006C1858  mov     eax, dword_901870
0x006C185D  cmp     eax, AnimatedTextureEnable
0x006C1863  jge     short loc_6C186A
0x006C1865  mov     AnimatedTextureEnable, eax
0x006C186A  mov     eax, dword_901874
0x006C186F  cmp     eax, g_ParticleSystemEnable
0x006C1875  jge     short loc_6C187C
0x006C1877  mov     g_ParticleSystemEnable, eax
0x006C187C  mov     eax, dword_901878
0x006C1881  cmp     eax, dword_9017F0
0x006C1887  jge     short loc_6C188E
0x006C1889  mov     dword_9017F0, eax
0x006C188E  mov     eax, dword_90187C
0x006C1893  cmp     eax, g_WorldLodLevel
0x006C1899  jge     short loc_6C18A0
0x006C189B  mov     g_WorldLodLevel, eax
0x006C18A0  mov     eax, dword_901880
0x006C18A5  cmp     eax, g_CarLodLevel
0x006C18AB  jge     short loc_6C18B2
0x006C18AD  mov     g_CarLodLevel, eax
0x006C18B2  mov     eax, dword_901890
0x006C18B7  cmp     eax, g_FSAALevel
0x006C18BD  jge     short loc_6C18C4
0x006C18BF  mov     g_FSAALevel, eax
0x006C18C4  mov     eax, dword_901888
0x006C18C9  cmp     eax, dword_901800
0x006C18CF  jge     short loc_6C18D6
0x006C18D1  mov     dword_901800, eax
0x006C18D6  mov     eax, dword_901884
0x006C18DB  cmp     eax, g_OverBrightEnable
0x006C18E1  jge     short loc_6C18E8
0x006C18E3  mov     g_OverBrightEnable, eax
0x006C18E8  mov     eax, dword_901894
0x006C18ED  cmp     eax, dword_90180C
0x006C18F3  jge     short loc_6C18FA
0x006C18F5  mov     dword_90180C, eax
0x006C18FA  mov     eax, dword_901898
0x006C18FF  cmp     eax, g_RainEnable
0x006C1905  jge     short loc_6C190C
0x006C1907  mov     g_RainEnable, eax
0x006C190C  mov     eax, dword_90189C
0x006C1911  cmp     eax, dword_901814
0x006C1917  jge     short loc_6C191E
0x006C1919  mov     dword_901814, eax
0x006C191E  mov     eax, dword_9018A0
0x006C1923  cmp     eax, g_TextureFiltering
0x006C1929  jge     short loc_6C1930
0x006C192B  mov     g_TextureFiltering, eax
0x006C1930  mov     eax, dword_9018B0
0x006C1935  cmp     eax, dword_901828
0x006C193B  jge     short loc_6C1942
0x006C193D  mov     dword_901828, eax
0x006C1942  mov     eax, dword_9018B8
0x006C1947  cmp     eax, dword_901830
0x006C194D  jge     short loc_6C1954
0x006C194F  mov     dword_901830, eax
0x006C1954  mov     eax, g_RacingResolution
0x006C1959  mov     ecx, dword_93DBEC[eax*4]
0x006C1960  test    ecx, ecx
0x006C1962  jnz     short locret_6C1984
0x006C1964  test    eax, eax
0x006C1966  jl      short locret_6C1984
0x006C1968  jmp     short loc_6C1970
0x006C1970  mov     ecx, dword_93DBEC[eax*4]
0x006C1977  test    ecx, ecx
0x006C1979  jnz     short loc_6C197F
0x006C197B  dec     eax
0x006C197C  jns     short loc_6C1970
0x006C197E  retn
0x006C197F  mov     g_RacingResolution, eax
0x006C1984  retn

==========================================================================================
FUNCTION 0x6C1990 sub_6C1990  (end 0x6C1B53)
==========================================================================================
0x006C1990  mov     eax, g_PerformanceLevel
0x006C1995  sub     esp, 40h
0x006C1998  push    ebx
0x006C1999  push    esi
0x006C199A  mov     esi, 1
0x006C199F  push    edi
0x006C19A0  xor     edi, edi
0x006C19A2  cmp     eax, esi
0x006C19A4  mov     g_RacingResolution, edi
0x006C19AA  mov     g_CarEnvironmentMapEnable, edi
0x006C19B0  mov     g_CarEnvironmentMapUpdateData, edi
0x006C19B6  mov     dword_9017C0, esi
0x006C19BC  mov     dword_9017C4, edi
0x006C19C2  mov     dword_9017D0, esi
0x006C19C8  mov     g_RoadReflectionEnable, edi
0x006C19CE  mov     g_ParticleSystemEnable, esi
0x006C19D4  mov     g_WorldLodLevel, edi
0x006C19DA  mov     g_CarLodLevel, edi
0x006C19E0  mov     g_RainEnable, esi
0x006C19E6  mov     g_MotionBlurEnable, esi
0x006C19EC  mov     g_X360EffectsEnable, esi
0x006C19F2  mov     dword_9017E4, esi; g_LightGlowEnable
0x006C19F8  mov     dword_9017F0, edi
0x006C19FE  mov     g_OverBrightEnable, edi
0x006C1A04  mov     dword_901804, edi
0x006C1A0A  mov     dword_90180C, esi
0x006C1A10  mov     g_FSAALevel, edi
0x006C1A16  mov     g_TextureFiltering, edi
0x006C1A1C  mov     dword_9017D8, edi
0x006C1A22  mov     dword_901824, edi
0x006C1A28  mov     dword_901828, edi
0x006C1A2E  mov     dword_901830, edi
0x006C1A34  mov     ebx, 2
0x006C1A39  jle     short loc_6C1A4D
0x006C1A3B  mov     dword_9017C0, ebx
0x006C1A41  mov     dword_9017C4, esi
0x006C1A47  mov     g_WorldLodLevel, esi
0x006C1A4D  cmp     eax, ebx
0x006C1A4F  jle     short loc_6C1A87
0x006C1A51  mov     g_CarEnvironmentMapEnable, esi
0x006C1A57  mov     g_TextureFiltering, esi
0x006C1A5D  mov     g_RoadReflectionEnable, esi
0x006C1A63  mov     dword_9017D8, esi
0x006C1A69  mov     g_ParticleSystemEnable, esi
0x006C1A6F  mov     g_WorldLodLevel, ebx
0x006C1A75  mov     dword_90180C, esi
0x006C1A7B  mov     dword_9017F0, esi
0x006C1A81  mov     dword_9017E4, esi; g_LightGlowEnable
0x006C1A87  cmp     eax, 3
0x006C1A8A  jle     short loc_6C1AE4
0x006C1A8C  mov     eax, GPUDevice
0x006C1A91  mov     ecx, GPUVendor
0x006C1A97  push    eax
0x006C1A98  push    ecx
0x006C1A99  mov     g_RainEnable, esi
0x006C1A9F  mov     dword_901800, esi
0x006C1AA5  mov     g_CarEnvironmentMapUpdateData, esi
0x006C1AAB  call    sub_6CB190
0x006C1AB0  fstp    st
0x006C1AB2  mov     eax, g_PerformanceLevel
0x006C1AB7  add     esp, 8
0x006C1ABA  mov     g_RacingResolution, esi
0x006C1AC0  mov     g_FSAALevel, esi
0x006C1AC6  mov     dword_901828, esi
0x006C1ACC  mov     dword_901830, esi
0x006C1AD2  mov     g_OverBrightEnable, esi
0x006C1AD8  mov     g_X360EffectsEnable, esi
0x006C1ADE  mov     g_MotionBlurEnable, esi
0x006C1AE4  cmp     eax, 4
0x006C1AE7  jl      short loc_6C1AFB
0x006C1AE9  mov     g_CarEnvironmentMapUpdateData, ebx
0x006C1AEF  mov     g_RoadReflectionEnable, ebx
0x006C1AF5  mov     g_CarLodLevel, esi
0x006C1AFB  cmp     dword_9018B0, edi
0x006C1B01  jz      short loc_6C1B09
0x006C1B03  mov     dword_901828, esi
0x006C1B09  lea     edx, [esp+4Ch+Buffer]
0x006C1B0D  push    edx; lpBuffer
0x006C1B0E  mov     dword_901800, edi
0x006C1B14  mov     dword_9017F0, edi
0x006C1B1A  mov     dword_901804, edi
0x006C1B20  mov     [esp+50h+Buffer.dwLength], 40h ; '@'
0x006C1B28  call    ds:GlobalMemoryStatusEx
0x006C1B2E  cmp     dword ptr [esp+4Ch+Buffer.ullTotalPhys+4], edi
0x006C1B32  ja      short loc_6C1B4C
0x006C1B34  jb      short loc_6C1B40
0x006C1B36  cmp     dword ptr [esp+4Ch+Buffer.ullTotalPhys], 19000000h
0x006C1B3E  jnb     short loc_6C1B4C
0x006C1B40  mov     g_WorldLodLevel, edi
0x006C1B46  mov     g_CarLodLevel, edi
0x006C1B4C  pop     edi
0x006C1B4D  pop     esi
0x006C1B4E  pop     ebx
0x006C1B4F  add     esp, 40h
0x006C1B52  retn

==========================================================================================
FUNCTION 0x6C1C00 sub_6C1C00  (end 0x6C1F02)
==========================================================================================
0x006C1C00  sub     esp, 0Ch
0x006C1C03  lea     eax, [esp+0Ch+hKey]
0x006C1C06  push    eax; phkResult
0x006C1C07  push    20019h; samDesired
0x006C1C0C  push    0; ulOptions
0x006C1C0E  push    offset SubKey; "Software\\EA Games\\Need for Speed Most"...
0x006C1C13  push    80000002h; hKey
0x006C1C18  call    ds:RegOpenKeyExA
0x006C1C1E  test    eax, eax
0x006C1C20  jnz     loc_6C1EFE
0x006C1C26  push    esi
0x006C1C27  mov     esi, ds:RegQueryValueExA
0x006C1C2D  push    edi
0x006C1C2E  lea     ecx, [esp+14h+cbData]
0x006C1C32  push    ecx; lpcbData
0x006C1C33  push    offset VERSION; lpData
0x006C1C38  lea     edx, [esp+1Ch+Type]
0x006C1C3C  push    edx; lpType
0x006C1C3D  push    eax; lpReserved
0x006C1C3E  mov     eax, [esp+24h+hKey]
0x006C1C42  push    offset ValueName; "VERSION"
0x006C1C47  mov     edi, 4
0x006C1C4C  push    eax; hKey
0x006C1C4D  mov     [esp+2Ch+Type], edi
0x006C1C51  mov     [esp+2Ch+cbData], edi
0x006C1C55  call    esi ; RegQueryValueExA
0x006C1C57  mov     eax, [esp+14h+hKey]
0x006C1C5B  lea     ecx, [esp+14h+Type]
0x006C1C5F  push    ecx; lpcbData
0x006C1C60  push    offset SIZE; lpData
0x006C1C65  lea     edx, [esp+1Ch+cbData]
0x006C1C69  push    edx; lpType
0x006C1C6A  push    0; lpReserved
0x006C1C6C  push    offset aSize_0; "SIZE"
0x006C1C71  push    eax; hKey
0x006C1C72  mov     [esp+2Ch+cbData], edi
0x006C1C76  mov     [esp+2Ch+Type], edi
0x006C1C7A  call    esi ; RegQueryValueExA
0x006C1C7C  mov     eax, [esp+14h+hKey]
0x006C1C80  lea     ecx, [esp+14h+Type]
0x006C1C84  push    ecx; lpcbData
0x006C1C85  push    offset g_CarEnvironmentMapEnable; lpData
0x006C1C8A  lea     edx, [esp+1Ch+cbData]
0x006C1C8E  push    edx; lpType
0x006C1C8F  push    0; lpReserved
0x006C1C91  push    offset aG_carenvironme; "g_CarEnvironmentMapEnable"
0x006C1C96  push    eax; hKey
0x006C1C97  mov     [esp+2Ch+cbData], edi
0x006C1C9B  mov     [esp+2Ch+Type], edi
0x006C1C9F  call    esi ; RegQueryValueExA
0x006C1CA1  mov     eax, [esp+14h+hKey]
0x006C1CA5  lea     ecx, [esp+14h+Type]
0x006C1CA9  push    ecx; lpcbData
0x006C1CAA  push    offset g_CarEnvironmentMapUpdateData; lpData
0x006C1CAF  lea     edx, [esp+1Ch+cbData]
0x006C1CB3  push    edx; lpType
0x006C1CB4  push    0; lpReserved
0x006C1CB6  push    offset aG_carenviron_0; "g_CarEnvironmentMapUpdateData"
0x006C1CBB  push    eax; hKey
0x006C1CBC  mov     [esp+2Ch+cbData], edi
0x006C1CC0  mov     [esp+2Ch+Type], edi
0x006C1CC4  call    esi ; RegQueryValueExA
0x006C1CC6  mov     eax, [esp+14h+hKey]
0x006C1CCA  lea     ecx, [esp+14h+Type]
0x006C1CCE  push    ecx; lpcbData
0x006C1CCF  push    offset g_RoadReflectionEnable; lpData
0x006C1CD4  lea     edx, [esp+1Ch+cbData]
0x006C1CD8  push    edx; lpType
0x006C1CD9  push    0; lpReserved
0x006C1CDB  push    offset aG_roadreflecti; "g_RoadReflectionEnable"
0x006C1CE0  push    eax; hKey
0x006C1CE1  mov     [esp+2Ch+cbData], edi
0x006C1CE5  mov     [esp+2Ch+Type], edi
0x006C1CE9  call    esi ; RegQueryValueExA
0x006C1CEB  mov     eax, [esp+14h+hKey]
0x006C1CEF  lea     ecx, [esp+14h+Type]
0x006C1CF3  push    ecx; lpcbData
0x006C1CF4  push    offset g_MotionBlurEnable; lpData
0x006C1CF9  lea     edx, [esp+1Ch+cbData]
0x006C1CFD  push    edx; lpType
0x006C1CFE  push    0; lpReserved
0x006C1D00  push    offset aG_motionbluren; "g_MotionBlurEnable"
0x006C1D05  push    eax; hKey
0x006C1D06  mov     [esp+2Ch+cbData], edi
0x006C1D0A  mov     [esp+2Ch+Type], edi
0x006C1D0E  call    esi ; RegQueryValueExA
0x006C1D10  lea     ecx, [esp+14h+Type]
0x006C1D14  push    ecx; lpcbData
0x006C1D15  mov     [esp+18h+cbData], edi
0x006C1D19  mov     [esp+18h+Type], edi
0x006C1D1D  push    offset g_ParticleSystemEnable; lpData
0x006C1D22  mov     eax, [esp+1Ch+hKey]
0x006C1D26  lea     edx, [esp+1Ch+cbData]
0x006C1D2A  push    edx; lpType
0x006C1D2B  push    0; lpReserved
0x006C1D2D  push    offset aG_particlesyst; "g_ParticleSystemEnable"
0x006C1D32  push    eax; hKey
0x006C1D33  call    esi ; RegQueryValueExA
0x006C1D35  mov     eax, [esp+14h+hKey]
0x006C1D39  lea     ecx, [esp+14h+Type]
0x006C1D3D  push    ecx; lpcbData
0x006C1D3E  push    offset g_WorldLodLevel; lpData
0x006C1D43  lea     edx, [esp+1Ch+cbData]
0x006C1D47  push    edx; lpType
0x006C1D48  push    0; lpReserved
0x006C1D4A  push    offset aG_worldlodleve; "g_WorldLodLevel"
0x006C1D4F  push    eax; hKey
0x006C1D50  mov     [esp+2Ch+cbData], edi
0x006C1D54  mov     [esp+2Ch+Type], edi
0x006C1D58  call    esi ; RegQueryValueExA
0x006C1D5A  mov     eax, [esp+14h+hKey]
0x006C1D5E  lea     ecx, [esp+14h+Type]
0x006C1D62  push    ecx; lpcbData
0x006C1D63  push    offset g_CarLodLevel; lpData
0x006C1D68  lea     edx, [esp+1Ch+cbData]
0x006C1D6C  push    edx; lpType
0x006C1D6D  push    0; lpReserved
0x006C1D6F  push    offset aG_carlodlevel; "g_CarLodLevel"
0x006C1D74  push    eax; hKey
0x006C1D75  mov     [esp+2Ch+cbData], edi
0x006C1D79  mov     [esp+2Ch+Type], edi
0x006C1D7D  call    esi ; RegQueryValueExA
0x006C1D7F  mov     eax, [esp+14h+hKey]
0x006C1D83  lea     ecx, [esp+14h+Type]
0x006C1D87  push    ecx; lpcbData
0x006C1D88  push    offset g_OverBrightEnable; lpData
0x006C1D8D  lea     edx, [esp+1Ch+cbData]
0x006C1D91  push    edx; lpType
0x006C1D92  push    0; lpReserved
0x006C1D94  push    offset aG_overbrighten; "g_OverBrightEnable"
0x006C1D99  push    eax; hKey
0x006C1D9A  mov     [esp+2Ch+cbData], edi
0x006C1D9E  mov     [esp+2Ch+Type], edi
0x006C1DA2  call    esi ; RegQueryValueExA
0x006C1DA4  mov     eax, [esp+14h+hKey]
0x006C1DA8  lea     ecx, [esp+14h+Type]
0x006C1DAC  push    ecx; lpcbData
0x006C1DAD  push    offset g_FSAALevel; lpData
0x006C1DB2  lea     edx, [esp+1Ch+cbData]
0x006C1DB6  push    edx; lpType
0x006C1DB7  push    0; lpReserved
0x006C1DB9  push    offset aG_fsaalevel; "g_FSAALevel"
0x006C1DBE  push    eax; hKey
0x006C1DBF  mov     [esp+2Ch+cbData], edi
0x006C1DC3  mov     [esp+2Ch+Type], edi
0x006C1DC7  call    esi ; RegQueryValueExA
0x006C1DC9  mov     eax, [esp+14h+hKey]
0x006C1DCD  lea     ecx, [esp+14h+Type]
0x006C1DD1  push    ecx; lpcbData
0x006C1DD2  push    offset g_RainEnable; lpData
0x006C1DD7  lea     edx, [esp+1Ch+cbData]
0x006C1DDB  push    edx; lpType
0x006C1DDC  push    0; lpReserved
0x006C1DDE  push    offset aG_rainenable; "g_RainEnable"
0x006C1DE3  push    eax; hKey
0x006C1DE4  mov     [esp+2Ch+cbData], edi
0x006C1DE8  mov     [esp+2Ch+Type], edi
0x006C1DEC  call    esi ; RegQueryValueExA
0x006C1DEE  mov     eax, [esp+14h+hKey]
0x006C1DF2  lea     ecx, [esp+14h+Type]
0x006C1DF6  push    ecx; lpcbData
0x006C1DF7  push    offset g_TextureFiltering; lpData
0x006C1DFC  lea     edx, [esp+1Ch+cbData]
0x006C1E00  push    edx; lpType
0x006C1E01  push    0; lpReserved
0x006C1E03  push    offset aG_texturefilte; "g_TextureFiltering"
0x006C1E08  push    eax; hKey
0x006C1E09  mov     [esp+2Ch+cbData], edi
0x006C1E0D  mov     [esp+2Ch+Type], edi
0x006C1E11  call    esi ; RegQueryValueExA
0x006C1E13  mov     [esp+14h+cbData], edi
0x006C1E17  mov     [esp+14h+Type], edi
0x006C1E1B  mov     eax, [esp+14h+hKey]
0x006C1E1F  lea     ecx, [esp+14h+Type]
0x006C1E23  push    ecx; lpcbData
0x006C1E24  push    offset g_RacingResolution; lpData
0x006C1E29  lea     edx, [esp+1Ch+cbData]
0x006C1E2D  push    edx; lpType
0x006C1E2E  push    0; lpReserved
0x006C1E30  push    offset aG_racingresolu; "g_RacingResolution"
0x006C1E35  push    eax; hKey
0x006C1E36  call    esi ; RegQueryValueExA
0x006C1E38  mov     eax, [esp+14h+hKey]
0x006C1E3C  lea     ecx, [esp+14h+Type]
0x006C1E40  push    ecx; lpcbData
0x006C1E41  push    offset g_PerformanceLevel; lpData
0x006C1E46  lea     edx, [esp+1Ch+cbData]
0x006C1E4A  push    edx; lpType
0x006C1E4B  push    0; lpReserved
0x006C1E4D  push    offset aG_performancel; "g_PerformanceLevel"
0x006C1E52  push    eax; hKey
0x006C1E53  mov     [esp+2Ch+cbData], edi
0x006C1E57  mov     [esp+2Ch+Type], edi
0x006C1E5B  call    esi ; RegQueryValueExA
0x006C1E5D  mov     eax, [esp+14h+hKey]
0x006C1E61  lea     ecx, [esp+14h+Type]
0x006C1E65  push    ecx; lpcbData
0x006C1E66  push    offset dword_901824; lpData
0x006C1E6B  lea     edx, [esp+1Ch+cbData]
0x006C1E6F  push    edx; lpType
0x006C1E70  push    0; lpReserved
0x006C1E72  push    offset aG_vsyncon; "g_VSyncOn"
0x006C1E77  push    eax; hKey
0x006C1E78  mov     [esp+2Ch+cbData], edi
0x006C1E7C  mov     [esp+2Ch+Type], edi
0x006C1E80  call    esi ; RegQueryValueExA
0x006C1E82  mov     eax, [esp+14h+hKey]
0x006C1E86  lea     ecx, [esp+14h+Type]
0x006C1E8A  push    ecx; lpcbData
0x006C1E8B  push    offset dword_901830; lpData
0x006C1E90  lea     edx, [esp+1Ch+cbData]
0x006C1E94  push    edx; lpType
0x006C1E95  push    0; lpReserved
0x006C1E97  push    offset aG_shadowdetail; "g_ShadowDetail"
0x006C1E9C  push    eax; hKey
0x006C1E9D  mov     [esp+2Ch+cbData], edi
0x006C1EA1  mov     [esp+2Ch+Type], edi
0x006C1EA5  call    esi ; RegQueryValueExA
0x006C1EA7  mov     eax, [esp+14h+hKey]
0x006C1EAB  lea     ecx, [esp+14h+Type]
0x006C1EAF  push    ecx; lpcbData
0x006C1EB0  push    offset dword_901828; lpData
0x006C1EB5  lea     edx, [esp+1Ch+cbData]
0x006C1EB9  push    edx; lpType
0x006C1EBA  push    0; lpReserved
0x006C1EBC  push    offset aG_visualtreatm; "g_VisualTreatment"
0x006C1EC1  push    eax; hKey
0x006C1EC2  mov     [esp+2Ch+cbData], edi
0x006C1EC6  mov     [esp+2Ch+Type], edi
0x006C1ECA  call    esi ; RegQueryValueExA
0x006C1ECC  mov     eax, [esp+14h+hKey]
0x006C1ED0  lea     ecx, [esp+14h+Type]
0x006C1ED4  push    ecx; lpcbData
0x006C1ED5  push    offset dword_982C78; lpData
0x006C1EDA  lea     edx, [esp+1Ch+cbData]
0x006C1EDE  push    edx; lpType
0x006C1EDF  push    0; lpReserved
0x006C1EE1  push    offset aVtmode; "VTMode"
0x006C1EE6  push    eax; hKey
0x006C1EE7  mov     [esp+2Ch+cbData], edi
0x006C1EEB  mov     [esp+2Ch+Type], edi
0x006C1EEF  call    esi ; RegQueryValueExA
0x006C1EF1  mov     ecx, [esp+14h+hKey]
0x006C1EF5  push    ecx; hKey
0x006C1EF6  call    ds:RegCloseKey
0x006C1EFC  pop     edi
0x006C1EFD  pop     esi
0x006C1EFE  add     esp, 0Ch
0x006C1F01  retn

==========================================================================================
FUNCTION 0x6C1F10 sub_6C1F10  (end 0x6C2132)
==========================================================================================
0x006C1F10  push    ecx
0x006C1F11  push    esi
0x006C1F12  mov     esi, ds:RegOpenKeyExA
0x006C1F18  lea     eax, [esp+8+phkResult]
0x006C1F1C  push    eax; phkResult
0x006C1F1D  push    0F003Fh; samDesired
0x006C1F22  push    0; ulOptions
0x006C1F24  push    offset SubKey; "Software\\EA Games\\Need for Speed Most"...
0x006C1F29  push    80000002h; hKey
0x006C1F2E  call    esi ; RegOpenKeyExA
0x006C1F30  test    eax, eax
0x006C1F32  jz      short loc_6C1F69
0x006C1F34  lea     ecx, [esp+8+phkResult]
0x006C1F38  push    ecx; phkResult
0x006C1F39  push    offset SubKey; "Software\\EA Games\\Need for Speed Most"...
0x006C1F3E  push    80000002h; hKey
0x006C1F43  call    ds:RegCreateKeyA
0x006C1F49  lea     edx, [esp+8+phkResult]
0x006C1F4D  push    edx; phkResult
0x006C1F4E  push    0F003Fh; samDesired
0x006C1F53  push    0; ulOptions
0x006C1F55  push    offset SubKey; "Software\\EA Games\\Need for Speed Most"...
0x006C1F5A  push    80000002h; hKey
0x006C1F5F  call    esi ; RegOpenKeyExA
0x006C1F61  test    eax, eax
0x006C1F63  jnz     loc_6C212F
0x006C1F69  mov     eax, [esp+8+phkResult]
0x006C1F6D  mov     esi, ds:RegSetValueExA
0x006C1F73  push    4; cbData
0x006C1F75  push    offset VERSION; lpData
0x006C1F7A  push    4; dwType
0x006C1F7C  push    0; Reserved
0x006C1F7E  push    offset ValueName; "VERSION"
0x006C1F83  push    eax; hKey
0x006C1F84  call    esi ; RegSetValueExA
0x006C1F86  mov     ecx, [esp+8+phkResult]
0x006C1F8A  push    4; cbData
0x006C1F8C  push    offset SIZE; lpData
0x006C1F91  push    4; dwType
0x006C1F93  push    0; Reserved
0x006C1F95  push    offset aSize_0; "SIZE"
0x006C1F9A  push    ecx; hKey
0x006C1F9B  call    esi ; RegSetValueExA
0x006C1F9D  mov     edx, [esp+8+phkResult]
0x006C1FA1  push    4; cbData
0x006C1FA3  push    offset g_CarEnvironmentMapEnable; lpData
0x006C1FA8  push    4; dwType
0x006C1FAA  push    0; Reserved
0x006C1FAC  push    offset aG_carenvironme; "g_CarEnvironmentMapEnable"
0x006C1FB1  push    edx; hKey
0x006C1FB2  call    esi ; RegSetValueExA
0x006C1FB4  mov     eax, [esp+8+phkResult]
0x006C1FB8  push    4; cbData
0x006C1FBA  push    offset g_CarEnvironmentMapUpdateData; lpData
0x006C1FBF  push    4; dwType
0x006C1FC1  push    0; Reserved
0x006C1FC3  push    offset aG_carenviron_0; "g_CarEnvironmentMapUpdateData"
0x006C1FC8  push    eax; hKey
0x006C1FC9  call    esi ; RegSetValueExA
0x006C1FCB  mov     ecx, [esp+8+phkResult]
0x006C1FCF  push    4; cbData
0x006C1FD1  push    offset g_RoadReflectionEnable; lpData
0x006C1FD6  push    4; dwType
0x006C1FD8  push    0; Reserved
0x006C1FDA  push    offset aG_roadreflecti; "g_RoadReflectionEnable"
0x006C1FDF  push    ecx; hKey
0x006C1FE0  call    esi ; RegSetValueExA
0x006C1FE2  mov     edx, [esp+8+phkResult]
0x006C1FE6  push    4; cbData
0x006C1FE8  push    offset g_MotionBlurEnable; lpData
0x006C1FED  push    4; dwType
0x006C1FEF  push    0; Reserved
0x006C1FF1  push    offset aG_motionbluren; "g_MotionBlurEnable"
0x006C1FF6  push    edx; hKey
0x006C1FF7  call    esi ; RegSetValueExA
0x006C1FF9  mov     eax, [esp+8+phkResult]
0x006C1FFD  push    4; cbData
0x006C1FFF  push    offset g_ParticleSystemEnable; lpData
0x006C2004  push    4; dwType
0x006C2006  push    0; Reserved
0x006C2008  push    offset aG_particlesyst; "g_ParticleSystemEnable"
0x006C200D  push    eax; hKey
0x006C200E  call    esi ; RegSetValueExA
0x006C2010  mov     ecx, [esp+8+phkResult]
0x006C2014  push    4; cbData
0x006C2016  push    offset g_WorldLodLevel; lpData
0x006C201B  push    4; dwType
0x006C201D  push    0; Reserved
0x006C201F  push    offset aG_worldlodleve; "g_WorldLodLevel"
0x006C2024  push    ecx; hKey
0x006C2025  call    esi ; RegSetValueExA
0x006C2027  mov     edx, [esp+8+phkResult]
0x006C202B  push    4; cbData
0x006C202D  push    offset g_CarLodLevel; lpData
0x006C2032  push    4; dwType
0x006C2034  push    0; Reserved
0x006C2036  push    offset aG_carlodlevel; "g_CarLodLevel"
0x006C203B  push    edx; hKey
0x006C203C  call    esi ; RegSetValueExA
0x006C203E  mov     eax, [esp+8+phkResult]
0x006C2042  push    4; cbData
0x006C2044  push    offset g_OverBrightEnable; lpData
0x006C2049  push    4; dwType
0x006C204B  push    0; Reserved
0x006C204D  push    offset aG_overbrighten; "g_OverBrightEnable"
0x006C2052  push    eax; hKey
0x006C2053  call    esi ; RegSetValueExA
0x006C2055  mov     ecx, [esp+8+phkResult]
0x006C2059  push    4; cbData
0x006C205B  push    offset g_FSAALevel; lpData
0x006C2060  push    4; dwType
0x006C2062  push    0; Reserved
0x006C2064  push    offset aG_fsaalevel; "g_FSAALevel"
0x006C2069  push    ecx; hKey
0x006C206A  call    esi ; RegSetValueExA
0x006C206C  mov     edx, [esp+8+phkResult]
0x006C2070  push    4; cbData
0x006C2072  push    offset g_RainEnable; lpData
0x006C2077  push    4; dwType
0x006C2079  push    0; Reserved
0x006C207B  push    offset aG_rainenable; "g_RainEnable"
0x006C2080  push    edx; hKey
0x006C2081  call    esi ; RegSetValueExA
0x006C2083  mov     eax, [esp+8+phkResult]
0x006C2087  push    4; cbData
0x006C2089  push    offset g_TextureFiltering; lpData
0x006C208E  push    4; dwType
0x006C2090  push    0; Reserved
0x006C2092  push    offset aG_texturefilte; "g_TextureFiltering"
0x006C2097  push    eax; hKey
0x006C2098  call    esi ; RegSetValueExA
0x006C209A  mov     ecx, [esp+8+phkResult]
0x006C209E  push    4; cbData
0x006C20A0  push    offset g_RacingResolution; lpData
0x006C20A5  push    4; dwType
0x006C20A7  push    0; Reserved
0x006C20A9  push    offset aG_racingresolu; "g_RacingResolution"
0x006C20AE  push    ecx; hKey
0x006C20AF  call    esi ; RegSetValueExA
0x006C20B1  mov     edx, [esp+8+phkResult]
0x006C20B5  push    4; cbData
0x006C20B7  push    offset dword_901820; lpData
0x006C20BC  push    4; dwType
0x006C20BE  push    0; Reserved
0x006C20C0  push    offset aFirsttime; "FirstTime"
0x006C20C5  push    edx; hKey
0x006C20C6  call    esi ; RegSetValueExA
0x006C20C8  mov     eax, [esp+8+phkResult]
0x006C20CC  push    4; cbData
0x006C20CE  push    offset g_PerformanceLevel; lpData
0x006C20D3  push    4; dwType
0x006C20D5  push    0; Reserved
0x006C20D7  push    offset aG_performancel; "g_PerformanceLevel"
0x006C20DC  push    eax; hKey
0x006C20DD  call    esi ; RegSetValueExA
0x006C20DF  mov     ecx, [esp+8+phkResult]
0x006C20E3  push    4; cbData
0x006C20E5  push    offset dword_901824; lpData
0x006C20EA  push    4; dwType
0x006C20EC  push    0; Reserved
0x006C20EE  push    offset aG_vsyncon; "g_VSyncOn"
0x006C20F3  push    ecx; hKey
0x006C20F4  call    esi ; RegSetValueExA
0x006C20F6  mov     edx, [esp+8+phkResult]
0x006C20FA  push    4; cbData
0x006C20FC  push    offset dword_901830; lpData
0x006C2101  push    4; dwType
0x006C2103  push    0; Reserved
0x006C2105  push    offset aG_shadowdetail; "g_ShadowDetail"
0x006C210A  push    edx; hKey
0x006C210B  call    esi ; RegSetValueExA
0x006C210D  mov     eax, [esp+8+phkResult]
0x006C2111  push    4; cbData
0x006C2113  push    offset dword_901828; lpData
0x006C2118  push    4; dwType
0x006C211A  push    0; Reserved
0x006C211C  push    offset aG_visualtreatm; "g_VisualTreatment"
0x006C2121  push    eax; hKey
0x006C2122  call    esi ; RegSetValueExA
0x006C2124  mov     ecx, [esp+8+phkResult]
0x006C2128  push    ecx; hKey
0x006C2129  call    ds:RegCloseKey
0x006C212F  pop     esi
0x006C2130  pop     ecx
0x006C2131  retn

==========================================================================================
FUNCTION 0x6C2E60 sub_6C2E60  (end 0x6C3351)
==========================================================================================
0x006C2E60  push    ecx
0x006C2E61  mov     eax, dword_982BE4
0x006C2E66  mov     ecx, dword_982BE8
0x006C2E6C  push    ebx
0x006C2E6D  mov     dword_8F9ADC, eax
0x006C2E72  mov     eax, g_MotionBlurEnable
0x006C2E77  xor     ebx, ebx
0x006C2E79  cmp     eax, ebx
0x006C2E7B  mov     dword_8F9AE0, ecx
0x006C2E81  jnz     short loc_6C2EAF
0x006C2E83  cmp     dword_901800, ebx
0x006C2E89  jnz     short loc_6C2EAF
0x006C2E8B  cmp     dword_9017F0, ebx
0x006C2E91  jnz     short loc_6C2EAF
0x006C2E93  cmp     g_RainEnable, ebx
0x006C2E99  jnz     short loc_6C2EAF
0x006C2E9B  cmp     dword_901828, ebx
0x006C2EA1  jnz     short loc_6C2EAF
0x006C2EA3  cmp     dword_9017D0, ebx
0x006C2EA9  jnz     short loc_6C2EAF
0x006C2EAB  xor     al, al
0x006C2EAD  jmp     short loc_6C2EB1
0x006C2EAF  mov     al, 1
0x006C2EB1  cmp     g_X360EffectsEnable, ebx
0x006C2EB7  jnz     short loc_6C2EC5
0x006C2EB9  cmp     g_OverBrightEnable, ebx
0x006C2EBF  mov     [esp+8+var_1], bl
0x006C2EC3  jz      short loc_6C2ECA
0x006C2EC5  mov     [esp+8+var_1], 1
0x006C2ECA  cmp     al, bl
0x006C2ECC  push    ebp
0x006C2ECD  push    esi
0x006C2ECE  push    edi
0x006C2ECF  mov     dword_93DE60, ebx
0x006C2ED5  mov     dword_93DEF8, ebx
0x006C2EDB  mov     dword_93DE64, ebx
0x006C2EE1  mov     dword_93DEFC, ebx
0x006C2EE7  jz      loc_6C2FE4
0x006C2EED  xor     esi, esi
0x006C2EEF  nop
0x006C2EF0  mov     ecx, dword_8F9AD8
0x006C2EF6  mov     eax, dword_982BDC
0x006C2EFB  mov     edx, [eax]
0x006C2EFD  push    ebx
0x006C2EFE  lea     edi, dword_93DEF8[esi]
0x006C2F04  push    edi
0x006C2F05  push    ebx
0x006C2F06  push    ecx
0x006C2F07  mov     ecx, dword_8F9AE0
0x006C2F0D  push    1
0x006C2F0F  push    1
0x006C2F11  push    ecx
0x006C2F12  mov     ecx, dword_8F9ADC
0x006C2F18  push    ecx
0x006C2F19  push    eax
0x006C2F1A  call    dword ptr [edx+5Ch]
0x006C2F1D  cmp     eax, ebx
0x006C2F1F  jnz     short loc_6C2F7A
0x006C2F21  mov     edi, [edi]
0x006C2F23  mov     edx, [edi]
0x006C2F25  lea     ebp, dword_93DE60[esi]
0x006C2F2B  push    ebp
0x006C2F2C  push    ebx
0x006C2F2D  push    edi
0x006C2F2E  call    dword ptr [edx+48h]
0x006C2F31  cmp     eax, ebx
0x006C2F33  jnz     short loc_6C2F7A
0x006C2F35  mov     edx, [ebp+0]
0x006C2F38  mov     eax, dword_982BDC
0x006C2F3D  mov     ecx, [eax]
0x006C2F3F  push    edx
0x006C2F40  push    ebx
0x006C2F41  push    eax
0x006C2F42  call    dword ptr [ecx+94h]
0x006C2F48  mov     eax, dword_982BDC
0x006C2F4D  mov     ecx, [eax]
0x006C2F4F  push    ebx
0x006C2F50  push    eax
0x006C2F51  call    dword ptr [ecx+9Ch]
0x006C2F57  mov     eax, dword_982BDC
0x006C2F5C  mov     edx, [eax]
0x006C2F5E  push    ebx
0x006C2F5F  push    ebx
0x006C2F60  push    ebx
0x006C2F61  push    1
0x006C2F63  push    ebx
0x006C2F64  push    ebx
0x006C2F65  push    eax
0x006C2F66  call    dword ptr [edx+0ACh]
0x006C2F6C  add     esi, 4
0x006C2F6F  cmp     esi, 8
0x006C2F72  jl      loc_6C2EF0
0x006C2F78  jmp     short loc_6C2FE4
0x006C2F7A  xor     esi, esi
0x006C2F7C  lea     esp, [esp+0]
0x006C2F80  mov     eax, dword_93DE60[esi]
0x006C2F86  cmp     eax, ebx
0x006C2F88  jz      short loc_6C2F96
0x006C2F8A  mov     ecx, [eax]
0x006C2F8C  push    eax
0x006C2F8D  call    dword ptr [ecx+8]
0x006C2F90  mov     dword_93DE60[esi], ebx
0x006C2F96  mov     eax, dword_93DEF8[esi]
0x006C2F9C  cmp     eax, ebx
0x006C2F9E  jz      short loc_6C2FAC
0x006C2FA0  mov     edx, [eax]
0x006C2FA2  push    eax
0x006C2FA3  call    dword ptr [edx+8]
0x006C2FA6  mov     dword_93DEF8[esi], ebx
0x006C2FAC  add     esi, 4
0x006C2FAF  cmp     esi, 8
0x006C2FB2  jl      short loc_6C2F80
0x006C2FB4  mov     g_RainEnable, ebx
0x006C2FBA  mov     dword_9017F0, ebx
0x006C2FC0  mov     dword_901800, ebx
0x006C2FC6  mov     g_MotionBlurEnable, ebx
0x006C2FCC  mov     dword_901898, ebx
0x006C2FD2  mov     dword_901878, ebx
0x006C2FD8  mov     dword_901888, ebx
0x006C2FDE  mov     dword_901864, ebx
0x006C2FE4  cmp     [esp+14h+var_1], bl
0x006C2FE8  mov     dword_93E7BC, ebx
0x006C2FEE  mov     dword_982960, ebx
0x006C2FF4  mov     dword_93E7B8, ebx
0x006C2FFA  jz      loc_6C311D
0x006C3000  mov     edx, dword_8F9AE8
0x006C3006  mov     eax, dword_982BDC
0x006C300B  mov     ecx, [eax]
0x006C300D  push    ebx
0x006C300E  push    offset dword_93E7BC
0x006C3013  push    ebx
0x006C3014  push    edx
0x006C3015  mov     edx, dword_982BE8
0x006C301B  push    1
0x006C301D  push    1
0x006C301F  push    edx
0x006C3020  mov     edx, dword_982BE4
0x006C3026  push    edx
0x006C3027  push    eax
0x006C3028  call    dword ptr [ecx+5Ch]
0x006C302B  cmp     eax, ebx
0x006C302D  jnz     loc_6C30C6
0x006C3033  mov     eax, dword_93E7BC
0x006C3038  mov     ecx, [eax]
0x006C303A  push    offset dword_982960
0x006C303F  push    ebx
0x006C3040  push    eax
0x006C3041  call    dword ptr [ecx+48h]
0x006C3044  mov     esi, eax
0x006C3046  cmp     esi, ebx
0x006C3048  jnz     short loc_6C3084
0x006C304A  mov     ecx, dword_982960
0x006C3050  mov     eax, dword_982BDC
0x006C3055  mov     edx, [eax]
0x006C3057  push    ecx
0x006C3058  push    ebx
0x006C3059  push    eax
0x006C305A  call    dword ptr [edx+94h]
0x006C3060  mov     eax, dword_982BDC
0x006C3065  mov     edx, [eax]
0x006C3067  push    ebx
0x006C3068  push    eax
0x006C3069  call    dword ptr [edx+9Ch]
0x006C306F  mov     eax, dword_982BDC
0x006C3074  mov     ecx, [eax]
0x006C3076  push    ebx
0x006C3077  push    ebx
0x006C3078  push    ebx
0x006C3079  push    1
0x006C307B  push    ebx
0x006C307C  push    ebx
0x006C307D  push    eax
0x006C307E  call    dword ptr [ecx+0ACh]
0x006C3084  mov     ecx, g_FSAALevel
0x006C308A  cmp     ecx, ebx
0x006C308C  jle     short loc_6C30BC
0x006C308E  mov     eax, dword_982BDC
0x006C3093  mov     edx, [eax]
0x006C3095  push    ebx
0x006C3096  push    offset dword_93E7B8
0x006C309B  push    ebx
0x006C309C  push    ecx
0x006C309D  mov     ecx, dword_8F9AE8
0x006C30A3  push    1
0x006C30A5  push    ecx
0x006C30A6  mov     ecx, dword_982BE8
0x006C30AC  push    ecx
0x006C30AD  mov     ecx, dword_982BE4
0x006C30B3  push    ecx
0x006C30B4  push    eax
0x006C30B5  call    dword ptr [edx+70h]
0x006C30B8  mov     esi, eax
0x006C30BA  jmp     short loc_6C30C2
0x006C30BC  mov     dword_93E7B8, ebx
0x006C30C2  cmp     esi, ebx
0x006C30C4  jz      short loc_6C311D
0x006C30C6  mov     eax, dword_982960
0x006C30CB  cmp     eax, ebx
0x006C30CD  jz      short loc_6C30DB
0x006C30CF  mov     edx, [eax]
0x006C30D1  push    eax
0x006C30D2  call    dword ptr [edx+8]
0x006C30D5  mov     dword_982960, ebx
0x006C30DB  mov     eax, dword_93E7BC
0x006C30E0  cmp     eax, ebx
0x006C30E2  jz      short loc_6C30F0
0x006C30E4  mov     ecx, [eax]
0x006C30E6  push    eax
0x006C30E7  call    dword ptr [ecx+8]
0x006C30EA  mov     dword_93E7BC, ebx
0x006C30F0  mov     eax, dword_93E7B8
0x006C30F5  cmp     eax, ebx
0x006C30F7  jz      short loc_6C3105
0x006C30F9  mov     edx, [eax]
0x006C30FB  push    eax
0x006C30FC  call    dword ptr [edx+8]
0x006C30FF  mov     dword_93E7B8, ebx
0x006C3105  mov     g_OverBrightEnable, ebx
0x006C310B  mov     g_X360EffectsEnable, ebx
0x006C3111  mov     dword_901884, ebx
0x006C3117  mov     dword_901868, ebx
0x006C311D  mov     esi, dword_982BE4
0x006C3123  mov     edi, dword_982BE8
0x006C3129  mov     eax, esi
0x006C312B  cdq
0x006C312C  and     edx, 3
0x006C312F  add     eax, edx
0x006C3131  mov     ecx, eax
0x006C3133  mov     eax, edi
0x006C3135  cdq
0x006C3136  and     edx, 3
0x006C3139  add     eax, edx
0x006C313B  sar     eax, 2
0x006C313E  mov     dword_93E81C, eax
0x006C3143  mov     dword_93E820, eax
0x006C3148  mov     dword_93E824, eax
0x006C314D  mov     eax, esi
0x006C314F  cdq
0x006C3150  and     edx, 1Fh
0x006C3153  add     eax, edx
0x006C3155  sar     eax, 5
0x006C3158  mov     dword_93CAEC, eax
0x006C315D  mov     eax, edi
0x006C315F  cdq
0x006C3160  and     edx, 1Fh
0x006C3163  sar     ecx, 2
0x006C3166  add     eax, edx
0x006C3168  sar     eax, 5
0x006C316B  mov     dword_93CAE0, ecx
0x006C3171  mov     dword_93CAE4, ecx
0x006C3177  mov     dword_93CAE8, ecx
0x006C317D  mov     dword_93E828, eax
0x006C3182  mov     dword_982990, ebx
0x006C3188  mov     dword_982964, ebx
0x006C318E  mov     dword_982994, ebx
0x006C3194  mov     dword_982968, ebx
0x006C319A  mov     dword_982998, ebx
0x006C31A0  mov     dword_98296C, ebx
0x006C31A6  mov     dword_98299C, ebx
0x006C31AC  mov     dword_982970, ebx
0x006C31B2  xor     esi, esi
0x006C31B4  jmp     short loc_6C31C0
0x006C31C0  mov     edx, dword_8F9AD8
0x006C31C6  mov     eax, dword_982BDC
0x006C31CB  mov     ecx, [eax]
0x006C31CD  push    ebx
0x006C31CE  lea     ebp, dword_982964[esi]
0x006C31D4  push    ebp
0x006C31D5  push    ebx
0x006C31D6  push    edx
0x006C31D7  mov     edx, dword_93E81C[esi]
0x006C31DD  push    1
0x006C31DF  push    1
0x006C31E1  push    edx
0x006C31E2  mov     edx, dword_93CAE0[esi]
0x006C31E8  push    edx
0x006C31E9  push    eax
0x006C31EA  call    dword ptr [ecx+5Ch]
0x006C31ED  cmp     eax, ebx
0x006C31EF  jnz     short loc_6C324A
0x006C31F1  mov     eax, [ebp+0]
0x006C31F4  mov     ecx, [eax]
0x006C31F6  lea     edi, dword_982990[esi]
0x006C31FC  push    edi
0x006C31FD  push    ebx
0x006C31FE  push    eax
0x006C31FF  call    dword ptr [ecx+48h]
0x006C3202  cmp     eax, ebx
0x006C3204  jnz     short loc_6C324A
0x006C3206  mov     ecx, [edi]
0x006C3208  mov     eax, dword_982BDC
0x006C320D  mov     edx, [eax]
0x006C320F  push    ecx
0x006C3210  push    ebx
0x006C3211  push    eax
0x006C3212  call    dword ptr [edx+94h]
0x006C3218  mov     eax, dword_982BDC
0x006C321D  mov     edx, [eax]
0x006C321F  push    ebx
0x006C3220  push    eax
0x006C3221  call    dword ptr [edx+9Ch]
0x006C3227  mov     eax, dword_982BDC
0x006C322C  mov     ecx, [eax]
0x006C322E  push    ebx
0x006C322F  push    ebx
0x006C3230  push    ebx
0x006C3231  push    1
0x006C3233  push    ebx
0x006C3234  push    ebx
0x006C3235  push    eax
0x006C3236  call    dword ptr [ecx+0ACh]
0x006C323C  add     esi, 4
0x006C323F  cmp     esi, 10h
0x006C3242  jl      loc_6C31C0
0x006C3248  jmp     short loc_6C329C
0x006C324A  xor     esi, esi
0x006C324C  lea     esp, [esp+0]
0x006C3250  mov     eax, dword_982990[esi]
0x006C3256  cmp     eax, ebx
0x006C3258  jz      short loc_6C3266
0x006C325A  mov     edx, [eax]
0x006C325C  push    eax
0x006C325D  call    dword ptr [edx+8]
0x006C3260  mov     dword_982990[esi], ebx
0x006C3266  mov     eax, dword_982964[esi]
0x006C326C  cmp     eax, ebx
0x006C326E  jz      short loc_6C327C
0x006C3270  mov     ecx, [eax]
0x006C3272  push    eax
0x006C3273  call    dword ptr [ecx+8]
0x006C3276  mov     dword_982964[esi], ebx
0x006C327C  add     esi, 4
0x006C327F  cmp     esi, 10h
0x006C3282  jl      short loc_6C3250
0x006C3284  mov     dword_9017F0, ebx
0x006C328A  mov     g_OverBrightEnable, ebx
0x006C3290  mov     dword_901878, ebx
0x006C3296  mov     dword_901884, ebx
0x006C329C  call    sub_6C2AA0
0x006C32A1  cmp     byte_988540, bl
0x006C32A7  pop     edi
0x006C32A8  pop     esi
0x006C32A9  pop     ebp
0x006C32AA  jnz     loc_6C334E
0x006C32B0  mov     ecx, dword_8F9AEC
0x006C32B6  mov     eax, dword_982BDC
0x006C32BB  mov     edx, [eax]
0x006C32BD  push    ebx
0x006C32BE  push    offset dword_98298C
0x006C32C3  push    1
0x006C32C5  push    ecx
0x006C32C6  mov     ecx, dword_8F9AF0
0x006C32CC  push    ebx
0x006C32CD  push    1
0x006C32CF  push    ecx
0x006C32D0  push    ecx
0x006C32D1  push    eax
0x006C32D2  mov     byte_988540, 1
0x006C32D9  call    dword ptr [edx+5Ch]
0x006C32DC  mov     ecx, dword_8F9AEC
0x006C32E2  mov     eax, dword_982BDC
0x006C32E7  mov     edx, [eax]
0x006C32E9  push    ebx
0x006C32EA  push    offset dword_93E870
0x006C32EF  push    1
0x006C32F1  push    ecx
0x006C32F2  mov     ecx, dword_8F9AF8
0x006C32F8  push    ebx
0x006C32F9  push    1
0x006C32FB  push    ecx
0x006C32FC  push    1
0x006C32FE  push    eax
0x006C32FF  call    dword ptr [edx+5Ch]
0x006C3302  mov     ecx, dword_8F9AFC
0x006C3308  mov     eax, dword_982BDC
0x006C330D  mov     edx, [eax]
0x006C330F  push    ebx
0x006C3310  push    offset unk_93D06C
0x006C3315  push    1
0x006C3317  push    ecx
0x006C3318  mov     ecx, dword_8F9B00
0x006C331E  push    ebx
0x006C331F  push    1
0x006C3321  push    2
0x006C3323  push    ecx
0x006C3324  push    eax
0x006C3325  call    dword ptr [edx+5Ch]
0x006C3328  mov     ecx, dword_8F9B04
0x006C332E  mov     eax, dword_982BDC
0x006C3333  mov     edx, [eax]
0x006C3335  push    ebx
0x006C3336  push    offset dword_982900
0x006C333B  push    1
0x006C333D  push    ecx
0x006C333E  mov     ecx, dword_8F9B08
0x006C3344  push    ebx
0x006C3345  push    1
0x006C3347  push    1
0x006C3349  push    ecx
0x006C334A  push    eax
0x006C334B  call    dword ptr [edx+5Ch]
0x006C334E  pop     ebx
0x006C334F  pop     ecx
0x006C3350  retn

==========================================================================================
FUNCTION 0x6DE300 sub_6DE300  (end 0x6DF8D2)
==========================================================================================
0x006DE300  push    ebp
0x006DE301  mov     ebp, esp
0x006DE303  and     esp, 0FFFFFFF0h
0x006DE306  sub     esp, 0AA4h
0x006DE30C  mov     eax, dword_9017D0
0x006DE311  push    ebx
0x006DE312  xor     ebx, ebx
0x006DE314  cmp     eax, ebx
0x006DE316  push    esi
0x006DE317  push    edi
0x006DE318  jnz     short loc_6DE324
0x006DE31A  mov     dword_982BA0, 1
0x006DE324  cmp     dword_982B34, ebx
0x006DE32A  mov     dword_982B9C, ebx
0x006DE330  jz      short loc_6DE33C
0x006DE332  mov     dword_982B9C, 800h
0x006DE33C  push    1
0x006DE33E  push    offset dword_919650
0x006DE343  call    sub_73CFE0
0x006DE348  add     esp, 8
0x006DE34B  cmp     eax, ebx
0x006DE34D  mov     dword_982BA4, eax
0x006DE352  jz      short loc_6DE385
0x006DE354  fld     flt_8F9340
0x006DE35A  fmul    WorldTimeElapsed
0x006DE360  fadd    flt_982BA8
0x006DE366  fst     flt_982BA8
0x006DE36C  fcomp   ds:_float_heat_1
0x006DE372  fnstsw  ax
0x006DE374  test    ah, 41h
0x006DE377  jnz     short loc_6DE3B4
0x006DE379  mov     flt_982BA8, 3F800000h
0x006DE383  jmp     short loc_6DE3B4
0x006DE385  fld     flt_8F9344
0x006DE38B  fmul    WorldTimeElapsed
0x006DE391  fsubr   flt_982BA8
0x006DE397  fst     flt_982BA8
0x006DE39D  fcomp   ds:_float_0
0x006DE3A3  fnstsw  ax
0x006DE3A5  test    ah, 5
0x006DE3A8  jp      short loc_6DE3B4
0x006DE3AA  mov     flt_982BA8, 0
0x006DE3B4  mov     esi, dword_901830
0x006DE3BA  xor     eax, eax
0x006DE3BC  cmp     esi, ebx
0x006DE3BE  setz    al
0x006DE3C1  mov     dword_8EC2B8, eax
0x006DE3C6  call    sub_6D6CF0
0x006DE3CB  mov     dword_982C80, ebx
0x006DE3D1  call    ePreDisplay__Fv; ePreDisplay(void)
0x006DE3D6  mov     ecx, TimeOfDayPointer
0x006DE3DC  call    sub_769360
0x006DE3E1  call    sub_6C83D0
0x006DE3E6  lea     ecx, [esp+0AB0h+var_9D0]
0x006DE3ED  push    ecx
0x006DE3EE  mov     [esp+0AB4h+var_10], ebx
0x006DE3F5  call    sub_6BFDD0
0x006DE3FA  mov     eax, g_RoadReflectionEnable
0x006DE3FF  add     esp, 4
0x006DE402  cmp     eax, ebx
0x006DE404  mov     edi, 6
0x006DE409  jz      loc_6DE64C
0x006DE40F  cmp     byte_9197A8, bl
0x006DE415  mov     eax, dword_919800
0x006DE41A  mov     dword_8FAE6C, 3
0x006DE424  jz      loc_6DF8CB
0x006DE42A  cmp     [eax+10h], ebx
0x006DE42D  jz      loc_6DF8CB
0x006DE433  push    1
0x006DE435  push    eax
0x006DE436  call    sub_6BD3B0
0x006DE43B  mov     edx, dword_919808
0x006DE441  mov     eax, [edx+290h]
0x006DE447  mov     ecx, TimeOfDayPointer
0x006DE44D  add     esp, 8
0x006DE450  push    eax
0x006DE451  call    sub_769580
0x006DE456  mov     ecx, dword_8FADF8
0x006DE45C  mov     edx, dword_8FADFC
0x006DE462  mov     eax, dword_8FADE0
0x006DE467  mov     esi, dword_8FADF0
0x006DE46D  mov     edi, dword_8FADF4
0x006DE473  mov     [esp+0AB0h+var_A8C], ecx
0x006DE477  mov     ecx, dword_8F92E0
0x006DE47D  mov     [esp+0AB0h+var_A88], edx
0x006DE481  mov     edx, dword_93DEC4
0x006DE487  mov     [esp+0AB0h+var_AA4], eax
0x006DE48B  mov     dword_8FADE0, ecx
0x006DE491  cmp     [edx+40h], bl
0x006DE494  jz      short loc_6DE49B
0x006DE496  mov     eax, ecx
0x006DE498  push    eax
0x006DE499  jmp     short loc_6DE4A2
0x006DE49B  mov     ecx, dword_8F92E0
0x006DE4A1  push    ecx; float
0x006DE4A2  push    4; int
0x006DE4A4  push    offset dword_9197A0; int
0x006DE4A9  call    StuffSkyLayer__FP5eView9SKY_LAYER; StuffSkyLayer(eView *,SKY_LAYER)
0x006DE4AE  add     esp, 0Ch
0x006DE4B1  call    sub_6E2F50
0x006DE4B6  mov     edx, [esp+0AB0h+var_A8C]
0x006DE4BA  mov     eax, [esp+0AB0h+var_A88]
0x006DE4BE  mov     ecx, [esp+0AB0h+var_AA4]
0x006DE4C2  mov     dword_8FADF0, esi
0x006DE4C8  mov     dword_8FADF4, edi
0x006DE4CE  mov     dword_8FADF8, edx
0x006DE4D4  mov     dword_8FADFC, eax
0x006DE4D9  mov     dword_8FADE0, ecx
0x006DE4DF  call    sub_6E2F50
0x006DE4E4  cmp     g_RoadReflectionEnable, 1
0x006DE4EB  jle     loc_6DE582
0x006DE4F1  mov     edx, dword_9196B8
0x006DE4F7  fld     dword ptr [edx+36A0h]
0x006DE4FD  fcomp   ds:flt_890DC4
0x006DE503  fnstsw  ax
0x006DE505  test    ah, 41h
0x006DE508  jnz     short loc_6DE582
0x006DE50A  push    8
0x006DE50C  push    offset dword_9197A0
0x006DE511  lea     ecx, [esp+0AB8h+var_9D0]
0x006DE518  call    StuffScenery__20GrandSceneryCullInfoP5eViewi; GrandSceneryCullInfo::StuffScenery(eView *,int)
0x006DE51D  mov     ecx, dword_93DEBC
0x006DE523  mov     dword_982C80, ecx
0x006DE529  mov     eax, [ecx]
0x006DE52B  mov     esi, ecx
0x006DE52D  call    dword ptr [eax+4]
0x006DE530  mov     edx, [esi+4Ch]
0x006DE533  mov     eax, dword_982BDC
0x006DE538  mov     ecx, [eax]
0x006DE53A  push    edx
0x006DE53B  push    eax
0x006DE53C  call    dword ptr [ecx+15Ch]
0x006DE542  mov     esi, [esi+48h]
0x006DE545  mov     eax, [esi]
0x006DE547  push    ebx
0x006DE548  lea     ecx, [esp+0AB4h+var_AA4]
0x006DE54C  push    ecx
0x006DE54D  push    esi
0x006DE54E  call    dword ptr [eax+0FCh]
0x006DE554  push    1
0x006DE556  push    offset dword_9197A0
0x006DE55B  call    sub_750B10
0x006DE560  mov     ecx, dword_93DEBC
0x006DE566  mov     edx, [ecx]
0x006DE568  add     esp, 8
0x006DE56B  mov     esi, ecx
0x006DE56D  call    dword ptr [edx+8]
0x006DE570  mov     eax, [esi+48h]
0x006DE573  mov     ecx, [eax]
0x006DE575  push    eax
0x006DE576  call    dword ptr [ecx+10Ch]
0x006DE57C  mov     dword_982C80, ebx
0x006DE582  call    sub_6E2F50
0x006DE587  cmp     TheGameFlowManager, 6
0x006DE58E  jnz     short loc_6DE5B7
0x006DE590  cmp     g_RoadReflectionEnable, 3
0x006DE597  jnz     short loc_6DE5B7
0x006DE599  mov     edx, dword_9196B8
0x006DE59F  fld     dword ptr [edx+36A0h]
0x006DE5A5  fcomp   ds:flt_890DC4
0x006DE5AB  fnstsw  ax
0x006DE5AD  test    ah, 41h
0x006DE5B0  jnz     short loc_6DE5B7
0x006DE5B2  call    sub_6D4960
0x006DE5B7  mov     ecx, dword_93DEBC
0x006DE5BD  mov     dword_982C80, ecx
0x006DE5C3  mov     eax, [ecx]
0x006DE5C5  mov     esi, ecx
0x006DE5C7  call    dword ptr [eax+4]
0x006DE5CA  mov     edx, [esi+4Ch]
0x006DE5CD  mov     eax, dword_982BDC
0x006DE5D2  mov     ecx, [eax]
0x006DE5D4  push    edx
0x006DE5D5  push    eax
0x006DE5D6  call    dword ptr [ecx+15Ch]
0x006DE5DC  mov     esi, [esi+48h]
0x006DE5DF  mov     eax, [esi]
0x006DE5E1  push    ebx
0x006DE5E2  lea     ecx, [esp+0AB4h+var_AA4]
0x006DE5E6  push    ecx
0x006DE5E7  push    esi
0x006DE5E8  call    dword ptr [eax+0FCh]
0x006DE5EE  cmp     dword_9017E4, ebx; g_LightGlowEnable
0x006DE5F4  jz      short loc_6DE628
0x006DE5F6  push    2
0x006DE5F8  push    offset dword_9197A0
0x006DE5FD  call    eRenderWorldLightFlares__FP5eView9flareType; eRenderWorldLightFlares(eView *,flareType)
0x006DE602  push    ebx
0x006DE603  push    1
0x006DE605  push    offset dword_9197A0
0x006DE60A  call    RenderVehicleFlares__FP5eViewii; RenderVehicleFlares(eView *,int,int)
0x006DE60F  push    1
0x006DE611  push    offset dword_9197A0
0x006DE616  call    nullsub_47
0x006DE61B  push    offset dword_9197A0; float
0x006DE620  call    eRenderLightFlarePool__FP5eView; eRenderLightFlarePool(eView *)
0x006DE625  add     esp, 20h
0x006DE628  mov     ecx, dword_93DEBC
0x006DE62E  mov     edx, [ecx]
0x006DE630  mov     esi, ecx
0x006DE632  call    dword ptr [edx+8]
0x006DE635  mov     eax, [esi+48h]
0x006DE638  mov     ecx, [eax]
0x006DE63A  push    eax
0x006DE63B  call    dword ptr [ecx+10Ch]
0x006DE641  mov     dword_982C80, ebx
0x006DE647  mov     edi, 6
0x006DE64C  cmp     g_CarEnvironmentMapEnable, ebx
0x006DE652  jz      loc_6DE85D
0x006DE658  cmp     TheGameFlowManager, edi
0x006DE65E  mov     byte_982BAC, 1
0x006DE665  mov     dword_8FAE6C, 3
0x006DE66F  jnz     loc_6DE88E
0x006DE675  mov     esi, offset unk_919DC0
0x006DE67A  lea     ebx, [ebx+0]
0x006DE680  cmp     [esi+8], bl
0x006DE683  jz      loc_6DE84C
0x006DE689  mov     eax, [esi+60h]
0x006DE68C  cmp     [eax+10h], ebx
0x006DE68F  jz      loc_6DE84C
0x006DE695  push    1
0x006DE697  push    eax
0x006DE698  call    sub_6BD3B0
0x006DE69D  mov     edx, [esi+68h]
0x006DE6A0  mov     eax, [edx+290h]
0x006DE6A6  mov     ecx, TimeOfDayPointer
0x006DE6AC  add     esp, 8
0x006DE6AF  push    eax
0x006DE6B0  call    sub_769580
0x006DE6B5  mov     eax, dword_982BDC
0x006DE6BA  mov     ecx, [eax]
0x006DE6BC  push    ebx
0x006DE6BD  push    7
0x006DE6BF  push    eax
0x006DE6C0  call    dword ptr [ecx+0E4h]
0x006DE6C6  mov     eax, dword_982BDC
0x006DE6CB  mov     edx, [eax]
0x006DE6CD  push    ebx
0x006DE6CE  push    0Eh
0x006DE6D0  push    eax
0x006DE6D1  call    dword ptr [edx+0E4h]
0x006DE6D7  call    sub_6C00B0
0x006DE6DC  cmp     TheGameFlowManager, edi
0x006DE6E2  jnz     loc_6DE77B
0x006DE6E8  mov     eax, TimeOfDayPointer
0x006DE6ED  fld     dword ptr [eax+64h]
0x006DE6F0  mov     ecx, dword_8FADF4
0x006DE6F6  mov     edx, dword_8FADF8
0x006DE6FC  fst     [esp+0AB0h+var_AA4]
0x006DE700  mov     eax, dword_8FADFC
0x006DE705  mov     edi, dword_8FADF0
0x006DE70B  mov     [esp+0AB0h+var_A90], ecx
0x006DE70F  mov     ecx, dword_8FADE0
0x006DE715  fstp    dword_8FADE0
0x006DE71B  mov     [esp+0AB0h+var_A8C], edx
0x006DE71F  mov     edx, dword_93DEC4
0x006DE725  mov     [esp+0AB0h+var_A88], eax
0x006DE729  cmp     [edx+40h], bl
0x006DE72C  mov     [esp+0AB0h+var_A98], ecx
0x006DE730  jz      short loc_6DE739
0x006DE732  mov     eax, [esp+0AB0h+var_AA4]
0x006DE736  push    eax
0x006DE737  jmp     short loc_6DE73E
0x006DE739  push    3F000000h; float
0x006DE73E  push    3; int
0x006DE740  push    esi; int
0x006DE741  call    StuffSkyLayer__FP5eView9SKY_LAYER; StuffSkyLayer(eView *,SKY_LAYER)
0x006DE746  add     esp, 0Ch
0x006DE749  call    sub_6E2F50
0x006DE74E  mov     ecx, [esp+0AB0h+var_A90]
0x006DE752  mov     edx, [esp+0AB0h+var_A8C]
0x006DE756  mov     eax, [esp+0AB0h+var_A88]
0x006DE75A  mov     dword_8FADF4, ecx
0x006DE760  mov     ecx, [esp+0AB0h+var_A98]
0x006DE764  mov     dword_8FADF0, edi
0x006DE76A  mov     dword_8FADF8, edx
0x006DE770  mov     dword_8FADFC, eax
0x006DE775  mov     dword_8FADE0, ecx
0x006DE77B  mov     eax, dword_982BDC
0x006DE780  mov     edx, [eax]
0x006DE782  push    1
0x006DE784  push    0Eh
0x006DE786  push    eax
0x006DE787  call    dword ptr [edx+0E4h]
0x006DE78D  mov     eax, dword_982BDC
0x006DE792  mov     ecx, [eax]
0x006DE794  push    1
0x006DE796  push    7
0x006DE798  push    eax
0x006DE799  call    dword ptr [ecx+0E4h]
0x006DE79F  call    sub_6E2F50
0x006DE7A4  cmp     g_CarEnvironmentMapUpdateData, ebx
0x006DE7AA  jz      short loc_6DE7AF
0x006DE7AC  push    ebx
0x006DE7AD  jmp     short loc_6DE7BD
0x006DE7AF  cmp     dword_901830, 2
0x006DE7B6  jge     short loc_6DE7CA
0x006DE7B8  push    1000h
0x006DE7BD  lea     ecx, [esp+0AB4h+var_9D0]
0x006DE7C4  push    esi
0x006DE7C5  call    StuffScenery__20GrandSceneryCullInfoP5eViewi; GrandSceneryCullInfo::StuffScenery(eView *,int)
0x006DE7CA  call    sub_6E2F50
0x006DE7CF  mov     ecx, dword_93DEBC
0x006DE7D5  mov     dword_982C80, ecx
0x006DE7DB  mov     edx, [ecx]
0x006DE7DD  mov     edi, ecx
0x006DE7DF  call    dword ptr [edx+4]
0x006DE7E2  mov     edx, [edi+4Ch]
0x006DE7E5  mov     eax, dword_982BDC
0x006DE7EA  mov     ecx, [eax]
0x006DE7EC  push    edx
0x006DE7ED  push    eax
0x006DE7EE  call    dword ptr [ecx+15Ch]
0x006DE7F4  mov     edi, [edi+48h]
0x006DE7F7  mov     eax, [edi]
0x006DE7F9  push    ebx
0x006DE7FA  lea     ecx, [esp+0AB4h+var_A7C]
0x006DE7FE  push    ecx
0x006DE7FF  push    edi
0x006DE800  call    dword ptr [eax+0FCh]
0x006DE806  cmp     dword_9017E4, ebx; g_LightGlowEnable
0x006DE80C  jz      short loc_6DE828
0x006DE80E  push    1
0x006DE810  push    esi
0x006DE811  call    eRenderWorldLightFlares__FP5eView9flareType; eRenderWorldLightFlares(eView *,flareType)
0x006DE816  push    esi; float
0x006DE817  call    eRenderLightFlarePool__FP5eView; eRenderLightFlarePool(eView *)
0x006DE81C  push    1
0x006DE81E  push    ebx
0x006DE81F  push    esi
0x006DE820  call    RenderVehicleFlares__FP5eViewii; RenderVehicleFlares(eView *,int,int)
0x006DE825  add     esp, 18h
0x006DE828  mov     ecx, dword_93DEBC
0x006DE82E  mov     edx, [ecx]
0x006DE830  mov     edi, ecx
0x006DE832  call    dword ptr [edx+8]
0x006DE835  mov     eax, [edi+48h]
0x006DE838  mov     ecx, [eax]
0x006DE83A  push    eax
0x006DE83B  call    dword ptr [ecx+10Ch]
0x006DE841  mov     dword_982C80, ebx
0x006DE847  mov     edi, 6
0x006DE84C  add     esi, 70h ; 'p'
0x006DE84F  cmp     esi, offset unk_919FF0
0x006DE855  jle     loc_6DE680
0x006DE85B  jmp     short loc_6DE88E
0x006DE85D  cmp     byte_982BAC, bl
0x006DE863  jz      short loc_6DE88E
0x006DE865  mov     esi, offset dword_919E20
0x006DE86A  lea     ebx, [ebx+0]
0x006DE870  mov     edx, [esi]
0x006DE872  push    2
0x006DE874  push    edx
0x006DE875  call    sub_6BD3B0
0x006DE87A  add     esi, 70h ; 'p'
0x006DE87D  add     esp, 8
0x006DE880  cmp     esi, offset dword_91A050
0x006DE886  jle     short loc_6DE870
0x006DE888  mov     byte_982BAC, bl
0x006DE88E  call    sub_6E2F50
0x006DE893  mov     eax, dword_982BDC
0x006DE898  mov     ecx, [eax]
0x006DE89A  push    1
0x006DE89C  push    0Eh
0x006DE89E  push    eax
0x006DE89F  mov     dword_8FAE6C, 2
0x006DE8A9  call    dword ptr [ecx+0E4h]
0x006DE8AF  mov     eax, dword_982BDC
0x006DE8B4  mov     edx, [eax]
0x006DE8B6  push    1
0x006DE8B8  push    7
0x006DE8BA  push    eax
0x006DE8BB  call    dword ptr [edx+0E4h]
0x006DE8C1  cmp     byte_919738, bl
0x006DE8C7  jz      loc_6DEB01
0x006DE8CD  push    1
0x006DE8CF  push    offset unk_93DB0C
0x006DE8D4  call    sub_6BD3B0
0x006DE8D9  mov     eax, TimeOfDayPointer
0x006DE8DE  fld     dword ptr [eax+64h]
0x006DE8E1  mov     ecx, dword_8FADF8
0x006DE8E7  mov     eax, dword_8FADE0
0x006DE8EC  fst     [esp+0AB8h+var_AA4]
0x006DE8F0  mov     edx, dword_8FADFC
0x006DE8F6  fstp    dword_8FADE0
0x006DE8FC  mov     esi, dword_8FADF0
0x006DE902  mov     edi, dword_8FADF4
0x006DE908  mov     [esp+0AB8h+var_A8C], ecx
0x006DE90C  mov     ecx, dword_93DEC4
0x006DE912  mov     [esp+0AB8h+var_A98], eax
0x006DE916  mov     al, [ecx+40h]
0x006DE919  add     esp, 8
0x006DE91C  cmp     al, bl
0x006DE91E  mov     [esp+0AB0h+var_A88], edx
0x006DE922  jz      short loc_6DE92B
0x006DE924  mov     edx, [esp+0AB0h+var_AA4]
0x006DE928  push    edx
0x006DE929  jmp     short loc_6DE930
0x006DE92B  push    3F000000h; float
0x006DE930  push    3; int
0x006DE932  push    offset dword_919730; int
0x006DE937  call    StuffSkyLayer__FP5eView9SKY_LAYER; StuffSkyLayer(eView *,SKY_LAYER)
0x006DE93C  add     esp, 0Ch
0x006DE93F  call    sub_6E2F50
0x006DE944  mov     eax, [esp+0AB0h+var_A8C]
0x006DE948  mov     ecx, [esp+0AB0h+var_A88]
0x006DE94C  mov     edx, [esp+0AB0h+var_A98]
0x006DE950  mov     dword_8FADF0, esi
0x006DE956  mov     dword_8FADF4, edi
0x006DE95C  mov     dword_8FADF8, eax
0x006DE961  mov     dword_8FADFC, ecx
0x006DE967  mov     dword_8FADE0, edx
0x006DE96D  call    sub_6E2F50
0x006DE972  push    1
0x006DE974  push    offset dword_919730
0x006DE979  lea     ecx, [esp+0AB8h+var_9D0]
0x006DE980  call    StuffScenery__20GrandSceneryCullInfoP5eViewi; GrandSceneryCullInfo::StuffScenery(eView *,int)
0x006DE985  push    ebx
0x006DE986  push    offset dword_919730
0x006DE98B  call    sub_750B10
0x006DE990  mov     ecx, dword_93DEBC
0x006DE996  mov     dword_982C80, ecx
0x006DE99C  mov     eax, [ecx]
0x006DE99E  add     esp, 8
0x006DE9A1  mov     esi, ecx
0x006DE9A3  call    dword ptr [eax+4]
0x006DE9A6  mov     edx, [esi+4Ch]
0x006DE9A9  mov     eax, dword_982BDC
0x006DE9AE  mov     ecx, [eax]
0x006DE9B0  push    edx
0x006DE9B1  push    eax
0x006DE9B2  call    dword ptr [ecx+15Ch]
0x006DE9B8  mov     esi, [esi+48h]
0x006DE9BB  mov     eax, [esi]
0x006DE9BD  push    ebx
0x006DE9BE  lea     ecx, [esp+0AB4h+var_AA4]
0x006DE9C2  push    ecx
0x006DE9C3  push    esi
0x006DE9C4  call    dword ptr [eax+0FCh]
0x006DE9CA  cmp     dword_9017E4, ebx; g_LightGlowEnable
0x006DE9D0  jz      short loc_6DE9F8
0x006DE9D2  push    ebx
0x006DE9D3  push    ebx
0x006DE9D4  push    offset dword_919730
0x006DE9D9  call    RenderVehicleFlares__FP5eViewii; RenderVehicleFlares(eView *,int,int)
0x006DE9DE  mov     eax, dword_9017E4; g_LightGlowEnable
0x006DE9E3  add     esp, 0Ch
0x006DE9E6  cmp     eax, ebx
0x006DE9E8  jz      short loc_6DE9F8
0x006DE9EA  push    ebx
0x006DE9EB  push    offset dword_919730
0x006DE9F0  call    eRenderWorldLightFlares__FP5eView9flareType; eRenderWorldLightFlares(eView *,flareType)
0x006DE9F5  add     esp, 8
0x006DE9F8  mov     ecx, dword_93DEBC
0x006DE9FE  mov     edx, [ecx]
0x006DEA00  mov     esi, ecx
0x006DEA02  call    dword ptr [edx+8]
0x006DEA05  mov     eax, [esi+48h]
0x006DEA08  mov     ecx, [eax]
0x006DEA0A  push    eax
0x006DEA0B  call    dword ptr [ecx+10Ch]
0x006DEA11  mov     ecx, dword_93DE9C
0x006DEA17  mov     dword_982C80, ecx
0x006DEA1D  mov     edx, [ecx]
0x006DEA1F  mov     esi, ecx
0x006DEA21  call    dword ptr [edx+4]
0x006DEA24  mov     edx, [esi+4Ch]
0x006DEA27  mov     eax, dword_982BDC
0x006DEA2C  mov     ecx, [eax]
0x006DEA2E  push    edx
0x006DEA2F  push    eax
0x006DEA30  call    dword ptr [ecx+15Ch]
0x006DEA36  mov     esi, [esi+48h]
0x006DEA39  mov     eax, [esi]
0x006DEA3B  push    ebx
0x006DEA3C  lea     ecx, [esp+0AB4h+var_AA4]
0x006DEA40  push    ecx
0x006DEA41  push    esi
0x006DEA42  call    dword ptr [eax+0FCh]
0x006DEA48  cmp     dword_982B48, ebx
0x006DEA4E  jz      short loc_6DEA85
0x006DEA50  mov     ecx, dword_93DE9C
0x006DEA56  mov     eax, [ecx+48h]
0x006DEA59  mov     ecx, [ecx+44h]
0x006DEA5C  mov     ecx, [ecx+484h]
0x006DEA62  mov     edx, [eax]
0x006DEA64  push    0Fh
0x006DEA66  push    ecx
0x006DEA67  push    eax
0x006DEA68  call    dword ptr [edx+68h]
0x006DEA6B  mov     edx, dword_982B48
0x006DEA71  mov     eax, [edx]
0x006DEA73  mov     ecx, [eax+18h]
0x006DEA76  push    6
0x006DEA78  push    5
0x006DEA7A  push    0FFFFFFFFh
0x006DEA7C  push    ecx
0x006DEA7D  call    sub_6CFF40
0x006DEA82  add     esp, 10h
0x006DEA85  mov     ecx, dword_93DE9C
0x006DEA8B  mov     eax, [ecx+48h]
0x006DEA8E  mov     ecx, [ecx+44h]
0x006DEA91  mov     ecx, [ecx+484h]
0x006DEA97  mov     edx, [eax]
0x006DEA99  push    8
0x006DEA9B  push    ecx
0x006DEA9C  push    eax
0x006DEA9D  call    dword ptr [edx+68h]
0x006DEAA0  mov     eax, dword_982B4C
0x006DEAA5  cmp     eax, ebx
0x006DEAA7  jz      short loc_6DEABD
0x006DEAA9  mov     edx, [eax]
0x006DEAAB  mov     eax, [edx+18h]
0x006DEAAE  push    1
0x006DEAB0  push    2
0x006DEAB2  push    0FFFFFFFFh
0x006DEAB4  push    eax
0x006DEAB5  call    sub_6CFF40
0x006DEABA  add     esp, 10h
0x006DEABD  mov     ecx, dword_93DE9C
0x006DEAC3  mov     edx, [ecx]
0x006DEAC5  mov     esi, ecx
0x006DEAC7  call    dword ptr [edx+8]
0x006DEACA  mov     eax, [esi+48h]
0x006DEACD  mov     ecx, [eax]
0x006DEACF  push    eax
0x006DEAD0  call    dword ptr [ecx+10Ch]
0x006DEAD6  mov     ecx, dword_93DE9C
0x006DEADC  mov     dword_982C80, ebx
0x006DEAE2  mov     eax, [ecx+48h]
0x006DEAE5  mov     ecx, [ecx+44h]
0x006DEAE8  mov     ecx, [ecx+484h]
0x006DEAEE  mov     edx, [eax]
0x006DEAF0  push    0Fh
0x006DEAF2  push    ecx
0x006DEAF3  push    eax
0x006DEAF4  call    dword ptr [edx+68h]
0x006DEAF7  call    sub_6CFCE0
0x006DEAFC  mov     edi, 6
0x006DEB01  call    sub_6E2F50
0x006DEB06  xor     eax, eax
0x006DEB08  mov     dword_93DE38, eax
0x006DEB0D  xor     edx, edx
0x006DEB0F  mov     dword_93DE3C, eax
0x006DEB14  mov     dword_9828A0, edx
0x006DEB1A  mov     dword_93DE40, eax
0x006DEB1F  mov     dword_9828A4, edx
0x006DEB25  mov     dword_93DE44, eax
0x006DEB2A  mov     dword_9828A8, edx
0x006DEB30  mov     dword_93DE48, eax
0x006DEB35  mov     dword_9828AC, edx
0x006DEB3B  mov     dword_93DE4C, eax
0x006DEB40  mov     dword_9828B0, edx
0x006DEB46  mov     dword_93DE50, eax
0x006DEB4B  mov     dword_9828B4, edx
0x006DEB51  mov     dword_93DE54, eax
0x006DEB56  cmp     byte_919658, bl
0x006DEB5C  mov     dword_9828B8, edx
0x006DEB62  mov     dword_982B90, ebx
0x006DEB68  mov     dword_982B94, ebx
0x006DEB6E  mov     dword_982B98, ebx
0x006DEB74  mov     dword_9828BC, edx
0x006DEB7A  jz      loc_6DF59E
0x006DEB80  mov     ecx, dword_9196B0
0x006DEB86  cmp     [ecx+10h], ebx
0x006DEB89  jz      loc_6DF59E
0x006DEB8F  mov     edx, dword_9196B8
0x006DEB95  mov     eax, [edx+290h]
0x006DEB9B  mov     ecx, TimeOfDayPointer
0x006DEBA1  push    eax
0x006DEBA2  call    sub_769580
0x006DEBA7  push    ebx
0x006DEBA8  push    offset dword_919650
0x006DEBAD  mov     ecx, offset unk_9AE364
0x006DEBB2  call    sub_753D90
0x006DEBB7  mov     esi, dword_9196B0
0x006DEBBD  call    sub_6E2F50
0x006DEBC2  mov     edx, [esi+8]
0x006DEBC5  mov     eax, dword_982BDC
0x006DEBCA  mov     ecx, [eax]
0x006DEBCC  push    edx
0x006DEBCD  push    ebx
0x006DEBCE  push    eax
0x006DEBCF  call    dword ptr [ecx+94h]
0x006DEBD5  mov     edx, [esi+0Ch]
0x006DEBD8  mov     eax, dword_982BDC
0x006DEBDD  mov     ecx, [eax]
0x006DEBDF  push    edx
0x006DEBE0  push    eax
0x006DEBE1  call    dword ptr [ecx+9Ch]
0x006DEBE7  mov     ecx, [esi+18h]
0x006DEBEA  mov     eax, [esi+14h]
0x006DEBED  mov     [esp+0AB0h+var_A88], ecx
0x006DEBF1  mov     [esp+0AB0h+var_A8C], eax
0x006DEBF5  mov     eax, dword_982BDC
0x006DEBFA  lea     ecx, [esp+0AB0h+var_A94]
0x006DEBFE  push    ecx
0x006DEBFF  mov     [esp+0AB4h+var_A94], ebx
0x006DEC03  mov     [esp+0AB4h+var_A90], ebx
0x006DEC07  mov     [esp+0AB4h+var_A84], 0
0x006DEC0F  mov     [esp+0AB4h+var_A80], 3F800000h
0x006DEC17  mov     edx, [eax]
0x006DEC19  push    eax
0x006DEC1A  call    dword ptr [edx+0BCh]
0x006DEC20  cmp     dword_9017C4, ebx
0x006DEC26  mov     dword_982A20, esi
0x006DEC2C  mov     dword_982C80, ebx
0x006DEC32  jz      short loc_6DEC4A
0x006DEC34  push    offset sub_4FAEE0
0x006DEC39  push    offset unk_919960
0x006DEC3E  lea     ecx, [esp+0AB8h+var_9D0]
0x006DEC45  call    sub_724230
0x006DEC4A  cmp     dword_901830, ebx
0x006DEC50  jle     short loc_6DEC7A
0x006DEC52  fld     flt_8FAE68
0x006DEC58  fcomp   flt_8FAE50
0x006DEC5E  fnstsw  ax
0x006DEC60  test    ah, 1
0x006DEC63  jnz     short loc_6DEC7A
0x006DEC65  lea     edx, [esp+0AB0h+var_9D0]
0x006DEC6C  push    edx
0x006DEC6D  push    offset dword_919650
0x006DEC72  call    sub_6E54E0
0x006DEC77  add     esp, 8
0x006DEC7A  mov     esi, dword_9196B0
0x006DEC80  call    sub_6E2F50
0x006DEC85  mov     edx, [esi+8]
0x006DEC88  mov     eax, dword_982BDC
0x006DEC8D  mov     ecx, [eax]
0x006DEC8F  push    edx
0x006DEC90  push    ebx
0x006DEC91  push    eax
0x006DEC92  call    dword ptr [ecx+94h]
0x006DEC98  mov     edx, [esi+0Ch]
0x006DEC9B  mov     eax, dword_982BDC
0x006DECA0  mov     ecx, [eax]
0x006DECA2  push    edx
0x006DECA3  push    eax
0x006DECA4  call    dword ptr [ecx+9Ch]
0x006DECAA  mov     ecx, [esi+18h]
0x006DECAD  mov     eax, [esi+14h]
0x006DECB0  mov     [esp+0AB0h+var_A88], ecx
0x006DECB4  mov     [esp+0AB0h+var_A8C], eax
0x006DECB8  mov     eax, dword_982BDC
0x006DECBD  lea     ecx, [esp+0AB0h+var_A94]
0x006DECC1  push    ecx
0x006DECC2  mov     [esp+0AB4h+var_A94], ebx
0x006DECC6  mov     [esp+0AB4h+var_A90], ebx
0x006DECCA  mov     [esp+0AB4h+var_A84], 0
0x006DECD2  mov     [esp+0AB4h+var_A80], 3F800000h
0x006DECDA  mov     edx, [eax]
0x006DECDC  push    eax
0x006DECDD  call    dword ptr [edx+0BCh]
0x006DECE3  mov     dword_982A20, esi
0x006DECE9  mov     dword_982C80, ebx
0x006DECEF  call    sub_6CFCE0
0x006DECF4  mov     ecx, dword_93DEC4
0x006DECFA  mov     eax, [ecx+48h]
0x006DECFD  mov     ecx, [ecx+44h]
0x006DED00  mov     ecx, [ecx+18Ch]
0x006DED06  mov     edx, [eax]
0x006DED08  push    7
0x006DED0A  push    ecx
0x006DED0B  push    eax
0x006DED0C  call    dword ptr [edx+68h]
0x006DED0F  mov     edx, dword_8F92E4
0x006DED15  push    ebx; int
0x006DED16  push    edx; float
0x006DED17  push    offset dword_919650; int
0x006DED1C  call    sub_6DE210
0x006DED21  add     esp, 0Ch
0x006DED24  push    20h ; ' '
0x006DED26  push    offset dword_919650
0x006DED2B  lea     ecx, [esp+0AB8h+var_9D0]
0x006DED32  call    StuffScenery__20GrandSceneryCullInfoP5eViewi; GrandSceneryCullInfo::StuffScenery(eView *,int)
0x006DED37  call    sub_6E2F50
0x006DED3C  mov     ecx, dword_93DE78
0x006DED42  mov     dword_982C80, ecx
0x006DED48  mov     eax, [ecx]
0x006DED4A  mov     esi, ecx
0x006DED4C  call    dword ptr [eax+4]
0x006DED4F  mov     edx, [esi+4Ch]
0x006DED52  mov     eax, dword_982BDC
0x006DED57  mov     ecx, [eax]
0x006DED59  push    edx
0x006DED5A  push    eax
0x006DED5B  call    dword ptr [ecx+15Ch]
0x006DED61  mov     esi, [esi+48h]
0x006DED64  mov     eax, [esi]
0x006DED66  push    ebx
0x006DED67  lea     ecx, [esp+0AB4h+var_AA4]
0x006DED6B  push    ecx
0x006DED6C  push    esi
0x006DED6D  call    dword ptr [eax+0FCh]
0x006DED73  push    ebx
0x006DED74  push    offset dword_919650
0x006DED79  call    sub_7538D0
0x006DED7E  mov     ecx, dword_93DE78
0x006DED84  mov     edx, [ecx]
0x006DED86  add     esp, 8
0x006DED89  mov     esi, ecx
0x006DED8B  call    dword ptr [edx+8]
0x006DED8E  mov     eax, [esi+48h]
0x006DED91  mov     ecx, [eax]
0x006DED93  push    eax
0x006DED94  call    dword ptr [ecx+10Ch]
0x006DED9A  mov     dword_982C80, ebx
0x006DEDA0  call    sub_6E2F50
0x006DEDA5  mov     ecx, dword_93DEBC
0x006DEDAB  mov     dword_982C80, ecx
0x006DEDB1  mov     edx, [ecx]
0x006DEDB3  mov     esi, ecx
0x006DEDB5  call    dword ptr [edx+4]
0x006DEDB8  mov     edx, [esi+4Ch]
0x006DEDBB  mov     eax, dword_982BDC
0x006DEDC0  mov     ecx, [eax]
0x006DEDC2  push    edx
0x006DEDC3  push    eax
0x006DEDC4  call    dword ptr [ecx+15Ch]
0x006DEDCA  mov     esi, [esi+48h]
0x006DEDCD  mov     eax, [esi]
0x006DEDCF  push    ebx
0x006DEDD0  lea     ecx, [esp+0AB4h+var_AA4]
0x006DEDD4  push    ecx
0x006DEDD5  push    esi
0x006DEDD6  call    dword ptr [eax+0FCh]
0x006DEDDC  push    offset dword_919650
0x006DEDE1  call    sub_7268C0
0x006DEDE6  mov     ecx, dword_93DEBC
0x006DEDEC  mov     edx, [ecx]
0x006DEDEE  add     esp, 4
0x006DEDF1  mov     esi, ecx
0x006DEDF3  call    dword ptr [edx+8]
0x006DEDF6  mov     eax, [esi+48h]
0x006DEDF9  mov     ecx, [eax]
0x006DEDFB  push    eax
0x006DEDFC  call    dword ptr [ecx+10Ch]
0x006DEE02  mov     ecx, dword_93DEBC
0x006DEE08  mov     dword_982C80, ecx
0x006DEE0E  mov     edx, [ecx]
0x006DEE10  mov     esi, ecx
0x006DEE12  call    dword ptr [edx+4]
0x006DEE15  mov     edx, [esi+4Ch]
0x006DEE18  mov     eax, dword_982BDC
0x006DEE1D  mov     ecx, [eax]
0x006DEE1F  push    edx
0x006DEE20  push    eax
0x006DEE21  call    dword ptr [ecx+15Ch]
0x006DEE27  mov     esi, [esi+48h]
0x006DEE2A  mov     eax, [esi]
0x006DEE2C  push    ebx
0x006DEE2D  lea     ecx, [esp+0AB4h+var_AA4]
0x006DEE31  push    ecx
0x006DEE32  push    esi
0x006DEE33  call    dword ptr [eax+0FCh]
0x006DEE39  push    ebx
0x006DEE3A  push    offset dword_919650
0x006DEE3F  call    sub_750B10
0x006DEE44  mov     ecx, dword_93DEBC
0x006DEE4A  mov     edx, [ecx]
0x006DEE4C  add     esp, 8
0x006DEE4F  mov     esi, ecx
0x006DEE51  call    dword ptr [edx+8]
0x006DEE54  mov     eax, [esi+48h]
0x006DEE57  mov     ecx, [eax]
0x006DEE59  push    eax
0x006DEE5A  call    dword ptr [ecx+10Ch]
0x006DEE60  mov     dword_982C80, ebx
0x006DEE66  call    sub_6E2F50
0x006DEE6B  mov     ecx, dword_93DEBC
0x006DEE71  mov     dword_982C80, ecx
0x006DEE77  mov     edx, [ecx]
0x006DEE79  mov     esi, ecx
0x006DEE7B  call    dword ptr [edx+4]
0x006DEE7E  mov     edx, [esi+4Ch]
0x006DEE81  mov     eax, dword_982BDC
0x006DEE86  mov     ecx, [eax]
0x006DEE88  push    edx
0x006DEE89  push    eax
0x006DEE8A  call    dword ptr [ecx+15Ch]
0x006DEE90  mov     esi, [esi+48h]
0x006DEE93  mov     eax, [esi]
0x006DEE95  push    ebx
0x006DEE96  lea     ecx, [esp+0AB4h+var_AA4]
0x006DEE9A  push    ecx
0x006DEE9B  push    esi
0x006DEE9C  call    dword ptr [eax+0FCh]
0x006DEEA2  call    nullsub_28
0x006DEEA7  mov     ecx, dword_93DEBC
0x006DEEAD  mov     edx, [ecx]
0x006DEEAF  mov     esi, ecx
0x006DEEB1  call    dword ptr [edx+8]
0x006DEEB4  mov     eax, [esi+48h]
0x006DEEB7  mov     ecx, [eax]
0x006DEEB9  push    eax
0x006DEEBA  call    dword ptr [ecx+10Ch]
0x006DEEC0  mov     dword_982C80, ebx
0x006DEEC6  call    sub_6E2F50
0x006DEECB  cmp     TheGameFlowManager, 3
0x006DEED2  jnz     loc_6DEFC2
0x006DEED8  mov     ecx, dword_93DEBC
0x006DEEDE  mov     dword_982C80, ecx
0x006DEEE4  mov     edx, [ecx]
0x006DEEE6  mov     esi, ecx
0x006DEEE8  call    dword ptr [edx+4]
0x006DEEEB  mov     edx, [esi+4Ch]
0x006DEEEE  mov     eax, dword_982BDC
0x006DEEF3  mov     ecx, [eax]
0x006DEEF5  push    edx
0x006DEEF6  push    eax
0x006DEEF7  call    dword ptr [ecx+15Ch]
0x006DEEFD  mov     esi, [esi+48h]
0x006DEF00  mov     eax, [esi]
0x006DEF02  push    ebx
0x006DEF03  lea     ecx, [esp+0AB4h+var_AA4]
0x006DEF07  push    ecx
0x006DEF08  push    esi
0x006DEF09  call    dword ptr [eax+0FCh]
0x006DEF0F  push    1; a10
0x006DEF11  push    offset dword_919650; a2
0x006DEF16  call    RenderFrontEndCars__FP5eViewi; RenderFrontEndCars(eView *,int)
0x006DEF1B  mov     ecx, dword_93DEBC
0x006DEF21  mov     edx, [ecx]
0x006DEF23  add     esp, 8
0x006DEF26  mov     esi, ecx
0x006DEF28  call    dword ptr [edx+8]
0x006DEF2B  mov     eax, [esi+48h]
0x006DEF2E  mov     ecx, [eax]
0x006DEF30  push    eax
0x006DEF31  call    dword ptr [ecx+10Ch]
0x006DEF37  mov     dword_982C80, ebx
0x006DEF3D  call    sub_6E2F50
0x006DEF42  call    GetInstance__16GarageMainScreen; GarageMainScreen::GetInstance(void)
0x006DEF47  cmp     eax, ebx
0x006DEF49  jz      short loc_6DEF54
0x006DEF4B  push    1; a4
0x006DEF4D  mov     ecx, eax; this
0x006DEF4F  call    HandleRender__16GarageMainScreenUi; GarageMainScreen::HandleRender(uint)
0x006DEF54  call    sub_6E2F50
0x006DEF59  mov     ecx, dword_93DEBC
0x006DEF5F  mov     dword_982C80, ecx
0x006DEF65  mov     edx, [ecx]
0x006DEF67  mov     esi, ecx
0x006DEF69  call    dword ptr [edx+4]
0x006DEF6C  mov     edx, [esi+4Ch]
0x006DEF6F  mov     eax, dword_982BDC
0x006DEF74  mov     ecx, [eax]
0x006DEF76  push    edx
0x006DEF77  push    eax
0x006DEF78  call    dword ptr [ecx+15Ch]
0x006DEF7E  mov     esi, [esi+48h]
0x006DEF81  mov     eax, [esi]
0x006DEF83  push    ebx
0x006DEF84  lea     ecx, [esp+0AB4h+var_AA4]
0x006DEF88  push    ecx
0x006DEF89  push    esi
0x006DEF8A  call    dword ptr [eax+0FCh]
0x006DEF90  push    ebx; a10
0x006DEF91  push    offset dword_919650; a2
0x006DEF96  call    RenderFrontEndCars__FP5eViewi; RenderFrontEndCars(eView *,int)
0x006DEF9B  mov     ecx, dword_93DEBC
0x006DEFA1  mov     edx, [ecx]
0x006DEFA3  add     esp, 8
0x006DEFA6  mov     esi, ecx
0x006DEFA8  call    dword ptr [edx+8]
0x006DEFAB  mov     eax, [esi+48h]
0x006DEFAE  mov     ecx, [eax]
0x006DEFB0  push    eax
0x006DEFB1  call    dword ptr [ecx+10Ch]
0x006DEFB7  mov     dword_982C80, ebx
0x006DEFBD  call    sub_6E2F50
0x006DEFC2  cmp     TheGameFlowManager, edi
0x006DEFC8  jnz     loc_6DF1CC
0x006DEFCE  mov     eax, eViews_0
0x006DEFD3  cmp     [eax+30h], ebx
0x006DEFD6  jz      loc_6DF1CC
0x006DEFDC  mov     [esp+0AB0h+var_A70], 0
0x006DEFE4  mov     [esp+0AB0h+var_A6C], 0
0x006DEFEC  mov     [esp+0AB0h+var_A68], 0
0x006DEFF4  mov     [esp+0AB0h+var_A60], 42800000h
0x006DEFFC  mov     [esp+0AB0h+var_A5C], 0
0x006DF004  mov     [esp+0AB0h+var_A58], 0
0x006DF00C  mov     [esp+0AB0h+var_A50], 42800000h
0x006DF014  mov     [esp+0AB0h+var_A4C], 42800000h
0x006DF01C  mov     [esp+0AB0h+var_A48], 0
0x006DF024  mov     [esp+0AB0h+var_A40], 0
0x006DF02C  mov     [esp+0AB0h+var_A3C], 42800000h
0x006DF034  mov     [esp+0AB0h+var_A38], 0
0x006DF03C  mov     [esp+0AB0h+var_A30], 0
0x006DF047  mov     [esp+0AB0h+var_A2C], 0
0x006DF052  mov     [esp+0AB0h+var_A28], 3F800000h
0x006DF05D  mov     [esp+0AB0h+var_A24], 0
0x006DF068  mov     [esp+0AB0h+var_A20], 3F800000h
0x006DF073  mov     [esp+0AB0h+var_A1C], 3F800000h
0x006DF07E  mov     [esp+0AB0h+var_A18], 0
0x006DF089  mov     [esp+0AB0h+var_A14], 3F800000h
0x006DF094  mov     [esp+0AB0h+var_9F0+10h], bl
0x006DF09B  mov     ecx, 80808080h
0x006DF0A0  mov     dword ptr [esp+0AB0h+var_9F0], ecx
0x006DF0A7  mov     dword ptr [esp+0AB0h+var_9F0+4], ecx
0x006DF0AE  mov     dword ptr [esp+0AB0h+var_9F0+8], ecx
0x006DF0B5  mov     dword ptr [esp+0AB0h+var_9F0+0Ch], ecx
0x006DF0BC  mov     edx, [eax+14Ch]
0x006DF0C2  mov     [esp+0AB0h+var_A70], edx
0x006DF0C6  mov     ecx, [eax+158h]
0x006DF0CC  mov     [esp+0AB0h+var_A60], ecx
0x006DF0D0  mov     edx, [eax+164h]
0x006DF0D6  mov     [esp+0AB0h+var_A40], edx
0x006DF0DA  mov     ecx, [eax+170h]
0x006DF0E0  mov     [esp+0AB0h+var_A50], ecx
0x006DF0E4  mov     edx, [eax+150h]
0x006DF0EA  mov     [esp+0AB0h+var_A6C], edx
0x006DF0EE  mov     ecx, [eax+15Ch]
0x006DF0F4  mov     [esp+0AB0h+var_A5C], ecx
0x006DF0F8  mov     edx, [eax+168h]
0x006DF0FE  mov     [esp+0AB0h+var_A3C], edx
0x006DF102  mov     ecx, [eax+174h]
0x006DF108  mov     [esp+0AB0h+var_A4C], ecx
0x006DF10C  mov     edx, [eax+154h]
0x006DF112  mov     [esp+0AB0h+var_A68], edx
0x006DF116  mov     ecx, [eax+160h]
0x006DF11C  mov     [esp+0AB0h+var_A58], ecx
0x006DF120  mov     edx, [eax+16Ch]
0x006DF126  mov     [esp+0AB0h+var_A38], edx
0x006DF12A  mov     eax, [eax+178h]
0x006DF130  mov     [esp+0AB0h+var_A48], eax
0x006DF134  xor     eax, eax
0x006DF136  mov     cl, 0FFh
0x006DF138  jmp     short loc_6DF140
0x006DF140  mov     [esp+eax*4+0AB0h+var_9F0], cl
0x006DF147  mov     [esp+eax*4+0AB0h+var_9F0+1], cl
0x006DF14E  mov     [esp+eax*4+0AB0h+var_9F0+2], cl
0x006DF155  mov     [esp+eax*4+0AB0h+var_9F0+3], cl
0x006DF15C  inc     eax
0x006DF15D  cmp     eax, 4
0x006DF160  jl      short loc_6DF140
0x006DF162  mov     ecx, dword_93DE94
0x006DF168  mov     dword_982C80, ecx
0x006DF16E  mov     edx, [ecx]
0x006DF170  mov     esi, ecx
0x006DF172  call    dword ptr [edx+4]
0x006DF175  mov     edx, [esi+4Ch]
0x006DF178  mov     eax, dword_982BDC
0x006DF17D  mov     ecx, [eax]
0x006DF17F  push    edx
0x006DF180  push    eax
0x006DF181  call    dword ptr [ecx+15Ch]
0x006DF187  mov     esi, [esi+48h]
0x006DF18A  mov     eax, [esi]
0x006DF18C  push    ebx
0x006DF18D  lea     ecx, [esp+0AB4h+var_AA4]
0x006DF191  push    ecx
0x006DF192  push    esi
0x006DF193  call    dword ptr [eax+0FCh]
0x006DF199  push    ebx
0x006DF19A  push    offset eMathIdentityMatrix
0x006DF19F  lea     edx, [esp+0AB8h+var_A70]
0x006DF1A3  push    ebx
0x006DF1A4  push    edx
0x006DF1A5  call    sub_6D1A40
0x006DF1AA  mov     ecx, dword_93DE94
0x006DF1B0  mov     eax, [ecx]
0x006DF1B2  add     esp, 10h
0x006DF1B5  mov     esi, ecx
0x006DF1B7  call    dword ptr [eax+8]
0x006DF1BA  mov     eax, [esi+48h]
0x006DF1BD  mov     ecx, [eax]
0x006DF1BF  push    eax
0x006DF1C0  call    dword ptr [ecx+10Ch]
0x006DF1C6  mov     dword_982C80, ebx
0x006DF1CC  cmp     g_MotionBlurEnable, ebx
0x006DF1D2  jz      short loc_6DF1E1
0x006DF1D4  cmp     dword_8F9218, ebx
0x006DF1DA  jz      short loc_6DF1E1
0x006DF1DC  call    sub_6DBB20
0x006DF1E1  call    sub_6CFCE0
0x006DF1E6  push    offset dword_919650
0x006DF1EB  call    sub_41AD20
0x006DF1F0  mov     ecx, dword_93DEBC
0x006DF1F6  mov     dword_982C80, ecx
0x006DF1FC  mov     edx, [ecx]
0x006DF1FE  add     esp, 4
0x006DF201  mov     esi, ecx
0x006DF203  call    dword ptr [edx+4]
0x006DF206  mov     edx, [esi+4Ch]
0x006DF209  mov     eax, dword_982BDC
0x006DF20E  mov     ecx, [eax]
0x006DF210  push    edx
0x006DF211  push    eax
0x006DF212  call    dword ptr [ecx+15Ch]
0x006DF218  mov     esi, [esi+48h]
0x006DF21B  mov     eax, [esi]
0x006DF21D  push    ebx
0x006DF21E  lea     ecx, [esp+0AB4h+var_AA4]
0x006DF222  push    ecx
0x006DF223  push    esi
0x006DF224  call    dword ptr [eax+0FCh]
0x006DF22A  call    nullsub_28
0x006DF22F  mov     ecx, dword_93DEBC
0x006DF235  mov     edx, [ecx]
0x006DF237  mov     esi, ecx
0x006DF239  call    dword ptr [edx+8]
0x006DF23C  mov     eax, [esi+48h]
0x006DF23F  mov     ecx, [eax]
0x006DF241  push    eax
0x006DF242  call    dword ptr [ecx+10Ch]
0x006DF248  mov     dword_982C80, ebx
0x006DF24E  call    sub_6E2F50
0x006DF253  mov     ecx, dword_93DE98
0x006DF259  mov     dword_982C80, ecx
0x006DF25F  mov     edx, [ecx]
0x006DF261  mov     esi, ecx
0x006DF263  call    dword ptr [edx+4]
0x006DF266  mov     edx, [esi+4Ch]
0x006DF269  mov     eax, dword_982BDC
0x006DF26E  mov     ecx, [eax]
0x006DF270  push    edx
0x006DF271  push    eax
0x006DF272  call    dword ptr [ecx+15Ch]
0x006DF278  mov     esi, [esi+48h]
0x006DF27B  mov     eax, [esi]
0x006DF27D  push    ebx
0x006DF27E  lea     ecx, [esp+0AB4h+var_AA4]
0x006DF282  push    ecx
0x006DF283  push    esi
0x006DF284  call    dword ptr [eax+0FCh]
0x006DF28A  mov     ecx, dword_93DE98
0x006DF290  mov     edx, [ecx]
0x006DF292  mov     esi, ecx
0x006DF294  call    dword ptr [edx+8]
0x006DF297  mov     eax, [esi+48h]
0x006DF29A  mov     ecx, [eax]
0x006DF29C  push    eax
0x006DF29D  call    dword ptr [ecx+10Ch]
0x006DF2A3  cmp     dword_9017E4, ebx; g_LightGlowEnable
0x006DF2A9  mov     dword_982C80, ebx
0x006DF2AF  jz      loc_6DF39D
0x006DF2B5  cmp     g_X360EffectsEnable, ebx
0x006DF2BB  jz      short loc_6DF2EA
0x006DF2BD  mov     eax, dword_982BDC
0x006DF2C2  mov     edx, [eax]
0x006DF2C4  push    ebx
0x006DF2C5  push    0Eh
0x006DF2C7  push    eax
0x006DF2C8  call    dword ptr [edx+0E4h]
0x006DF2CE  call    sub_6DAE20
0x006DF2D3  mov     eax, dword_982BDC
0x006DF2D8  mov     ecx, [eax]
0x006DF2DA  push    1
0x006DF2DC  push    0Eh
0x006DF2DE  push    eax
0x006DF2DF  call    dword ptr [ecx+0E4h]
0x006DF2E5  jmp     loc_6DF39D
0x006DF2EA  mov     ecx, dword_93DEBC
0x006DF2F0  mov     dword_982C80, ecx
0x006DF2F6  mov     edx, [ecx]
0x006DF2F8  mov     esi, ecx
0x006DF2FA  call    dword ptr [edx+4]
0x006DF2FD  mov     edx, [esi+4Ch]
0x006DF300  mov     eax, dword_982BDC
0x006DF305  mov     ecx, [eax]
0x006DF307  push    edx
0x006DF308  push    eax
0x006DF309  call    dword ptr [ecx+15Ch]
0x006DF30F  mov     esi, [esi+48h]
0x006DF312  mov     eax, [esi]
0x006DF314  push    ebx
0x006DF315  lea     ecx, [esp+0AB4h+var_AA4]
0x006DF319  push    ecx
0x006DF31A  push    esi
0x006DF31B  call    dword ptr [eax+0FCh]
0x006DF321  mov     eax, dword_982BDC
0x006DF326  mov     edx, [eax]
0x006DF328  push    ebx
0x006DF329  push    0Eh
0x006DF32B  push    eax
0x006DF32C  call    dword ptr [edx+0E4h]
0x006DF332  push    ebx
0x006DF333  push    offset dword_919650
0x006DF338  call    eRenderWorldLightFlares__FP5eView9flareType; eRenderWorldLightFlares(eView *,flareType)
0x006DF33D  push    ebx
0x006DF33E  push    ebx
0x006DF33F  push    offset dword_919650
0x006DF344  call    RenderVehicleFlares__FP5eViewii; RenderVehicleFlares(eView *,int,int)
0x006DF349  push    ebx
0x006DF34A  push    offset dword_919650
0x006DF34F  call    nullsub_47
0x006DF354  push    offset dword_919650; float
0x006DF359  call    eRenderLightFlarePool__FP5eView; eRenderLightFlarePool(eView *)
0x006DF35E  push    ebx
0x006DF35F  push    offset dword_919650
0x006DF364  call    nullsub_46
0x006DF369  mov     eax, dword_982BDC
0x006DF36E  mov     ecx, [eax]
0x006DF370  add     esp, 28h
0x006DF373  push    1
0x006DF375  push    0Eh
0x006DF377  push    eax
0x006DF378  call    dword ptr [ecx+0E4h]
0x006DF37E  mov     ecx, dword_93DEBC
0x006DF384  mov     edx, [ecx]
0x006DF386  mov     esi, ecx
0x006DF388  call    dword ptr [edx+8]
0x006DF38B  mov     eax, [esi+48h]
0x006DF38E  mov     ecx, [eax]
0x006DF390  push    eax
0x006DF391  call    dword ptr [ecx+10Ch]
0x006DF397  mov     dword_982C80, ebx
0x006DF39D  cmp     dword_9017D0, ebx
0x006DF3A3  jz      short loc_6DF409
0x006DF3A5  mov     ecx, dword_93DE78
0x006DF3AB  mov     dword_982C80, ecx
0x006DF3B1  mov     edx, [ecx]
0x006DF3B3  mov     esi, ecx
0x006DF3B5  call    dword ptr [edx+4]
0x006DF3B8  mov     edx, [esi+4Ch]
0x006DF3BB  mov     eax, dword_982BDC
0x006DF3C0  mov     ecx, [eax]
0x006DF3C2  push    edx
0x006DF3C3  push    eax
0x006DF3C4  call    dword ptr [ecx+15Ch]
0x006DF3CA  mov     esi, [esi+48h]
0x006DF3CD  mov     eax, [esi]
0x006DF3CF  push    ebx
0x006DF3D0  lea     ecx, [esp+0AB4h+var_AA4]
0x006DF3D4  push    ecx
0x006DF3D5  push    esi
0x006DF3D6  call    dword ptr [eax+0FCh]
0x006DF3DC  push    ebx
0x006DF3DD  push    offset dword_919650
0x006DF3E2  call    sub_44B190
0x006DF3E7  mov     ecx, dword_93DE78
0x006DF3ED  mov     edx, [ecx]
0x006DF3EF  add     esp, 8
0x006DF3F2  mov     esi, ecx
0x006DF3F4  call    dword ptr [edx+8]
0x006DF3F7  mov     eax, [esi+48h]
0x006DF3FA  mov     ecx, [eax]
0x006DF3FC  push    eax
0x006DF3FD  call    dword ptr [ecx+10Ch]
0x006DF403  mov     dword_982C80, ebx
0x006DF409  mov     ecx, dword_93DEC4
0x006DF40F  mov     eax, [ecx+48h]
0x006DF412  mov     ecx, [ecx+44h]
0x006DF415  mov     ecx, [ecx+18Ch]
0x006DF41B  mov     edx, [eax]
0x006DF41D  push    8
0x006DF41F  push    ecx
0x006DF420  push    eax
0x006DF421  call    dword ptr [edx+68h]
0x006DF424  mov     eax, dword_982BDC
0x006DF429  push    4
0x006DF42B  push    17h
0x006DF42D  mov     byte_982BAD, 1
0x006DF434  mov     edx, [eax]
0x006DF436  push    eax
0x006DF437  call    dword ptr [edx+0E4h]
0x006DF43D  mov     eax, dword_8F92E4
0x006DF442  push    ebx; int
0x006DF443  push    eax; float
0x006DF444  push    offset dword_919650; int
0x006DF449  call    sub_6DE210
0x006DF44E  mov     ecx, dword_93DEC4
0x006DF454  mov     byte_982BAD, bl
0x006DF45A  mov     eax, [ecx+48h]
0x006DF45D  mov     ecx, [ecx+44h]
0x006DF460  mov     ecx, [ecx+18Ch]
0x006DF466  mov     edx, [eax]
0x006DF468  add     esp, 0Ch
0x006DF46B  push    7
0x006DF46D  push    ecx
0x006DF46E  push    eax
0x006DF46F  call    dword ptr [edx+68h]
0x006DF472  cmp     g_ParticleSystemEnable, ebx
0x006DF478  jz      short loc_6DF4DF
0x006DF47A  mov     ecx, dword_93DEC0
0x006DF480  mov     dword_982C80, ecx
0x006DF486  mov     edx, [ecx]
0x006DF488  mov     esi, ecx
0x006DF48A  call    dword ptr [edx+4]
0x006DF48D  mov     edx, [esi+4Ch]
0x006DF490  mov     eax, dword_982BDC
0x006DF495  mov     ecx, [eax]
0x006DF497  push    edx
0x006DF498  push    eax
0x006DF499  call    dword ptr [ecx+15Ch]
0x006DF49F  mov     esi, [esi+48h]
0x006DF4A2  mov     eax, [esi]
0x006DF4A4  push    ebx
0x006DF4A5  lea     ecx, [esp+0AB4h+var_AA4]
0x006DF4A9  push    ecx
0x006DF4AA  push    esi
0x006DF4AB  call    dword ptr [eax+0FCh]
0x006DF4B1  push    offset dword_919650
0x006DF4B6  mov     ecx, offset gEmitterSystem
0x006DF4BB  call    sub_503D00
0x006DF4C0  mov     ecx, dword_93DEC0
0x006DF4C6  mov     edx, [ecx]
0x006DF4C8  mov     esi, ecx
0x006DF4CA  call    dword ptr [edx+8]
0x006DF4CD  mov     eax, [esi+48h]
0x006DF4D0  mov     ecx, [eax]
0x006DF4D2  push    eax
0x006DF4D3  call    dword ptr [ecx+10Ch]
0x006DF4D9  mov     dword_982C80, ebx
0x006DF4DF  cmp     TheGameFlowManager, edi
0x006DF4E5  jnz     loc_6DF57B
0x006DF4EB  cmp     g_ParticleSystemEnable, ebx
0x006DF4F1  jz      loc_6DF57B
0x006DF4F7  mov     ecx, dword_93DEC0
0x006DF4FD  mov     dword_982C80, ecx
0x006DF503  mov     edx, [ecx]
0x006DF505  mov     esi, ecx
0x006DF507  call    dword ptr [edx+4]
0x006DF50A  mov     edx, [esi+4Ch]
0x006DF50D  mov     eax, dword_982BDC
0x006DF512  mov     ecx, [eax]
0x006DF514  push    edx
0x006DF515  push    eax
0x006DF516  call    dword ptr [ecx+15Ch]
0x006DF51C  mov     esi, [esi+48h]
0x006DF51F  mov     eax, [esi]
0x006DF521  push    ebx
0x006DF522  lea     ecx, [esp+0AB4h+var_AA4]
0x006DF526  push    ecx
0x006DF527  push    esi
0x006DF528  call    dword ptr [eax+0FCh]
0x006DF52E  mov     eax, dword_982BDC
0x006DF533  mov     edx, [eax]
0x006DF535  push    ebx
0x006DF536  push    0Eh
0x006DF538  push    eax
0x006DF539  call    dword ptr [edx+0E4h]
0x006DF53F  mov     ecx, dword_9196B8
0x006DF545  call    sub_758100
0x006DF54A  mov     eax, dword_982BDC
0x006DF54F  mov     ecx, [eax]
0x006DF551  push    1
0x006DF553  push    0Eh
0x006DF555  push    eax
0x006DF556  call    dword ptr [ecx+0E4h]
0x006DF55C  mov     ecx, dword_93DEC0
0x006DF562  mov     edx, [ecx]
0x006DF564  mov     esi, ecx
0x006DF566  call    dword ptr [edx+8]
0x006DF569  mov     eax, [esi+48h]
0x006DF56C  mov     ecx, [eax]
0x006DF56E  push    eax
0x006DF56F  call    dword ptr [ecx+10Ch]
0x006DF575  mov     dword_982C80, ebx
0x006DF57B  push    offset dword_919650
0x006DF580  mov     ecx, offset unk_93E018
0x006DF585  call    sub_6D04E0
0x006DF58A  mov     ecx, pVisualTreatmentPlat
0x006DF590  cmp     ecx, ebx
0x006DF592  jz      short loc_6DF59E
0x006DF594  push    offset dword_919650
0x006DF599  call    Update__16IVisualTreatmentP5eView; IVisualTreatment::Update(eView *)
0x006DF59E  mov     ecx, dword_9885B8
0x006DF5A4  mov     al, byte_982C38
0x006DF5A9  inc     ecx
0x006DF5AA  cmp     al, bl
0x006DF5AC  mov     dword_9885B8, ecx
0x006DF5B2  mov     byte_982B8C, bl
0x006DF5B8  jz      short loc_6DF5E7
0x006DF5BA  mov     eax, dword_982BDC
0x006DF5BF  mov     edx, [eax]
0x006DF5C1  push    7
0x006DF5C3  push    0A8h ; '¨'
0x006DF5C8  push    eax
0x006DF5C9  call    dword ptr [edx+0E4h]
0x006DF5CF  mov     eax, dword_982A28
0x006DF5D4  push    ebx
0x006DF5D5  push    1
0x006DF5D7  push    7
0x006DF5D9  push    1
0x006DF5DB  push    0FFFFFFFFh
0x006DF5DD  push    ebx
0x006DF5DE  push    eax
0x006DF5DF  call    sub_6D4100
0x006DF5E4  add     esp, 1Ch
0x006DF5E7  mov     edx, g_OverBrightEnable
0x006DF5ED  mov     eax, TheGameFlowManager
0x006DF5F2  cmp     edx, ebx
0x006DF5F4  setnz   dl
0x006DF5F7  cmp     eax, edi
0x006DF5F9  mov     byte_982B26, dl
0x006DF5FF  jnz     short loc_6DF610
0x006DF601  cmp     dword_90180C, ebx
0x006DF607  mov     byte_982B27, 1
0x006DF60E  jnz     short loc_6DF616
0x006DF610  mov     byte_982B27, bl
0x006DF616  mov     esi, eViews_0
0x006DF61C  cmp     [esi+0Ch], ebx
0x006DF61F  jz      short loc_6DF630
0x006DF621  cmp     dword_901804, ebx
0x006DF627  mov     byte_982B29, 1
0x006DF62E  jnz     short loc_6DF636
0x006DF630  mov     byte_982B29, bl
0x006DF636  cmp     eax, edi
0x006DF638  jnz     short loc_6DF64E
0x006DF63A  cmp     g_RainEnable, ebx
0x006DF640  jz      short loc_6DF64E
0x006DF642  cmp     g_ParticleSystemEnable, ebx
0x006DF648  jz      short loc_6DF64E
0x006DF64A  mov     cl, 1
0x006DF64C  jmp     short loc_6DF650
0x006DF64E  xor     cl, cl
0x006DF650  mov     eax, dword_9017D0
0x006DF655  cmp     eax, ebx
0x006DF657  mov     byte_982B2A, cl
0x006DF65D  jz      short loc_6DF66C
0x006DF65F  cmp     _14FacePixelation$mPixelationOn, bl; FacePixelation::mPixelationOn
0x006DF665  mov     [esp+0AB0h+var_A9D], 1
0x006DF66A  jnz     short loc_6DF670
0x006DF66C  mov     [esp+0AB0h+var_A9D], bl
0x006DF670  cmp     dl, bl
0x006DF672  jnz     short loc_6DF684
0x006DF674  cmp     cl, bl
0x006DF676  jnz     short loc_6DF684
0x006DF678  cmp     dword_901828, ebx
0x006DF67E  jnz     short loc_6DF684
0x006DF680  cmp     eax, ebx
0x006DF682  jz      short loc_6DF696
0x006DF684  call    sub_6DBEC0
0x006DF689  cmp     dword_901828, ebx
0x006DF68F  jz      short loc_6DF696
0x006DF691  call    sub_6DBFE0
0x006DF696  cmp     byte_982B26, bl
0x006DF69C  mov     byte ptr [esp+0AB0h+var_AA4], bl
0x006DF6A0  mov     byte ptr [esp+0AB0h+var_A7C], bl
0x006DF6A4  mov     byte ptr [esp+0AB0h+var_A74], bl
0x006DF6A8  mov     byte ptr [esp+0AB0h+var_A98], bl
0x006DF6AC  mov     byte ptr [esp+0AB0h+var_A78], bl
0x006DF6B0  jz      short loc_6DF6B9
0x006DF6B2  mov     byte ptr [esp+0AB0h+var_AA4], 1
0x006DF6B7  jmp     short loc_6DF6F1
0x006DF6B9  cmp     dword_901800, ebx
0x006DF6BF  jz      short loc_6DF6C8
0x006DF6C1  mov     byte ptr [esp+0AB0h+var_A7C], 1
0x006DF6C6  jmp     short loc_6DF6F1
0x006DF6C8  cmp     dword_901828, ebx
0x006DF6CE  jz      short loc_6DF6D7
0x006DF6D0  mov     byte ptr [esp+0AB0h+var_A98], 1
0x006DF6D5  jmp     short loc_6DF6F1
0x006DF6D7  cmp     byte_982B24, bl
0x006DF6DD  jnz     short loc_6DF6F1
0x006DF6DF  cmp     [esp+0AB0h+var_A9D], bl
0x006DF6E3  jz      short loc_6DF6EC
0x006DF6E5  mov     byte ptr [esp+0AB0h+var_A78], 1
0x006DF6EA  jmp     short loc_6DF6F1
0x006DF6EC  mov     byte ptr [esp+0AB0h+var_A74], 1
0x006DF6F1  cmp     byte_982B30, bl
0x006DF6F7  mov     [esp+0AB0h+var_A9C], 0
0x006DF6FF  jz      short loc_6DF710
0x006DF701  mov     ecx, dword_982B2C
0x006DF707  mov     [esp+0AB0h+var_A9C], ecx
0x006DF70B  jmp     loc_6DF7DF
0x006DF710  cmp     TheGameFlowManager, edi
0x006DF716  mov     ecx, dword_93DEAC
0x006DF71C  jz      short loc_6DF764
0x006DF71E  mov     edx, 0.5
0x006DF723  mov     flt_8F94C0, edx
0x006DF729  mov     flt_8F94C4, edx
0x006DF72F  mov     dword_8F94EC, 0.2
0x006DF739  mov     flt_8F94C8, edx
0x006DF73F  cmp     [ecx+0Ch], ebx
0x006DF742  mov     [esp+0AB0h+var_A9C], 0.80000001
0x006DF74A  jbe     loc_6DF7DF
0x006DF750  mov     [esp+0AB0h+var_A9C], 0.40000001
0x006DF758  mov     dword_8F94EC, 0.14
0x006DF762  jmp     short loc_6DF7BA
0x006DF764  cmp     [esi+24h], ebx
0x006DF767  jz      short loc_6DF7BA
0x006DF769  mov     eax, [esi+0F0h]
0x006DF76F  mov     dword_8F94EC, 0
0x006DF779  mov     edx, [esi+0E0h]
0x006DF77F  mov     [esp+0AB0h+var_A9C], eax
0x006DF783  mov     flt_8F94C0, edx
0x006DF789  mov     eax, [esi+0E4h]
0x006DF78F  mov     flt_8F94C4, eax
0x006DF794  mov     edx, [esi+0E8h]
0x006DF79A  mov     flt_8F94C8, edx
0x006DF7A0  mov     eax, offset flt_8F94C0
0x006DF7A5  fld     dword ptr [eax]
0x006DF7A7  add     eax, 4
0x006DF7AA  cmp     eax, offset unk_8F94CC
0x006DF7AF  fmul    ds:_float_0pt004
0x006DF7B5  fstp    dword ptr [eax-4]
0x006DF7B8  jl      short loc_6DF7A5
0x006DF7BA  cmp     [ecx+0Ch], ebx
0x006DF7BD  jbe     short loc_6DF7DF
0x006DF7BF  fld     [esp+0AB0h+var_A9C]
0x006DF7C3  fmul    ds:_float_0pt5
0x006DF7C9  fstp    [esp+0AB0h+var_A9C]
0x006DF7CD  fld     dword_8F94EC
0x006DF7D3  fmul    ds:flt_89107C
0x006DF7D9  fstp    dword_8F94EC
0x006DF7DF  cmp     byte_982B2A, bl
0x006DF7E5  jz      short loc_6DF7F4
0x006DF7E7  mov     eax, [esp+0AB0h+var_A74]
0x006DF7EB  push    eax
0x006DF7EC  call    sub_6D4620
0x006DF7F1  add     esp, 4
0x006DF7F4  cmp     [esp+0AB0h+var_A9D], bl
0x006DF7F8  jz      short loc_6DF817
0x006DF7FA  mov     ecx, dword_9196BC
0x006DF800  call    sub_73D0C0
0x006DF805  mov     ecx, [esp+0AB0h+var_A78]
0x006DF809  push    ecx
0x006DF80A  push    offset dword_919650
0x006DF80F  call    sub_6D49A0
0x006DF814  add     esp, 8
0x006DF817  cmp     dword_901800, ebx
0x006DF81D  jz      short loc_6DF846
0x006DF81F  fld     flt_8F94E8
0x006DF825  mov     edx, [esp+0AB0h+var_A7C]
0x006DF829  fsub    flt_8F94E4
0x006DF82F  push    edx
0x006DF830  push    ecx
0x006DF831  fmul    [esp+0AB8h+var_A9C]
0x006DF835  fadd    flt_8F94E4
0x006DF83B  fstp    [esp+0AB8h+var_AB8]
0x006DF83E  call    sub_6D3850
0x006DF843  add     esp, 8
0x006DF846  xor     esi, esi
0x006DF848  mov     eax, dword_982BDC
0x006DF84D  mov     ecx, [eax]
0x006DF84F  push    ebx
0x006DF850  push    esi
0x006DF851  push    eax
0x006DF852  call    dword ptr [ecx+104h]
0x006DF858  inc     esi
0x006DF859  cmp     esi, 8
0x006DF85C  jl      short loc_6DF848
0x006DF85E  cmp     dword_901828, ebx
0x006DF864  jz      short loc_6DF882
0x006DF866  mov     ecx, pVisualTreatmentPlat
0x006DF86C  cmp     ecx, ebx
0x006DF86E  jz      short loc_6DF882
0x006DF870  cmp     byte_8F9B5C, bl
0x006DF876  jz      short loc_6DF882
0x006DF878  mov     edx, [esp+0AB0h+var_A98]
0x006DF87C  push    edx
0x006DF87D  call    sub_6D53D0
0x006DF882  cmp     byte_982B26, bl
0x006DF888  jz      short loc_6DF8CB
0x006DF88A  xor     esi, esi
0x006DF88C  lea     esp, [esp+0]
0x006DF890  mov     eax, dword_982BDC
0x006DF895  mov     ecx, [eax]
0x006DF897  push    ebx
0x006DF898  push    esi
0x006DF899  push    eax
0x006DF89A  call    dword ptr [ecx+104h]
0x006DF8A0  inc     esi
0x006DF8A1  cmp     esi, 8
0x006DF8A4  jl      short loc_6DF890
0x006DF8A6  mov     edx, [esp+0AB0h+var_AA4]
0x006DF8AA  push    edx
0x006DF8AB  call    sub_6DC040
0x006DF8B0  add     esp, 4
0x006DF8B3  xor     esi, esi
0x006DF8B5  mov     eax, dword_982BDC
0x006DF8BA  mov     ecx, [eax]
0x006DF8BC  push    ebx
0x006DF8BD  push    esi
0x006DF8BE  push    eax
0x006DF8BF  call    dword ptr [ecx+104h]
0x006DF8C5  inc     esi
0x006DF8C6  cmp     esi, 8
0x006DF8C9  jl      short loc_6DF8B5
0x006DF8CB  pop     edi
0x006DF8CC  pop     esi
0x006DF8CD  pop     ebx
0x006DF8CE  mov     esp, ebp
0x006DF8D0  pop     ebp
0x006DF8D1  retn
