.text:006C6080 ; =============== S U B R O U T I N E =======================================
.text:006C6080
.text:006C6080
.text:006C6080 ; int __thiscall sub_6C6080(_DWORD)
.text:006C6080 sub_6C6080      proc near               ; CODE XREF: sub_6D5C10+12E↓p
.text:006C6080                                         ; sub_6D7500+2B↓p
.text:006C6080
.text:006C6080 var_4           = byte ptr -4
.text:006C6080
.text:006C6080                 push    ecx
.text:006C6081                 push    esi
.text:006C6082                 push    edi
.text:006C6083                 mov     edi, ecx
.text:006C6085                 mov     eax, [edi+48h]
.text:006C6088                 test    eax, eax
.text:006C608A                 lea     esi, [edi+48h]
.text:006C608D                 jz      short loc_6C60A6
.text:006C608F                 mov     ecx, [eax]
.text:006C6091                 push    eax
.text:006C6092                 call    dword ptr [ecx+114h]
.text:006C6098                 mov     eax, [esi]
.text:006C609A                 mov     edx, [eax]
.text:006C609C                 push    eax
.text:006C609D                 call    dword ptr [edx+8]
.text:006C60A0                 mov     dword ptr [esi], 0
.text:006C60A6
.text:006C60A6 loc_6C60A6:                             ; CODE XREF: sub_6C6080+D↑j
.text:006C60A6                 mov     ecx, dword_982C84
.text:006C60AC                 lea     eax, [esp+0Ch+var_4]
.text:006C60B0                 push    eax
.text:006C60B1                 mov     eax, [edi+4]
.text:006C60B4                 push    esi
.text:006C60B5                 push    ecx
.text:006C60B6                 mov     ecx, dword_982BDC
.text:006C60BC                 push    0
.text:006C60BE                 push    0
.text:006C60C0                 lea     edx, [eax+eax*8]
.text:006C60C3                 push    0
.text:006C60C5                 shl     edx, 4
.text:006C60C8                 mov     eax, off_8F9BE8[edx] ; "IDI_WORLD_FX"
.text:006C60CE                 push    eax
.text:006C60CF                 push    0
.text:006C60D1                 push    ecx
.text:006C60D2                 call    D3DXCreateEffectFromResourceA
.text:006C60D7                 mov     ecx, edi
.text:006C60D9                 call    sub_6C5500
.text:006C60DE                 mov     ecx, edi
.text:006C60E0                 call    sub_6C5E60
.text:006C60E5                 pop     edi
.text:006C60E6                 pop     esi
.text:006C60E7                 pop     ecx
.text:006C60E8                 retn
.text:006C60E8 sub_6C6080      endp

.text:006C5500                 sub     esp, 3Ch
.text:006C5503                 push    ebp
.text:006C5504                 push    esi
.text:006C5505                 mov     esi, ecx
.text:006C5507                 mov     eax, [esi+48h]
.text:006C550A                 mov     ecx, [eax]
.text:006C550C                 lea     edx, [esp+44h+var_3C]
.text:006C5510                 push    edx
.text:006C5511                 push    eax
.text:006C5512                 call    dword ptr [ecx+0Ch]
.text:006C5515                 mov     eax, [esp+44h+var_38]
.text:006C5519                 xor     ebp, ebp
.text:006C551B                 test    eax, eax
.text:006C551D                 jbe     short loc_6C5578
.text:006C551F                 push    ebx
.text:006C5520                 push    edi

.text:006C5578 loc_6C5578:                             ; CODE XREF: sub_6C5500+1D↑j
.text:006C5578                 pop     esi
.text:006C5579                 pop     ebp
.text:006C557A                 add     esp, 3Ch
.text:006C557D                 retn
.text:006C557D sub_6C5500      endp

.text:006C5E60 sub_6C5E60      proc near               ; CODE XREF: sub_6C6080+60↓p
.text:006C5E60                 push    esi
.text:006C5E61                 mov     esi, ecx
.text:006C5E63                 mov     eax, [esi+44h]
.text:006C5E66                 mov     ecx, [eax+1DCh]
.text:006C5E6C                 test    ecx, ecx
.text:006C5E6E                 jz      short loc_6C5E7C
.text:006C5E70                 mov     eax, [esi+48h]
.text:006C5E73                 mov     edx, [eax]
.text:006C5E75                 push    1
.text:006C5E77                 push    ecx
.text:006C5E78                 push    eax
.text:006C5E79                 call    dword ptr [edx+68h]
.text:006C5E7C
.text:006C5E7C loc_6C5E7C:                             ; CODE XREF: sub_6C5E60+E↑j
.text:006C5E7C                 mov     eax, [esi+44h]
.text:006C5E7F                 mov     ecx, [eax+9Ch]
.text:006C5E85                 test    ecx, ecx
.text:006C5E87                 jz      short loc_6C5ED5
.text:006C5E89                 mov     dword_93DE20, 0
.text:006C5E93                 mov     dword_93DE24, 0
.text:006C5E9D                 mov     dword_93DE28, 0
.text:006C5EA7                 mov     dword_93DE2C, 5
.text:006C5EB1                 mov     dword_93DE30, 6
.text:006C5EBB                 mov     edx, [esi+44h]
.text:006C5EBE                 mov     edx, [edx+9Ch]
.text:006C5EC4                 mov     eax, [esi+48h]
.text:006C5EC7                 mov     ecx, [eax]
.text:006C5EC9                 push    5
.text:006C5ECB                 push    offset dword_93DE20
.text:006C5ED0                 push    edx
.text:006C5ED1                 push    eax
.text:006C5ED2                 call    dword ptr [ecx+70h]

.data:0093DE20 ; int dword_93DE20
.data:0093DE20 dword_93DE20    dd ?                    ; DATA XREF: sub_6C5E60+29↑w
.data:0093DE20                                         ; sub_6C5E60+6B↑o ...
.data:0093DE24 dword_93DE24    dd ?                    ; DATA XREF: sub_6C5E60+33↑w
.data:0093DE24                                         ; sub_6C67C0+41↑w
.data:0093DE28 dword_93DE28    dd ?                    ; DATA XREF: sub_6C5E60+3D↑w
.data:0093DE28                                         ; sub_6C6810+55↑w
.data:0093DE2C dword_93DE2C    dd ?                    ; DATA XREF: sub_6C5E60+47↑w
.data:0093DE2C                                         ; sub_6C6810+5A↑w
.data:0093DE30 dword_93DE30    dd ?                    ; DATA XREF: sub_6C5E60+51↑w
.data:0093DE30                                         ; sub_6C6810+60↑w
.data:0093DE34                 align 8
.data:0093DE38 ; int dword_93DE38[]