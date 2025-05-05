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



.text:006D5C10
.text:006D5C10 ; =============== S U B R O U T I N E =======================================
.text:006D5C10
.text:006D5C10
.text:006D5C10 ; unsigned int __thiscall sub_6D5C10(int this, int)
.text:006D5C10 sub_6D5C10      proc near               ; CODE XREF: sub_6D6000+3B8↓p
.text:006D5C10
.text:006D5C10 var_B8          = dword ptr -0B8h
.text:006D5C10 var_B4          = dword ptr -0B4h
.text:006D5C10 var_B0          = dword ptr -0B0h
.text:006D5C10 var_AC          = dword ptr -0ACh
.text:006D5C10 var_A8          = dword ptr -0A8h
.text:006D5C10 var_A4          = dword ptr -0A4h
.text:006D5C10 var_A0          = dword ptr -0A0h
.text:006D5C10 var_9C          = byte ptr -9Ch
.text:006D5C10 var_94          = dword ptr -94h
.text:006D5C10 var_8C          = byte ptr -8Ch
.text:006D5C10 var_88          = dword ptr -88h
.text:006D5C10 var_80          = byte ptr -80h
.text:006D5C10 var_7F          = byte ptr -7Fh
.text:006D5C10 var_7E          = byte ptr -7Eh
.text:006D5C10 var_7D          = byte ptr -7Dh
.text:006D5C10 var_7C          = byte ptr -7Ch
.text:006D5C10 var_7B          = byte ptr -7Bh
.text:006D5C10 var_7A          = byte ptr -7Ah
.text:006D5C10 var_79          = byte ptr -79h
.text:006D5C10 arg_0           = dword ptr  4
.text:006D5C10
.text:006D5C10                 sub     esp, 0B8h
.text:006D5C16                 push    ebx
.text:006D5C17                 push    ebp
.text:006D5C18                 push    esi
.text:006D5C19                 mov     esi, ecx
.text:006D5C1B                 mov     eax, [esi+48h]
.text:006D5C1E                 xor     ebx, ebx
.text:006D5C20                 cmp     eax, ebx
.text:006D5C22                 push    edi
.text:006D5C23                 jz      short loc_6D5C2E
.text:006D5C25                 mov     ecx, [eax]
.text:006D5C27                 push    eax
.text:006D5C28                 call    dword ptr [ecx+8]
.text:006D5C2B                 mov     [esi+48h], ebx
.text:006D5C2E
.text:006D5C2E loc_6D5C2E:                             ; CODE XREF: sub_6D5C10+13↑j
.text:006D5C2E                 mov     edx, GPUDevice
.text:006D5C34                 mov     eax, GPUVendor
.text:006D5C39                 mov     ecx, [esp+0C8h+arg_0]
.text:006D5C40                 mov     [esi+4], ecx
.text:006D5C43                 lea     ecx, [ecx+ecx*8]
.text:006D5C46                 shl     ecx, 4
.text:006D5C49                 add     ecx, offset off_8F9B60 ; "WorldShader"
.text:006D5C4F                 mov     [esp+0C8h+var_B0], eax
.text:006D5C53                 lea     eax, [ecx+4]
.text:006D5C56                 mov     [esp+0C8h+var_A8], edx
.text:006D5C5A                 mov     edx, [eax]
.text:006D5C5C                 xor     edi, edi
.text:006D5C5E                 xor     ebp, ebp
.text:006D5C60                 cmp     edx, 8
.text:006D5C63                 jz      loc_6D5D05
.text:006D5C69                 lea     ebx, [esp+0C8h+var_80]
.text:006D5C6D                 sub     ebx, ecx
.text:006D5C6F                 mov     [esp+0C8h+var_B4], ebx
.text:006D5C73                 lea     ebx, [esp+0C8h+var_7F]
.text:006D5C77                 sub     ebx, ecx
.text:006D5C79                 mov     [esp+0C8h+var_A4], ebx
.text:006D5C7D                 lea     ebx, [esp+0C8h+var_7E]
.text:006D5C81                 sub     ebx, ecx
.text:006D5C83                 mov     [esp+0C8h+var_AC], ebx
.text:006D5C87                 lea     ebx, [esp+0C8h+var_7D]
.text:006D5C8B                 sub     ebx, ecx
.text:006D5C8D                 mov     [esp+0C8h+var_B8], ecx
.text:006D5C91                 mov     edx, eax
.text:006D5C93                 mov     [esp+0C8h+var_A0], ebx
.text:006D5C97
.text:006D5C97 loc_6D5C97:                             ; CODE XREF: sub_6D5C10+F1↓j
.text:006D5C97                 mov     ecx, [edx]
.text:006D5C99                 mov     ebx, [esp+0C8h+var_B4]
.text:006D5C9D                 shl     ecx, 2
.text:006D5CA0                 mov     dl, byte_8FAD28[ecx]
.text:006D5CA6                 mov     [ebx+eax], dl
.text:006D5CA9                 mov     edx, [esp+0C8h+var_A4]
.text:006D5CAD                 mov     ebx, [esp+0C8h+var_AC]
.text:006D5CB1                 mov     byte ptr [edx+eax], 0
.text:006D5CB5                 mov     edx, [esp+0C8h+var_B8]
.text:006D5CB9                 mov     edx, [edx+8]
.text:006D5CBC                 mov     [esp+0C8h+var_B8], edx
.text:006D5CC0                 mov     dl, byte_8FAD48[edx*4]
.text:006D5CC7                 mov     [ebx+eax], dl
.text:006D5CCA                 mov     edx, [esp+0C8h+var_B8]
.text:006D5CCE                 mov     dl, byte_8FAD70[edx]
.text:006D5CD4                 mov     ebx, [esp+0C8h+var_A0]
.text:006D5CD8                 mov     [ebx+eax], dl
.text:006D5CDB                 mov     edx, dword_8FAD7C[ecx]
.text:006D5CE1                 add     eax, 8
.text:006D5CE4                 lea     ecx, [eax-4]
.text:006D5CE7                 mov     word ptr [esp+edi*8+0C8h+var_7E], bp
.text:006D5CEC                 add     ebp, edx
.text:006D5CEE                 mov     [esp+0C8h+var_B8], ecx
.text:006D5CF2                 mov     ecx, [eax]
.text:006D5CF4                 mov     word ptr [esp+edi*8+0C8h+var_80], 0
.text:006D5CFB                 inc     edi
.text:006D5CFC                 cmp     ecx, 8
.text:006D5CFF                 mov     edx, eax
.text:006D5D01                 jnz     short loc_6D5C97
.text:006D5D03                 xor     ebx, ebx
.text:006D5D05
.text:006D5D05 loc_6D5D05:                             ; CODE XREF: sub_6D5C10+53↑j
.text:006D5D05                 mov     eax, dword_982BDC
.text:006D5D0A                 lea     ecx, [esi+4Ch]
.text:006D5D0D                 push    ecx
.text:006D5D0E                 lea     ecx, [esp+0CCh+var_80]
.text:006D5D12                 push    ecx
.text:006D5D13                 mov     word ptr [esp+edi*8+0D0h+var_80], 0FFh
.text:006D5D1A                 mov     word ptr [esp+edi*8+0D0h+var_7E], bx
.text:006D5D1F                 mov     [esp+edi*8+0D0h+var_7C], 11h
.text:006D5D24                 mov     [esp+edi*8+0D0h+var_7B], bl
.text:006D5D28                 mov     [esp+edi*8+0D0h+var_7A], bl
.text:006D5D2C                 mov     [esp+edi*8+0D0h+var_79], bl
.text:006D5D30                 mov     edx, [eax]
.text:006D5D32                 push    eax
.text:006D5D33                 call    dword ptr [edx+158h]
.text:006D5D39                 mov     ecx, esi
.text:006D5D3B                 mov     [esi+8], ebp
.text:006D5D3E                 call    sub_6C6080
.text:006D5D43                 mov     eax, [esi+48h]
.text:006D5D46                 lea     ecx, [esp+0C8h+var_9C]
.text:006D5D4A                 push    ecx
.text:006D5D4B                 mov     [esi+10h], ebx
.text:006D5D4E                 mov     edx, [eax]
.text:006D5D50                 push    eax
.text:006D5D51                 call    dword ptr [edx+0Ch]
.text:006D5D54                 mov     eax, [esp+0C8h+var_94]
.text:006D5D58                 xor     ebp, ebp
.text:006D5D5A                 cmp     eax, ebx
.text:006D5D5C                 jbe     loc_6D5FCC
.text:006D5D62
.text:006D5D62 loc_6D5D62:                             ; CODE XREF: sub_6D5C10+382↓j
.text:006D5D62                 mov     eax, [esi+48h]
.text:006D5D65                 mov     edx, [eax]
.text:006D5D67                 push    ebp
.text:006D5D68                 push    eax
.text:006D5D69                 call    dword ptr [edx+30h]
.text:006D5D6C                 mov     edi, eax
.text:006D5D6E                 cmp     edi, ebx
.text:006D5D70                 jz      loc_6D5F8B
.text:006D5D76                 mov     eax, [esi+48h]
.text:006D5D79                 mov     ecx, [eax]
.text:006D5D7B                 push    offset aShader  ; "shader"
.text:006D5D80                 push    edi
.text:006D5D81                 push    eax
.text:006D5D82                 call    dword ptr [ecx+4Ch]
.text:006D5D85                 cmp     eax, ebx
.text:006D5D87                 mov     [esp+0C8h+var_B8], ebx
.text:006D5D8B                 mov     [esp+0C8h+var_B4], ebx
.text:006D5D8F                 jz      short loc_6D5DA2
.text:006D5D91                 mov     ecx, [esi+48h]
.text:006D5D94                 mov     edx, [ecx]
.text:006D5D96                 lea     ebx, [esp+0C8h+var_B8]
.text:006D5D9A                 push    ebx
.text:006D5D9B                 push    eax
.text:006D5D9C                 push    ecx
.text:006D5D9D                 call    dword ptr [edx+6Ch]
.text:006D5DA0                 xor     ebx, ebx
.text:006D5DA2
.text:006D5DA2 loc_6D5DA2:                             ; CODE XREF: sub_6D5C10+17F↑j
.text:006D5DA2                 mov     eax, [esi+48h]
.text:006D5DA5                 mov     ecx, [eax]
.text:006D5DA7                 push    offset aShadowlevel_0 ; "shadowLevel"
.text:006D5DAC                 push    edi
.text:006D5DAD                 push    eax
.text:006D5DAE                 call    dword ptr [ecx+4Ch]
.text:006D5DB1                 cmp     eax, ebx
.text:006D5DB3                 jz      short loc_6D5DC6
.text:006D5DB5                 mov     ecx, [esi+48h]
.text:006D5DB8                 mov     edx, [ecx]
.text:006D5DBA                 lea     ebx, [esp+0C8h+var_B4]
.text:006D5DBE                 push    ebx
.text:006D5DBF                 push    eax
.text:006D5DC0                 push    ecx
.text:006D5DC1                 call    dword ptr [edx+6Ch]
.text:006D5DC4                 xor     ebx, ebx
.text:006D5DC6
.text:006D5DC6 loc_6D5DC6:                             ; CODE XREF: sub_6D5C10+1A3↑j
.text:006D5DC6                 mov     eax, dword_8FAD24
.text:006D5DCB                 cmp     eax, 0FFFFFFFFh
.text:006D5DCE                 mov     ecx, [esp+0C8h+var_B8]
.text:006D5DD2                 jz      short loc_6D5DDC
.text:006D5DD4                 cmp     eax, ecx
.text:006D5DD6                 jl      loc_6D5F8B
.text:006D5DDC
.text:006D5DDC loc_6D5DDC:                             ; CODE XREF: sub_6D5C10+1C2↑j
.text:006D5DDC                 mov     eax, GPUVendor
.text:006D5DE1                 cmp     eax, 10DEh
.text:006D5DE6                 jnz     short loc_6D5E29
.text:006D5DE8                 cmp     dword_93D9E4, 0FFFF0300h
.text:006D5DF2                 jnb     short loc_6D5E4B
.text:006D5DF4                 mov     eax, GPUDevice
.text:006D5DF9                 cmp     eax, 330h
.text:006D5DFE                 jz      short loc_6D5E4B
.text:006D5E00                 cmp     eax, 331h
.text:006D5E05                 jz      short loc_6D5E4B
.text:006D5E07                 cmp     eax, 332h
.text:006D5E0C                 jz      short loc_6D5E4B
.text:006D5E0E                 cmp     eax, 333h
.text:006D5E13                 jz      short loc_6D5E4B
.text:006D5E15                 cmp     eax, 334h
.text:006D5E1A                 jz      short loc_6D5E4B
.text:006D5E1C                 cmp     byte_8FAD9C, bl
.text:006D5E22                 jz      short loc_6D5E4B
.text:006D5E24                 cmp     ecx, 1
.text:006D5E27                 jmp     short loc_6D5E45




.data:008F8108 flt_8F8108      dd 16.0                 ; DATA XREF: sub_659FF0+F5↑r
.data:008F8108                                         ; sub_659FF0+148↑r ...
.data:008F810C                 dd 64.0
.data:008F8110                 dd 256.0
.data:008F8114                 dd 1024.0
.data:008F8118                 dd 4096.0
.data:008F811C                 dd 16384.0
.data:008F8120                 dd 65536.0
.data:008F8124                 dd 0.00024414062
.data:008F8128                 dd 0.0009765625
.data:008F812C                 dd 0.00390625
.data:008F8130                 dd 0.015625
.data:008F8134                 dd 0.0625
.data:008F8138                 dd 0.25
.data:008F813C                 dd 1.0
.data:008F8140                 dd 0.0625
.data:008F8144                 dd 0.015625
.data:008F8148                 dd 0.00390625
.data:008F814C                 dd 0.0009765625
.data:008F8150                 dd 0.00024414062
.data:008F8154                 dd 0.000061035156
.data:008F8158                 dd 0.000015258789
.data:008F815C ; int QueuedFileDefaultPriority
.data:008F815C QueuedFileDefaultPriority dd 5          ; DATA XREF: .text:004AD002↑r
.data:008F815C                                         ; EAXAemsManager::InitiateLoad(void)+6E↑r ...
.data:008F8160 QueuedFileJoylogEnabled db 1            ; DATA XREF: .text:0064CEA4↑w
.data:008F8160                                         ; CheckQueuedFileCallbacks(void)+20↑r
.data:008F8161                 align 4
.data:008F8164 dword_8F8164    dd 1                    ; DATA XREF: sub_6609F0+8F↑r
.data:008F8164                                         ; sub_6609F0+9B↑r ...
.data:008F8168 ; int dword_8F8168
.data:008F8168 dword_8F8168    dd 1                    ; DATA XREF: StartQueuedFileReading(void):loc_661B16↑r
.data:008F816C flt_8F816C      dd 0.80000001           ; DATA XREF: .text:00651169↑r
.data:008F8170                 db 0FFh ; ÿ
.data:008F8171                 db 0FFh ; ÿ
.data:008F8172                 db 0FFh ; ÿ
.data:008F8173                 db 0FFh ; ÿ
.data:008F8174                 db 0FFh ; ÿ
.data:008F8175                 db 0FFh ; ÿ
.data:008F8176                 db 0FFh ; ÿ
.data:008F8177                 db 0FFh ; ÿ
.data:008F8178 dword_8F8178    dd 2                    ; DATA XREF: .text:0065C164↑r
.data:008F817C                 db    3
.data:008F817D                 db    0
.data:008F817E                 db    0
.data:008F817F                 db    0
.data:008F8180                 db    4
.data:008F8181                 db    0
.data:008F8182                 db    0
.data:008F8183                 db    0
.data:008F8184                 db    5
.data:008F8185                 db    0
.data:008F8186                 db    0
.data:008F8187                 db    0
.data:008F8188                 db    6
.data:008F8189                 db    0
.data:008F818A                 db    0
.data:008F818B                 db    0
.data:008F818C                 db    7
.data:008F818D                 db    0
.data:008F818E                 db    0
.data:008F818F                 db    0
.data:008F8190                 db    2
.data:008F8191                 db    0
.data:008F8192                 db    0
.data:008F8193                 db    0
.data:008F8194                 db  60h ; `
.data:008F8195                 db  93h ; “
.data:008F8196                 db  8Fh
.data:008F8197                 db    0
.data:008F8198                 db    0
.data:008F8199                 db    0
.data:008F819A                 db    0
.data:008F819B                 db    0
.data:008F819C                 db    2
.data:008F819D                 db    0
.data:008F819E                 db    0
.data:008F819F                 db    0
.data:008F81A0                 db  48h ; H
.data:008F81A1                 db 0DAh ; Ú
.data:008F81A2                 db  93h ; “
.data:008F81A3                 db    0
.data:008F81A4                 db    1
.data:008F81A5                 db    0
.data:008F81A6                 db    0
.data:008F81A7                 db    0
.data:008F81A8                 db    7
.data:008F81A9                 db    0
.data:008F81AA                 db    0
.data:008F81AB                 db    0
.data:008F81AC                 db    0
.data:008F81AD                 db    0
.data:008F81AE                 db    0
.data:008F81AF                 db    0
.data:008F81B0                 db    0
.data:008F81B1                 db    0
.data:008F81B2                 db    0
.data:008F81B3                 db    0
.data:008F81B4                 db    4
.data:008F81B5                 db    0
.data:008F81B6                 db    0
.data:008F81B7                 db    0
.data:008F81B8                 db  90h
.data:008F81B9                 db 0B2h ; ²
.data:008F81BA                 db  6Ch ; l
.data:008F81BB                 db    0
.data:008F81BC                 db    1
.data:008F81BD                 db    0
.data:008F81BE                 db    0
.data:008F81BF                 db    0
.data:008F81C0                 db    7
.data:008F81C1                 db    0
.data:008F81C2                 db    0
.data:008F81C3                 db    0
.data:008F81C4                 db    0
.data:008F81C5                 db    0
.data:008F81C6                 db    0
.data:008F81C7                 db    0
.data:008F81C8                 db    0
.data:008F81C9                 db    0
.data:008F81CA                 db    0
.data:008F81CB                 db    0
.data:008F81CC                 db    2
.data:008F81CD                 db    0
.data:008F81CE                 db    0
.data:008F81CF                 db    0
.data:008F81D0                 db 0ECh ; ì
.data:008F81D1                 db  29h ; )
.data:008F81D2                 db  98h ; ˜
.data:008F81D3                 db    0
.data:008F81D4                 db    0
.data:008F81D5                 db    0
.data:008F81D6                 db    0
.data:008F81D7                 db    0
.data:008F81D8                 db    7
.data:008F81D9                 db    0
.data:008F81DA                 db    0
.data:008F81DB                 db    0
.data:008F81DC                 db    0
.data:008F81DD                 db    0
.data:008F81DE                 db    0
.data:008F81DF                 db    0
.data:008F81E0                 db    0
.data:008F81E1                 db    0
.data:008F81E2                 db    0
.data:008F81E3                 db    0
.data:008F81E4 unk_8F81E4      db    6                 ; DATA XREF: .data:008F82E0↓o
.data:008F81E5                 db    0
.data:008F81E6                 db    0
.data:008F81E7                 db    0
.data:008F81E8                 db 0CCh ; Ì
.data:008F81E9                 db  81h
.data:008F81EA                 db  8Fh
.data:008F81EB                 db    0
.data:008F81EC                 db    0
.data:008F81ED                 db    0
.data:008F81EE                 db    0
.data:008F81EF                 db    0
.data:008F81F0                 db    7
.data:008F81F1                 db    0
.data:008F81F2                 db    0
.data:008F81F3                 db    0
.data:008F81F4                 db    0
.data:008F81F5                 db    0
.data:008F81F6                 db    0
.data:008F81F7                 db    0
.data:008F81F8                 db    0
.data:008F81F9                 db    0
.data:008F81FA                 db    0
.data:008F81FB                 db    0
.data:008F81FC unk_8F81FC      db    6                 ; DATA XREF: .data:008F82DC↓o
.data:008F81FD                 db    0
.data:008F81FE                 db    0
.data:008F81FF                 db    0
.data:008F8200                 db 0CCh ; Ì
.data:008F8201                 db  81h
.data:008F8202                 db  8Fh
.data:008F8203                 db    0
.data:008F8204                 db    0
.data:008F8205                 db    0
.data:008F8206                 db    0
.data:008F8207                 db    0
.data:008F8208                 db    6
.data:008F8209                 db    0
.data:008F820A                 db    0
.data:008F820B                 db    0
.data:008F820C                 db  90h
.data:008F820D                 db  81h
.data:008F820E                 db  8Fh
.data:008F820F                 db    0
.data:008F8210                 db    0
.data:008F8211                 db    0
.data:008F8212                 db    0
.data:008F8213                 db    0
.data:008F8214                 db    7
.data:008F8215                 db    0
.data:008F8216                 db    0
.data:008F8217                 db    0
.data:008F8218                 db    0
.data:008F8219                 db    0
.data:008F821A                 db    0
.data:008F821B                 db    0
.data:008F821C                 db    0
.data:008F821D                 db    0
.data:008F821E                 db    0
.data:008F821F                 db    0
.data:008F8220 unk_8F8220      db    7                 ; DATA XREF: .text:00887005↑o
.data:008F8221                 db    0
.data:008F8222                 db    0
.data:008F8223                 db    0
.data:008F8224                 db    0
.data:008F8225                 db    0
.data:008F8226                 db    0
.data:008F8227                 db    0
.data:008F8228                 db    0
.data:008F8229                 db    0
.data:008F822A                 db    0
.data:008F822B                 db    0
.data:008F822C unk_8F822C      db    6                 ; DATA XREF: .data:008F82D8↓o
.data:008F822D                 db    0
.data:008F822E                 db    0
.data:008F822F                 db    0
.data:008F8230                 db 0CCh ; Ì
.data:008F8231                 db  84h ; „
.data:008F8232                 db  8Fh
.data:008F8233                 db    0
.data:008F8234                 db    0
.data:008F8235                 db    0
.data:008F8236                 db    0
.data:008F8237                 db    0
.data:008F8238                 db    6
.data:008F8239                 db    0
.data:008F823A                 db    0
.data:008F823B                 db    0
.data:008F823C                 db 0B4h ; ´
.data:008F823D                 db  81h
.data:008F823E                 db  8Fh
.data:008F823F                 db    0
.data:008F8240                 db    0
.data:008F8241                 db    0
.data:008F8242                 db    0
.data:008F8243                 db    0
.data:008F8244                 db    2
.data:008F8245                 db    0
.data:008F8246                 db    0
.data:008F8247                 db    0
.data:008F8248                 db  4Ch ; L
.data:008F8249                 db  59h ; Y
.data:008F824A                 db  92h ; ’
.data:008F824B                 db    0
.data:008F824C                 db    1
.data:008F824D                 db    0
.data:008F824E                 db    0
.data:008F824F                 db    0
.data:008F8250                 db    6
.data:008F8251                 db    0
.data:008F8252                 db    0
.data:008F8253                 db    0
.data:008F8254                 db  20h
.data:008F8255                 db  82h ; ‚
.data:008F8256                 db  8Fh
.data:008F8257                 db    0
.data:008F8258                 db    0
.data:008F8259                 db    0
.data:008F825A                 db    0
.data:008F825B                 db    0
.data:008F825C                 db    7
.data:008F825D                 db    0
.data:008F825E                 db    0
.data:008F825F                 db    0
.data:008F8260                 db    0
.data:008F8261                 db    0
.data:008F8262                 db    0
.data:008F8263                 db    0
.data:008F8264                 db    0
.data:008F8265                 db    0
.data:008F8266                 db    0
.data:008F8267                 db    0
.data:008F8268 unk_8F8268      db    2                 ; DATA XREF: .data:008F82E4↓o
.data:008F8269                 db    0
.data:008F826A                 db    0
.data:008F826B                 db    0
.data:008F826C                 db 0ACh ; ¬
.data:008F826D                 db  3Ah ; :
.data:008F826E                 db  9Ah ; š
.data:008F826F                 db    0
.data:008F8270                 db    1
.data:008F8271                 db    0
.data:008F8272                 db    0
.data:008F8273                 db    0
.data:008F8274                 db    7
.data:008F8275                 db    0
.data:008F8276                 db    0
.data:008F8277                 db    0
.data:008F8278                 db    0
.data:008F8279                 db    0
.data:008F827A                 db    0
.data:008F827B                 db    0
.data:008F827C                 db    0
.data:008F827D                 db    0
.data:008F827E                 db    0
.data:008F827F                 db    0
.data:008F8280 unk_8F8280      db    2                 ; DATA XREF: .data:008F82EC↓o
.data:008F8281                 db    0
.data:008F8282                 db    0
.data:008F8283                 db    0
.data:008F8284                 db  0Ch
.data:008F8285                 db 0D9h ; Ù
.data:008F8286                 db  93h ; “
.data:008F8287                 db    0
.data:008F8288                 db    2
.data:008F8289                 db    0
.data:008F828A                 db    0
.data:008F828B                 db    0
.data:008F828C                 db    2
.data:008F828D                 db    0
.data:008F828E                 db    0
.data:008F828F                 db    0
.data:008F8290                 db    4
.data:008F8291                 db 0DCh ; Ü
.data:008F8292                 db  93h ; “
.data:008F8293                 db    0
.data:008F8294                 db    2
.data:008F8295                 db    0
.data:008F8296                 db    0
.data:008F8297                 db    0
.data:008F8298                 db    2
.data:008F8299                 db    0
.data:008F829A                 db    0
.data:008F829B                 db    0
.data:008F829C                 db 0F4h ; ô
.data:008F829D                 db 0DEh ; Ş
.data:008F829E                 db  93h ; “
.data:008F829F                 db    0
.data:008F82A0                 db    1
.data:008F82A1                 db    0
.data:008F82A2                 db    0
.data:008F82A3                 db    0
.data:008F82A4                 db    2
.data:008F82A5                 db    0
.data:008F82A6                 db    0
.data:008F82A7                 db    0
.data:008F82A8                 db  50h ; P
.data:008F82A9                 db 0DAh ; Ú
.data:008F82AA                 db  93h ; “
.data:008F82AB                 db    0
.data:008F82AC                 db    0
.data:008F82AD                 db    0
.data:008F82AE                 db    0
.data:008F82AF                 db    0
.data:008F82B0                 db    2
.data:008F82B1                 db    0
.data:008F82B2                 db    0
.data:008F82B3                 db    0
.data:008F82B4                 db  58h ; X
.data:008F82B5                 db 0DAh ; Ú
.data:008F82B6                 db  93h ; “
.data:008F82B7                 db    0
.data:008F82B8                 db  80h ; €
.data:008F82B9                 db    0
.data:008F82BA                 db    0
.data:008F82BB                 db    0
.data:008F82BC                 db    7
.data:008F82BD                 db    0
.data:008F82BE                 db    0
.data:008F82BF                 db    0
.data:008F82C0                 db    0
.data:008F82C1                 db    0
.data:008F82C2                 db    0
.data:008F82C3                 db    0
.data:008F82C4                 db    0
.data:008F82C5                 db    0
.data:008F82C6                 db    0
.data:008F82C7                 db    0
.data:008F82C8                 db    7
.data:008F82C9                 db    0
.data:008F82CA                 db    0
.data:008F82CB                 db    0
.data:008F82CC                 db    0
.data:008F82CD                 db    0
.data:008F82CE                 db    0
.data:008F82CF                 db    0
.data:008F82D0                 db    0
.data:008F82D1                 db    0
.data:008F82D2                 db    0
.data:008F82D3                 db    0
.data:008F82D4 dword_8F82D4    dd 0                    ; DATA XREF: .text:00652DF3↑r
.data:008F82D4                                         ; .text:00652E24↑r ...
.data:008F82D8                 dd offset unk_8F822C
.data:008F82DC                 dd offset unk_8F81FC
.data:008F82E0                 dd offset unk_8F81E4
.data:008F82E4                 dd offset unk_8F8268
.data:008F82E8                 dd offset unk_8F84CC
.data:008F82EC                 dd offset unk_8F8280
.data:008F82F0 ; int CurrentVideoMode
.data:008F82F0 CurrentVideoMode dd 2                   ; DATA XREF: GetVideoMode(void)↑r
.data:008F82F0                                         ; SetVideoMode(VIDEO_MODE)+4↑r ...
.data:008F82F4 ; int RealTimeFrames
.data:008F82F4 RealTimeFrames  dd 1                    ; DATA XREF: .text:00466CA7↑r
.data:008F82F4                                         ; .text:00467004↑r ...
.data:008F82F8 ; int RealTime
.data:008F82F8 RealTime        dd 1                    ; DATA XREF: UpdateCameraMovers(float)+75↑r
.data:008F82F8                                         ; AdvanceRealTime(void)+5E↑w
.data:008F82FC ; int RealTimeFramesElapsed
.data:008F82FC RealTimeFramesElapsed dd 2              ; DATA XREF: AdvanceRealTime(void)+53↑w
.data:008F82FC                                         ; .text:00887040↑r
.data:008F8300 DefaultLimitMinimumVideoTimeElapsed dd 0.0083333338
.data:008F8300                                         ; DATA XREF: ResetFPSLimiter(void)↑r
.data:008F8300                                         ; sub_653FF0+6↑r ...
.data:008F8304 ; float MaxTicksPerTimestep
.data:008F8304 MaxTicksPerTimestep dd 4.0              ; DATA XREF: PrepareRealTimestep(float)+58↑r
.data:008F8304                                         ; NISActivity::Play(void):loc_6F578C↑r ...
.data:008F8308 ; float WorldTimeSeconds
.data:008F8308 WorldTimeSeconds dd 1.0                 ; DATA XREF: ResetWorldTime(void)+38↑w
.data:008F8308                                         ; AdvanceWorldTime(void)+B5↑w ...
.data:008F830C ; int WorldTimeFrames
.data:008F830C WorldTimeFrames dd 1                    ; DATA XREF: ResetWorldTime(void)+26↑w
.data:008F830C                                         ; AdvanceWorldTime(void)+69↑w ...
.data:008F8310 ; int WorldTime
.data:008F8310 WorldTime       dd 1                    ; DATA XREF: UpdateTextureAnimations(void):loc_5030F6↑r
.data:008F8310                                         ; ResetWorldTime(void)+31↑w ...
.data:008F8314 ; int WorldLoopCounter
.data:008F8314 WorldLoopCounter dd 100000              ; DATA XREF: ResetWorldTime(void)+4D↑w
.data:008F8314                                         ; AdvanceWorldTime(void)+9A↑r ...
.data:008F8318 ; int NeedToPrepareWorldTimestep
.data:008F8318 NeedToPrepareWorldTimestep dd 1         ; DATA XREF: PrepareWorldTimestep(float)↑r
.data:008F8318                                         ; PrepareWorldTimestep(float)+D↑w ...
.data:008F831C ; int dword_8F831C
.data:008F831C dword_8F831C    dd 1                    ; DATA XREF: sub_41CBF0+5C↑r
.data:008F831C                                         ; sub_41CBF0+6A↑w ...
.data:008F8320                 dd 800000h
.data:008F8324                 dd 7E800000h
.data:008F8328 off_8F8328      dd offset off_8A8C2C    ; DATA XREF: AddVault(char const *,void *,uint)+CD↑o
.data:008F8328                                         ; AddVault(char const *,void *,uint)+17E↑o
.data:008F832C off_8F832C      dd offset off_8A8948    ; DATA XREF: AddVault(char const *,void *,uint)+146↑o
.data:008F8330 off_8F8330      dd offset off_8A8C30    ; DATA XREF: .data:AttribAlloc::mAllocator↑o
.data:008F8334 ; TrackInfo TheRegionLoader
.data:008F8334 TheRegionLoader db    0                 ; DATA XREF: BeginGameFlowLoadRegion(void)↑o
.data:008F8334                                         ; .text:0066725A↑o ...
.data:008F8335                 db    0
.data:008F8336                 db    0
.data:008F8337                 db    0
.data:008F8338 dword_8F8338    dd 9B35FE3Ah            ; DATA XREF: .text:00886D7C↑w
.data:008F833C                 db    0
.data:008F833D                 db    0
.data:008F833E                 db    0
.data:008F833F                 db    0
.data:008F8340                 db    0
.data:008F8341                 db    0
.data:008F8342                 db    0
.data:008F8343                 db    0
.data:008F8344                 db    0
.data:008F8345                 db    0
.data:008F8346                 db    0
.data:008F8347                 db    0
.data:008F8348                 db    0
.data:008F8349                 db    0
.data:008F834A                 db    0
.data:008F834B                 db    0
.data:008F834C TheTrackLoader  db    0                 ; DATA XREF: sub_659630↑o
.data:008F834C                                         ; RedoTopologyAndSceneryGroups(void)+1E↑o ...
.data:008F834D                 db    0
.data:008F834E                 db    0
.data:008F834F                 db    0
.data:008F8350 dword_8F8350    dd 6C0F87BFh            ; DATA XREF: .text:00666FB3↑w
.data:008F8350                                         ; .text:00886D9C↑w
.data:008F8354                 db    0
.data:008F8355                 db    0
.data:008F8356                 db    0
.data:008F8357                 db    0
.data:008F8358                 db    0
.data:008F8359                 db    0
.data:008F835A                 db    0
.data:008F835B                 db    0
.data:008F835C                 db    0
.data:008F835D                 db    0
.data:008F835E                 db    0
.data:008F835F                 db    0
.data:008F8360                 db    0
.data:008F8361                 db    0
.data:008F8362                 db    0
.data:008F8363                 db    0
.data:008F8364                 db    0
.data:008F8365                 db    0
.data:008F8366                 db    0
.data:008F8367                 db    0
.data:008F8368                 db  9Ah ; š
.data:008F8369                 db  99h ; ™
.data:008F836A                 db  99h ; ™
.data:008F836B                 db  3Eh ; >
.data:008F836C                 db    0
.data:008F836D                 db    0
.data:008F836E                 db    0
.data:008F836F                 db    0
.data:008F8370 flt_8F8370      dd -1200.0              ; DATA XREF: ApplyCameraShake(int,bMatrix4 *)+F4↑r
.data:008F8374 flt_8F8374      dd 2000.0               ; DATA XREF: ApplyCameraShake(int,bMatrix4 *)+10D↑r
.data:008F8378 flt_8F8378      dd 0.0                  ; DATA XREF: ApplyCameraShake(int,bMatrix4 *)+132↑r
.data:008F837C                 align 10h
.data:008F8380 flt_8F8380      dd 0.69999999           ; DATA XREF: MaybeCameraShake(int,bVector3 *)+C↑r
.data:008F8384 flt_8F8384      dd 0.69999999           ; DATA XREF: MaybeCameraShake(int,bVector3 *)+19↑r
.data:008F8388 flt_8F8388      dd 1.0                  ; DATA XREF: MaybeCameraShake(int,bVector3 *)+26↑r
.data:008F838C                 db    0
.data:008F838D                 db    0
.data:008F838E                 db    0
.data:008F838F                 db    0
.data:008F8390                 db    0
.data:008F8391                 db  7Fh ; 
.data:008F8392                 db  8Fh
.data:008F8393                 db    0
.data:008F8394                 db    4
.data:008F8395                 db    0
.data:008F8396                 db    0
.data:008F8397                 db    0
.data:008F8398                 db  20h
.data:008F8399                 db  7Fh ; 
.data:008F839A                 db  8Fh
.data:008F839B                 db    0
.data:008F839C                 db    4
.data:008F839D                 db    0
.data:008F839E                 db    0
.data:008F839F                 db    0
.data:008F83A0                 db  40h ; @
.data:008F83A1                 db  7Fh ; 
.data:008F83A2                 db  8Fh
.data:008F83A3                 db    0
.data:008F83A4                 db    3
.data:008F83A5                 db    0
.data:008F83A6                 db    0
.data:008F83A7                 db    0
.data:008F83A8                 db  58h ; X
.data:008F83A9                 db  7Fh ; 
.data:008F83AA                 db  8Fh
.data:008F83AB                 db    0
.data:008F83AC                 db    3
.data:008F83AD                 db    0
.data:008F83AE                 db    0
.data:008F83AF                 db    0
.data:008F83B0                 db  70h ; p
.data:008F83B1                 db  7Fh ; 
.data:008F83B2                 db  8Fh
.data:008F83B3                 db    0
.data:008F83B4                 db    3
.data:008F83B5                 db    0
.data:008F83B6                 db    0
.data:008F83B7                 db    0
.data:008F83B8                 db  88h ; ˆ
.data:008F83B9                 db  7Fh ; 
.data:008F83BA                 db  8Fh
.data:008F83BB                 db    0
.data:008F83BC                 db    2
.data:008F83BD                 db    0
.data:008F83BE                 db    0
.data:008F83BF                 db    0
.data:008F83C0                 db  98h ; ˜
.data:008F83C1                 db  7Fh ; 
.data:008F83C2                 db  8Fh
.data:008F83C3                 db    0
.data:008F83C4                 db    2
.data:008F83C5                 db    0
.data:008F83C6                 db    0
.data:008F83C7                 db    0
.data:008F83C8                 db 0A8h ; ¨
.data:008F83C9                 db  7Fh ; 
.data:008F83CA                 db  8Fh
.data:008F83CB                 db    0
.data:008F83CC                 db    4
.data:008F83CD                 db    0
.data:008F83CE                 db    0
.data:008F83CF                 db    0
.data:008F83D0 off_8F83D0      dd offset flt_8F7FC8    ; DATA XREF: sub_64C1D0+1A↑o
.data:008F83D0                                         ; sub_64C590+1E↑o ...
.data:008F83D4                 db    5
.data:008F83D5                 db    0
.data:008F83D6                 db    0
.data:008F83D7                 db    0
.data:008F83D8                 db 0F0h ; ğ
.data:008F83D9                 db  7Fh ; 
.data:008F83DA                 db  8Fh
.data:008F83DB                 db    0
.data:008F83DC                 db    3
.data:008F83DD                 db    0
.data:008F83DE                 db    0
.data:008F83DF                 db    0
.data:008F83E0                 db    8
.data:008F83E1                 db  80h ; €
.data:008F83E2                 db  8Fh
.data:008F83E3                 db    0
.data:008F83E4                 db    2
.data:008F83E5                 db    0
.data:008F83E6                 db    0
.data:008F83E7                 db    0
.data:008F83E8                 db  18h
.data:008F83E9                 db  80h ; €
.data:008F83EA                 db  8Fh
.data:008F83EB                 db    0
.data:008F83EC                 db    2
.data:008F83ED                 db    0
.data:008F83EE                 db    0
.data:008F83EF                 db    0
.data:008F83F0                 db  28h ; (
.data:008F83F1                 db  80h ; €
.data:008F83F2                 db  8Fh
.data:008F83F3                 db    0
.data:008F83F4                 db    2
.data:008F83F5                 db    0
.data:008F83F6                 db    0
.data:008F83F7                 db    0
.data:008F83F8                 db  38h ; 8
.data:008F83F9                 db  80h ; €
.data:008F83FA                 db  8Fh
.data:008F83FB                 db    0
.data:008F83FC                 db    2
.data:008F83FD                 db    0
.data:008F83FE                 db    0
.data:008F83FF                 db    0
.data:008F8400                 db  48h ; H
.data:008F8401                 db  80h ; €
.data:008F8402                 db  8Fh
.data:008F8403                 db    0
.data:008F8404                 db    3
.data:008F8405                 db    0
.data:008F8406                 db    0
.data:008F8407                 db    0
.data:008F8408 ; int (__thiscall **gEasterEggs)(void *, char)
.data:008F8408 gEasterEggs     dd offset off_8A8AA8    ; DATA XREF: FEPlayerCarDB::AwardBonusCars(void)+2↑o
.data:008F8408                                         ; UnlockSystem::IsEventAvailable(uint)+71↑o ...
.data:008F840C                 align 10h
.data:008F8410 byte_8F8410     db 0                    ; DATA XREF: sub_5A8700:loc_5A8759↑r
.data:008F8411                 align 8
.data:008F8418                 dd offset aBurgerking   ; "burgerking"
.data:008F841C                 db    2
.data:008F841D                 db    0
.data:008F841E                 db    0
.data:008F841F                 db    0
.data:008F8420                 db    0
.data:008F8421                 db    0
.data:008F8422                 db    0
.data:008F8423                 db    0
.data:008F8424                 db    0
.data:008F8425                 db    0
.data:008F8426                 db    0
.data:008F8427                 db    0
.data:008F8428                 db    0
.data:008F8429                 db    0
.data:008F842A                 db    0
.data:008F842B                 db    0
.data:008F842C                 db    0
.data:008F842D                 db    0
.data:008F842E                 db    0
.data:008F842F                 db    0
.data:008F8430                 db    0
.data:008F8431                 db    0
.data:008F8432                 db    0
.data:008F8433                 db    0
.data:008F8434                 db    0
.data:008F8435                 db    0
.data:008F8436                 db    0
.data:008F8437                 db    0
.data:008F8438                 db    0
.data:008F8439                 db    0
.data:008F843A                 db    0
.data:008F843B                 db    0
.data:008F843C                 db    0
.data:008F843D                 db    0
.data:008F843E                 db    0
.data:008F843F                 db    0
.data:008F8440                 db    0
.data:008F8441                 db    0
.data:008F8442                 db    0
.data:008F8443                 db    0
.data:008F8444                 db    0
.data:008F8445                 db    0
.data:008F8446                 db    0
.data:008F8447                 db    0
.data:008F8448                 db    0
.data:008F8449                 db    0
.data:008F844A                 db    1
.data:008F844B                 db    1
.data:008F844C                 db    0
.data:008F844D                 db    0
.data:008F844E                 db    0
.data:008F844F                 db    0
.data:008F8450                 db    0
.data:008F8451                 db    0
.data:008F8452                 db    0
.data:008F8453                 db    0
.data:008F8454                 db    0
.data:008F8455                 db    0
.data:008F8456                 db    0
.data:008F8457                 db    0
.data:008F8458                 db    0
.data:008F8459                 db    0
.data:008F845A                 db    0
.data:008F845B                 db    0
.data:008F845C                 db    0
.data:008F845D                 db    0
.data:008F845E                 db  80h ; €
.data:008F845F                 db  3Fh ; ?
.data:008F8460                 db    0
.data:008F8461                 db    0
.data:008F8462                 db  80h ; €
.data:008F8463                 db  3Fh ; ?
.data:008F8464                 db  66h ; f
.data:008F8465                 db  66h ; f
.data:008F8466                 db  96h ; –
.data:008F8467                 db 0C0h ; À
.data:008F8468                 db    0
.data:008F8469                 db    0
.data:008F846A                 db    0
.data:008F846B                 db    0
.data:008F846C                 db 0CDh ; Í
.data:008F846D                 db 0CCh ; Ì
.data:008F846E                 db 0CCh ; Ì
.data:008F846F                 db  3Fh ; ?
.data:008F8470                 db  66h ; f
.data:008F8471                 db  66h ; f
.data:008F8472                 db  96h ; –
.data:008F8473                 db 0C0h ; À
.data:008F8474                 db    0
.data:008F8475                 db    0
.data:008F8476                 db    0
.data:008F8477                 db    0
.data:008F8478                 db 0CDh ; Í
.data:008F8479                 db 0CCh ; Ì
.data:008F847A                 db 0CCh ; Ì
.data:008F847B                 db  3Fh ; ?
.data:008F847C                 db    0
.data:008F847D                 db    0
.data:008F847E                 db    0
.data:008F847F                 db    0
.data:008F8480                 db    0
.data:008F8481                 db    0
.data:008F8482                 db    0
.data:008F8483                 db    0
.data:008F8484                 db    0
.data:008F8485                 db    0
.data:008F8486                 db    0
.data:008F8487                 db    0
.data:008F8488                 db    0
.data:008F8489                 db    0
.data:008F848A                 db    0
.data:008F848B                 db    0
.data:008F848C                 db    0
.data:008F848D                 db    0
.data:008F848E                 db    0
.data:008F848F                 db    0
.data:008F8490                 db    0
.data:008F8491                 db    0
.data:008F8492                 db    0
.data:008F8493                 db    0
.data:008F8494                 db    0
.data:008F8495                 db    0
.data:008F8496                 db    0
.data:008F8497                 db    0
.data:008F8498                 db    0
.data:008F8499                 db    0
.data:008F849A                 db    0
.data:008F849B                 db    0
.data:008F849C                 db    0
.data:008F849D                 db    0
.data:008F849E                 db 0C0h ; À
.data:008F849F                 db  41h ; A
.data:008F84A0                 db    0
.data:008F84A1                 db    0
.data:008F84A2                 db 0C0h ; À
.data:008F84A3                 db  41h ; A
.data:008F84A4                 db    0
.data:008F84A5                 db    0
.data:008F84A6                 db    0
.data:008F84A7                 db  3Fh ; ?
.data:008F84A8                 db    0
.data:008F84A9                 db    0
.data:008F84AA                 db    0
.data:008F84AB                 db  3Fh ; ?
.data:008F84AC                 db    0
.data:008F84AD                 db    0
.data:008F84AE                 db    0
.data:008F84AF                 db    0
.data:008F84B0                 db    0
.data:008F84B1                 db    0
.data:008F84B2                 db    0
.data:008F84B3                 db    0
.data:008F84B4                 db    0
.data:008F84B5                 db    0
.data:008F84B6                 db    0
.data:008F84B7                 db    0
.data:008F84B8                 db    0
.data:008F84B9                 db    0
.data:008F84BA                 db    0
.data:008F84BB                 db    0
.data:008F84BC                 db    0
.data:008F84BD                 db    0
.data:008F84BE                 db    0
.data:008F84BF                 db    0
.data:008F84C0                 db    0
.data:008F84C1                 db    0
.data:008F84C2                 db    0
.data:008F84C3                 db    0
.data:008F84C4                 db  25h ; %
.data:008F84C5                 db  25h ; %
.data:008F84C6                 db    0
.data:008F84C7                 db    0
.data:008F84C8                 db  64h ; d
.data:008F84C9                 db  64h ; d
.data:008F84CA                 db    0
.data:008F84CB                 db    0
.data:008F84CC unk_8F84CC      db    3                 ; DATA XREF: .data:008F82E8↑o
.data:008F84CD                 db    0
.data:008F84CE                 db    0
.data:008F84CF                 db    0
.data:008F84D0                 db 0B0h ; °
.data:008F84D1                 db  34h ; 4
.data:008F84D2                 db  9Bh ; ›
.data:008F84D3                 db    0
.data:008F84D4 dword_8F84D4    dd 0                    ; DATA XREF: .text:00886FC5↑w
.data:008F84D8 dword_8F84D8    dd 0                    ; DATA XREF: .text:00886FDB↑w
.data:008F84DC dword_8F84DC    dd 0                    ; DATA XREF: .text:00886FE1↑w
.data:008F84E0 dword_8F84E0    dd 0                    ; DATA XREF: .text:00886FCF↑w
.data:008F84E4 dword_8F84E4    dd 0                    ; DATA XREF: .text:00886FEB↑w
.data:008F84E8 dword_8F84E8    dd 0                    ; DATA XREF: .text:00886FF1↑w
.data:008F84EC dword_8F84EC    dd 0                    ; DATA XREF: .text:00886FD4↑w
.data:008F84F0 dword_8F84F0    dd 0                    ; DATA XREF: .text:00886FFB↑w
.data:008F84F4 dword_8F84F4    dd 0                    ; DATA XREF: .text:00887005↑w
.data:008F84F8 dword_8F84F8    dd 0                    ; DATA XREF: .text:0088700F↑w
.data:008F84FC dword_8F84FC    dd 0                    ; DATA XREF: .text:00887014↑w
.data:008F8500 dword_8F8500    dd 0                    ; DATA XREF: .text:0088701E↑w
.data:008F8504 dword_8F8504    dd 0                    ; DATA XREF: .text:00887023↑w
.data:008F8508 timeStep_29593  dd 0.016659999          ; DATA XREF: Main_DisplayFrame(void)+38↑r
.data:008F850C ; char hexChars_33689[]
.data:008F850C hexChars_33689  db '0123456789abcdef',0 ; DATA XREF: MD5::Final(void)+91↑r
.data:008F850C                                         ; MD5::Final(void)+9E↑r
.data:008F851D                 align 10h
.data:008F8520 aBurgerking     db 'burgerking',0       ; DATA XREF: .text:00652F89↑o
.data:008F8520                                         ; .data:008F8418↑o
.data:008F852B                 align 20h
.data:008F8540                 db    3
.data:008F8541                 db    0
.data:008F8542                 db    0
.data:008F8543                 db    0
.data:008F8544                 db    0
.data:008F8545                 db    0
.data:008F8546                 db    0
.data:008F8547                 db    0
.data:008F8548                 db    0
.data:008F8549                 db    0
.data:008F854A                 db    0
.data:008F854B                 db    0
.data:008F854C                 db    0
.data:008F854D                 db    0
.data:008F854E                 db    0
.data:008F854F                 db    0
.data:008F8550                 db    0
.data:008F8551                 db    1
.data:008F8552                 db    1
.data:008F8553                 db    0
.data:008F8554 aCastrol        db 'castrol',0
.data:008F855C                 db    0
.data:008F855D                 db    0
.data:008F855E                 db    0
.data:008F855F                 db    0
.data:008F8560                 db    0
.data:008F8561                 db    0
.data:008F8562                 db    0
.data:008F8563                 db    0
.data:008F8564                 db    0
.data:008F8565                 db    0
.data:008F8566                 db    0
.data:008F8567                 db    0
.data:008F8568                 db    0
.data:008F8569                 db    0
.data:008F856A                 db    0
.data:008F856B                 db    0
.data:008F856C                 db    0
.data:008F856D                 db    0
.data:008F856E                 db    0
.data:008F856F                 db    0
.data:008F8570                 db    0
.data:008F8571                 db    0
.data:008F8572                 db    0
.data:008F8573                 db    0
.data:008F8574                 db    4
.data:008F8575                 db    0
.data:008F8576                 db    0
.data:008F8577                 db    0
.data:008F8578                 db    0
.data:008F8579                 db    0
.data:008F857A                 db    0
.data:008F857B                 db    0
.data:008F857C                 db    0
.data:008F857D                 db    0
.data:008F857E                 db    0
.data:008F857F                 db    0
.data:008F8580                 db    0
.data:008F8581                 db    0
.data:008F8582                 db    0
.data:008F8583                 db    0
.data:008F8584                 db    0
.data:008F8585                 db    1
.data:008F8586                 db    1
.data:008F8587                 db    0
.data:008F8588 dword_8F8588    dd 0F82F0F4Fh           ; DATA XREF: sub_65EF50+17↑r
.data:008F858C                 dd 38E7B70Fh
.data:008F8590                 dd 0E300D24Fh
.data:008F8594                 dd 5801FC8Fh
.data:008F8598 dword_8F8598    dd 0B0A70D0Bh           ; DATA XREF: sub_65EF50+1E↑r
.data:008F859C                 dd 2443B54Bh
.data:008F85A0                 dd 0B30C900Bh
.data:008F85A4                 dd 48DEECBh
.data:008F85A8 dword_8F85A8    dd 0E313A4CEh           ; DATA XREF: sub_65F000+17↑r
.data:008F85AC                 dd 0AB5E30Eh
.data:008F85B0                 dd 0E08B1FCEh
.data:008F85B4                 dd 78EFE78Eh
.data:008F85B8 dword_8F85B8    dd 0EB9FBC0Bh           ; DATA XREF: sub_65F000+1E↑r
.data:008F85BC                 dd 4699A9CBh
.data:008F85C0                 dd 0B0A70D0Bh
.data:008F85C4                 dd 2443B54Bh
.data:008F85C8 dword_8F85C8    dd 1147C30Fh            ; DATA XREF: sub_65F0B0+17↑r
.data:008F85C8                                         ; sub_7BEE20+17↑r
.data:008F85CC                 dd 51906CFh
.data:008F85D0                 dd 0DC3BD18Fh
.data:008F85D4                 dd 0F6B234CFh
.data:008F85D8 dword_8F85D8    dd 167914Bh             ; DATA XREF: sub_65F0B0+1E↑r
.data:008F85D8                                         ; sub_7BEE20+1E↑r
.data:008F85DC                 dd 119148Bh
.data:008F85E0                 dd 0C49B83CBh
.data:008F85E4                 dd 0A216268Bh
.data:008F85E8 dword_8F85E8    dd 2640220Eh            ; DATA XREF: .text:0065E36E↑r
.data:008F85E8                                         ; sub_65F160+17↑r
.data:008F85EC                 db 0CEh ; Î
.data:008F85ED                 db    1
.data:008F85EE                 db  9Fh ; Ÿ
.data:008F85EF                 db  89h ; ‰
.data:008F85F0                 db  8Eh
.data:008F85F1                 db  9Bh ; ›
.data:008F85F2                 db 0E7h ; ç
.data:008F85F3                 db  49h ; I
.data:008F85F4                 db  4Eh ; N
.data:008F85F5                 db  4Ch ; L
.data:008F85F6                 db  11h
.data:008F85F7                 db  59h ; Y
.data:008F85F8 dword_8F85F8    dd 326072CBh            ; DATA XREF: .text:0065E365↑r
.data:008F85F8                                         ; sub_65F160+1E↑r
.data:008F85FC                 db  0Bh
.data:008F85FD                 db  0Bh
.data:008F85FE                 db  3Fh ; ?
.data:008F85FF                 db  89h ; ‰
.data:008F8600                 db  4Bh ; K
.data:008F8601                 db  91h ; ‘
.data:008F8602                 db  67h ; g
.data:008F8603                 db    1
.data:008F8604                 db  8Bh ; ‹
.data:008F8605                 db  14h
.data:008F8606                 db  19h
.data:008F8607                 db    1
.data:008F8608 dword_8F8608    dd 51906CFh             ; DATA XREF: .text:0065E39E↑r
.data:008F8608                                         ; sub_65F210+17↑r
.data:008F860C                 db  8Fh
.data:008F860D                 db 0D1h ; Ñ
.data:008F860E                 db  3Bh ; ;
.data:008F860F                 db 0DCh ; Ü
.data:008F8610                 db 0CFh ; Ï
.data:008F8611                 db  34h ; 4
.data:008F8612                 db 0B2h ; ²
.data:008F8613                 db 0F6h ; ö
.data:008F8614                 db  0Fh
.data:008F8615                 db  7Eh ; ~
.data:008F8616                 db  46h ; F
.data:008F8617                 db 0FFh ; ÿ
.data:008F8618 dword_8F8618    dd 119148Bh             ; DATA XREF: .text:0065E395↑r
.data:008F8618                                         ; sub_65F210+1E↑r
.data:008F861C                 db 0CBh ; Ë
.data:008F861D                 db  83h ; ƒ
.data:008F861E                 db  9Bh ; ›
.data:008F861F                 db 0C4h ; Ä
.data:008F8620                 db  8Bh ; ‹
.data:008F8621                 db  26h ; &
.data:008F8622                 db  16h
.data:008F8623                 db 0A2h ; ¢
.data:008F8624                 db  4Bh ; K
.data:008F8625                 db  6Ch ; l
.data:008F8626                 db 0E2h ; â
.data:008F8627                 db 0F3h ; ó
.data:008F8628 dword_8F8628    dd 26D9A20Fh            ; DATA XREF: sub_65F2C0+17↑r
.data:008F862C                 db  4Fh ; O
.data:008F862D                 db 0AEh ; ®
.data:008F862E                 db  9Bh ; ›
.data:008F862F                 db 0BFh ; ¿
.data:008F8630                 db  8Fh
.data:008F8631                 db 0FBh ; û
.data:008F8632                 db  15h
.data:008F8633                 db  4Ah ; J
.data:008F8634                 db  4Fh ; O
.data:008F8635                 db  0Fh
.data:008F8636                 db  2Fh ; /
.data:008F8637                 db 0F8h ; ø
.data:008F8638 dword_8F8638    dd 32FDE04Bh            ; DATA XREF: sub_65F2C0+1E↑r
.data:008F863C                 db  0Bh
.data:008F863D                 db 0BCh ; ¼
.data:008F863E                 db  9Fh ; Ÿ
.data:008F863F                 db 0EBh ; ë
.data:008F8640                 db 0CBh ; Ë
.data:008F8641                 db 0A9h ; ©
.data:008F8642                 db  99h ; ™
.data:008F8643                 db  46h ; F
.data:008F8644                 db  0Bh
.data:008F8645                 db  0Dh
.data:008F8646                 db 0A7h ; §
.data:008F8647                 db 0B0h ; °
.data:008F8648 dword_8F8648    dd 899F01CEh            ; DATA XREF: sub_65F370+17↑r
.data:008F864C                 db  8Eh
.data:008F864D                 db  9Bh ; ›
.data:008F864E                 db 0E7h ; ç
.data:008F864F                 db  49h ; I
.data:008F8650                 db  4Eh ; N
.data:008F8651                 db  4Ch ; L
.data:008F8652                 db  11h
.data:008F8653                 db  59h ; Y
.data:008F8654                 db  0Eh
.data:008F8655                 db  89h ; ‰
.data:008F8656                 db  3Bh ; ;
.data:008F8657                 db 0CCh ; Ì
.data:008F8658 dword_8F8658    dd 893F0B0Bh            ; DATA XREF: sub_65F370+1E↑r
.data:008F865C                 db  4Bh ; K
.data:008F865D                 db  91h ; ‘
.data:008F865E                 db  67h ; g
.data:008F865F                 db    1
.data:008F8660                 db  8Bh ; ‹
.data:008F8661                 db  14h
.data:008F8662                 db  19h
.data:008F8663                 db    1
.data:008F8664                 db 0CBh ; Ë
.data:008F8665                 db  83h ; ƒ
.data:008F8666                 db  9Bh ; ›
.data:008F8667                 db 0C4h ; Ä
.data:008F8668 dword_8F8668    dd 8B237C8Eh            ; DATA XREF: .text:0065E42E↑r
.data:008F8668                                         ; sub_65F420+17↑r
.data:008F866C                 dd 0D004FFCEh
.data:008F8670                 dd 2E55B08Eh
.data:008F8674                 dd 0E313A4CEh
.data:008F8678 dword_8F8678    dd 0CB03644Bh           ; DATA XREF: .text:0065E425↑r
.data:008F8678                                         ; sub_65F420+1E↑r
.data:008F867C                 dd 0D8A8AD0Bh
.data:008F8680                 dd 32FDE04Bh
.data:008F8684                 dd 0EB9FBC0Bh
.data:008F8688 ; int dword_8F8688
.data:008F8688 dword_8F8688    dd 1                    ; DATA XREF: RegionLoader::Unload(void)+1C5↑r
.data:008F8688                                         ; RegionLoader::Unload(void)+1CD↑w
.data:008F868C BuildVersionMachine dd offset aD1003350 ; "D1003350"
.data:008F8690 BuildVersionChangelistName dd offset a19366919367119
.data:008F8690                                         ; DATA XREF: InitJoylog(void):loc_6607D0↑r
.data:008F8690                                         ; InitJoylog(void):loc_660819↑r
.data:008F8690                                         ; "193669 193671 193675 193772..."
.data:008F8694 ; int BuildVersionChangelistNumber
.data:008F8694 BuildVersionChangelistNumber dd 194010  ; DATA XREF: GetGameVersionNumberStringWithChangelistNumber(char *,int)+19↑r
.data:008F8694                                         ; LobbyCore::Init(void)+4A↑r
.data:008F8698 BuildVersionDate dd offset a113005      ; "11/30/05"
.data:008F869C BuildVersionName dd offset nullString
.data:008F86A0 FullBuildVersionName dd offset nullString
.data:008F86A4 ; int SkipFETrackNumber
.data:008F86A4 SkipFETrackNumber dd 2000               ; DATA XREF: RaceStarter::StartSkipFERace(void)+8↑r
.data:008F86A4                                         ; InitConfig(void)↑r ...
.data:008F86A8 ; char *SkipFEPlayerCar[2]
.data:008F86A8 SkipFEPlayerCar dd offset aBmwm3gtre46  ; DATA XREF: sub_42A810+167↑r
.data:008F86A8                                         ; HudResourceManager::ChooseLoadableTextures(ePlayerHudType,int &,float &)+23↑r ...
.data:008F86A8                                         ; "bmwm3gtre46"
.data:008F86AC ; char *SkipFEPlayer2Car
.data:008F86AC SkipFEPlayer2Car dd offset a911turbo    ; DATA XREF: QuickGame::CreateCars(UMath::Vector3 const &):loc_6F4AD9↑r
.data:008F86AC                                         ; ExtrapolatedCar::State::SpawnVehicle(uint)+F↑r
.data:008F86AC                                         ; "911turbo"
.data:008F86B0 ; char (*SkipFEOpponentPresetRide[2])[4]
.data:008F86B0 SkipFEOpponentPresetRide dd offset nullString
.data:008F86B0                                         ; DATA XREF: GRacerInfo::CreateVehicle(uint)+98↑r
.data:008F86B0                                         ; GRacerInfo::CreateVehicle(uint)+AA↑r
.data:008F86B4                 dd offset nullString
.data:008F86B8 ; int SkipFENumPlayerCars
.data:008F86B8 SkipFENumPlayerCars dd 1                ; DATA XREF: RaceStarter::StartSkipFERace(void)+68↑r
.data:008F86B8                                         ; JoylogConfigItems(void)+24↑r ...
.data:008F86BC ; int SkipFENumLaps
.data:008F86BC SkipFENumLaps   dd 2                    ; DATA XREF: RaceStarter::StartSkipFERace(void)+28↑r
.data:008F86BC                                         ; JoylogConfigItems(void)+4D↑r ...
.data:008F86C0 ; int SkipFEDisableCops
.data:008F86C0 SkipFEDisableCops dd 1                  ; DATA XREF: AICopManager::AICopManager(Sim::Param)+3D5↑r
.data:008F86C0                                         ; sub_5FD820+7C↑r ...
.data:008F86C4 ; int SkipFEPovType1
.data:008F86C4 SkipFEPovType1  dd 2                    ; DATA XREF: RaceStarter::StartSkipFERace(void):loc_57EDAB↑r
.data:008F86C4                                         ; RaceStarter::StartSkipFERace(void)+F6↑r
.data:008F86C8 ; float SkipFEDifficulty
.data:008F86C8 SkipFEDifficulty dd 0.5                 ; DATA XREF: RaceStarter::StartSkipFERace(void):loc_57ECE6↑r
.data:008F86C8                                         ; JoylogConfigItems(void)+BD↑r ...
.data:008F86CC ; int SkipFEDifficulty_0
.data:008F86CC SkipFEDifficulty_0 dd 1                 ; DATA XREF: RaceStarter::StartSkipFERace(void)+5C↑r
.data:008F86CC                                         ; JoylogConfigItems(void)+D0↑r ...
.data:008F86D0 ; int dword_8F86D0
.data:008F86D0 dword_8F86D0    dd 1                    ; DATA XREF: RaceStarter::StartSkipFERace(void)+4A↑r
.data:008F86D4 ; int SkipFEControllerConfig1
.data:008F86D4 SkipFEControllerConfig1 dd 0FFFFFFFFh   ; DATA XREF: RaceStarter::StartSkipFERace(void)+9B↑r
.data:008F86D8 ; int SkipFEControllerConfig2
.data:008F86D8 SkipFEControllerConfig2 dd 0FFFFFFFFh   ; DATA XREF: RaceStarter::StartSkipFERace(void):loc_57ED92↑r
.data:008F86DC                 dd offset nullString
.data:008F86E0 dword_8F86E0    dd 1                    ; DATA XREF: sub_6EE400+D↑r
.data:008F86E0                                         ; LocalPlayer::OnCollision(Sim::Collision::Info const &)+7C↑r
.data:008F86E4 PrecipitationEnable dd 1                ; DATA XREF: sub_6233D0+31↑w
.data:008F86E4                                         ; NISActivity::Release(void)+D7↑w ...
.data:008F86E8 TimeOfDaySwapEnable dd 1                ; DATA XREF: TickOverTimeOfday(void)↑r
.data:008F86EC ; char EnableParticleSystem
.data:008F86EC EnableParticleSystem db 1               ; DATA XREF: sub_4FF190↑r
.data:008F86EC                                         ; .text:004FF4E0↑r ...
.data:008F86ED                 align 10h
.data:008F86F0                 dd 1
.data:008F86F4 DebugCameraNearPlane dd 0.5             ; DATA XREF: DebugWorldCameraMover::DebugWorldCameraMover(int,bVector3 const *,bVector3 const *,JoystickPort)+C9↑r
.data:008F86F8 ; int IsSoundEnabled
.data:008F86F8 IsSoundEnabled  dd 1                    ; DATA XREF: EAXSound::PlayUISoundFX(eMenuSoundTriggers)↑r
.data:008F86F8                                         ; sub_4AE940↑r ...
.data:008F86FC ; int IsAudioStreamingEnabled
.data:008F86FC IsAudioStreamingEnabled dd 1            ; DATA XREF: .text:004B0AC6↑w
.data:008F86FC                                         ; sub_4B0AF0+21↑w ...
.data:008F8700 ; int IsSpeechEnabled
.data:008F8700 IsSpeechEnabled dd 1                    ; DATA XREF: AIPursuit::OnTask(HSIMTASK__ *,float):loc_443F30↑r
.data:008F8700                                         ; .text:004B0AC1↑w ...
.data:008F8704 ; int IsNISAudioEnabled
.data:008F8704 IsNISAudioEnabled dd 1                  ; DATA XREF: .text:004B0ABC↑w
.data:008F8704                                         ; sub_4B0AF0+D↑w ...
.data:008F8708 ; int IsMemcardEnabled
.data:008F8708 IsMemcardEnabled dd 1                   ; DATA XREF: UIOptionsScreen::ShouldShowAutoSave(void):loc_510555↑r
.data:008F8708                                         ; MemoryCard::ShouldDoAutoSave(bool)+3B↑r ...
.data:008F870C ; int IsAutoSaveEnabled
.data:008F870C IsAutoSaveEnabled dd 1                  ; DATA XREF: UIOptionsScreen::ShouldShowAutoSave(void)+2E↑r
.data:008F870C                                         ; MemoryCard::ShouldDoAutoSave(bool)+44↑r ...
.data:008F8710                 dd 1
.data:008F8714                 dd 0FFFFFFFFh
.data:008F8718 ; char IsCollectorsEdition
.data:008F8718 IsCollectorsEdition db 1                ; DATA XREF: GetIsCollectorsEdition(void)↑r
.data:008F8718                                         ; UnlockSystem::IsEventAvailable(uint):loc_576AB9↑r ...
.data:008F8719                 align 4
.data:008F871C                 dd 1
.data:008F8720                 dd 2
.data:008F8724                 dd 1
.data:008F8728                 dd 1
.data:008F872C                 dd offset a10_10_235_61 ; "10.10.235.61"
.data:008F8730                 dd 2694h
.data:008F8734                 dd 800000h
.data:008F8738                 dd 7E800000h
.data:008F873C                 dd 800000h
.data:008F8740                 dd 7E800000h
.data:008F8744                 dd 0
.data:008F8748                 dd 78h
.data:008F874C                 dd 2.0
.data:008F8750                 dd 1.25
.data:008F8754                 dd 2500.0
.data:008F8758                 dd 32h
.data:008F875C                 dd 0Ah
.data:008F8760 flt_8F8760      dd 0.2                  ; DATA XREF: sub_6A7F70+70↑r
.data:008F8760                                         ; sub_6A80F0+70↑r ...
.data:008F8764                 dd 8.0
.data:008F8768                 dd 2.0
.data:008F876C flt_8F876C      dd 0.050000001          ; DATA XREF: InputPlayer::FetchInput(void)+46C↑r
.data:008F876C                                         ; InputPlayer::FetchInput(void)+48D↑r ...
.data:008F8770                 dd 0.15000001
.data:008F8774                 dd 0.1
.data:008F8778                 dd 0.1
.data:008F877C                 dd 4000.0
.data:008F8780                 dd 0.30000001
.data:008F8784                 dd 1.0
.data:008F8788                 dd 78h
.data:008F878C                 dd 1.25
.data:008F8790                 dd 0.050000001
.data:008F8794                 dd 0A0h
.data:008F8798                 dd 0.2
.data:008F879C                 dd 0.1
.data:008F87A0 ; int put_maps
.data:008F87A0 put_maps        dd 0                    ; DATA XREF: FindPartMap(Physics::Upgrades::Type)+8↑o
.data:008F87A0                                         ; .text:0066B888↑o ...
.data:008F87A4                 dd offset aTires        ; "tires"
.data:008F87A8 ; int dword_8F87A8
.data:008F87A8 dword_8F87A8    dd 0BD38D1CAh           ; DATA XREF: FindPartMap(Physics::Upgrades::Type)↑r
.data:008F87A8                                         ; .text:0066B880↑r ...
.data:008F87AC                 dd 75B0F76Bh
.data:008F87B0                 dd 34E73F83h
.data:008F87B4                 dd 0C5860F58h
.data:008F87B8                 dd 0BD38D1CAh
.data:008F87BC                 dd 1
.data:008F87C0                 dd offset aBrakes       ; "brakes"
.data:008F87C4                 dd 36350867h
.data:008F87C8                 dd 4E168120h
.data:008F87CC                 dd 552C3D22h
.data:008F87D0                 dd 56C63B6Fh
.data:008F87D4                 dd 36350867h
.data:008F87D8                 dd 2
.data:008F87DC                 dd offset aChassis      ; "chassis"
.data:008F87E0                 dd 0AFA210F0h
.data:008F87E4                 dd 0F62A062Dh
.data:008F87E8                 dd 7845F9BCh
.data:008F87EC                 dd 0B6495C9Eh
.data:008F87F0                 dd 0AFA210F0h
.data:008F87F4                 dd 3
.data:008F87F8                 dd offset aTransmission ; "transmission"
.data:008F87FC                 dd 7A7A3E5h
.data:008F8800                 dd 860E4531h
.data:008F8804                 dd 170D5554h
.data:008F8808                 dd 25AE629Ah
.data:008F880C                 dd 7A7A3E5h
.data:008F8810                 dd 4
.data:008F8814                 dd offset aEngine       ; "engine"
.data:008F8818                 dd 0F1F5FBC7h
.data:008F881C                 dd 7708DB25h
.data:008F8820                 dd 0B12CCB69h
.data:008F8824                 dd 9206EFD2h
.data:008F8828                 dd 0F1F5FBC7h
.data:008F882C                 dd 5
.data:008F8830                 dd offset aInduction    ; "induction"
.data:008F8834                 dd 0C92A0142h
.data:008F8838                 dd 7202E66Eh
.data:008F883C                 dd 7F440672h
.data:008F8840                 dd 7546359Eh
.data:008F8844                 dd 0C92A0142h
.data:008F8848                 dd 6
.data:008F884C                 dd offset aNos_0        ; "nos"
.data:008F8850                 dd 0B1669F64h
.data:008F8854                 dd 9DC66E81h
.data:008F8858                 dd 8F37BEAEh
.data:008F885C                 dd 452D2634h
.data:008F8860                 dd 0B1669F64h
.data:008F8864                 dd 7
.data:008F8868                 dd 0
.data:008F886C                 dd 0
.data:008F8870                 dd 0
.data:008F8874                 dd 0
.data:008F8878                 dd 0
.data:008F887C                 dd 0
.data:008F8880                 dd 800000h
.data:008F8884                 dd 7E800000h
.data:008F8888                 dd 3C000000h
.data:008F888C                 dd 0
.data:008F8890 dword_8F8890    dd 4                    ; DATA XREF: PVehicle::LookupBehaviorSignature(Attrib::StringKey const &)+2B2↑o
.data:008F8894 dword_8F8894    dd 0                    ; DATA XREF: sub_887610+A↑w
.data:008F8898 dword_8F8898    dd 0                    ; DATA XREF: PVehicle::LookupBehaviorSignature(Attrib::StringKey const &)+2A4↑r
.data:008F8898                                         ; sub_887610+1F↑w
.data:008F889C dword_8F889C    dd 0                    ; DATA XREF: sub_887610+24↑w
.data:008F88A0 dword_8F88A0    dd 0                    ; DATA XREF: sub_887610+2E↑w
.data:008F88A4 dword_8F88A4    dd 0                    ; DATA XREF: sub_887610+44↑w
.data:008F88A8 dword_8F88A8    dd 0                    ; DATA XREF: sub_887610+49↑w
.data:008F88AC dword_8F88AC    dd 0                    ; DATA XREF: sub_887610+53↑w
.data:008F88B0 dword_8F88B0    dd 0                    ; DATA XREF: sub_887610+5E↑w
.data:008F88B4 dword_8F88B4    dd 0                    ; DATA XREF: sub_887610+6D↑w
.data:008F88B8 dword_8F88B8    dd 0                    ; DATA XREF: sub_887610+77↑w
.data:008F88BC dword_8F88BC    dd 0                    ; DATA XREF: sub_887610+86↑w
.data:008F88C0 dword_8F88C0    dd 0                    ; DATA XREF: sub_887610+8B↑w
.data:008F88C4 dword_8F88C4    dd 0                    ; DATA XREF: sub_887610+9F↑w
.data:008F88C8 dword_8F88C8    dd 0                    ; DATA XREF: sub_887610+B4↑w
.data:008F88CC dword_8F88CC    dd 0                    ; DATA XREF: sub_887610+B9↑w
.data:008F88D0 dword_8F88D0    dd 0                    ; DATA XREF: sub_887610+C3↑w
.data:008F88D4 dword_8F88D4    dd 0                    ; DATA XREF: sub_887610+D9↑w
.data:008F88D8 dword_8F88D8    dd 0                    ; DATA XREF: sub_887610+DE↑w
.data:008F88DC dword_8F88DC    dd 0                    ; DATA XREF: sub_887610+E8↑w
.data:008F88E0 dword_8F88E0    dd 0                    ; DATA XREF: sub_887610+F3↑w
.data:008F88E4 dword_8F88E4    dd 0                    ; DATA XREF: sub_887610+102↑w
.data:008F88E8 dword_8F88E8    dd 0                    ; DATA XREF: sub_887610+10C↑w
.data:008F88EC dword_8F88EC    dd 0                    ; DATA XREF: sub_887610+116↑w
.data:008F88F0 dword_8F88F0    dd 0                    ; DATA XREF: sub_887610+123↑w
.data:008F88F4 dword_8F88F4    dd 0                    ; DATA XREF: sub_887610+12D↑w
.data:008F88F8 dword_8F88F8    dd 0                    ; DATA XREF: sub_887610+132↑w
.data:008F88FC off_8F88FC      dd offset nullsub_256   ; DATA XREF: .text:00887936↑o
.data:008F8900                 dd offset nullsub_671
.data:008F8904 dword_8F8904    dd 0                    ; DATA XREF: .text:00887931↑w
.data:008F8908 dword_8F8908    dd 0                    ; DATA XREF: .text:0088791A↑w
.data:008F890C dword_8F890C    dd 0                    ; DATA XREF: .text:00887927↑w
.data:008F8910 off_8F8910      dd offset sub_672B20    ; DATA XREF: .text:00887C26↑o
.data:008F8914                 dd offset sub_672B40
.data:008F8918 dword_8F8918    dd 0                    ; DATA XREF: .text:00887C21↑w
.data:008F891C dword_8F891C    dd 0                    ; DATA XREF: .text:00887C0A↑w
.data:008F8920 dword_8F8920    dd 0                    ; DATA XREF: .text:00887C17↑w
.data:008F8924                 db    0
.data:008F8925                 db    0
.data:008F8926                 db    0
.data:008F8927                 db    0
.data:008F8928                 db    0
.data:008F8929                 db    0
.data:008F892A                 db  80h ; €
.data:008F892B                 db  3Eh ; >
.data:008F892C unk_8F892C      db    0                 ; DATA XREF: PerfStats::Fetch(Attrib::Gen::pvehicle const &,bVector2 *,int *)+5B0↑o
.data:008F892D                 db    0
.data:008F892E                 db 0C0h ; À
.data:008F892F                 db  3Fh ; ?
.data:008F8930 unk_8F8930      db    0                 ; DATA XREF: Physics::Info::EstimatePerformance(Attrib::Gen::pvehicle const &,Physics::Info::Performance &)+98↑o
.data:008F8931                 db    0
.data:008F8932                 db  80h ; €
.data:008F8933                 db  3Eh ; >
.data:008F8934                 db    0
.data:008F8935                 db    0
.data:008F8936                 db    0
.data:008F8937                 db    0
.data:008F8938                 db    0
.data:008F8939                 db    0
.data:008F893A                 db    0
.data:008F893B                 db  3Fh ; ?
.data:008F893C                 db    0
.data:008F893D                 db    0
.data:008F893E                 db    0
.data:008F893F                 db    0
.data:008F8940                 db    0
.data:008F8941                 db    0
.data:008F8942                 db  80h ; €
.data:008F8943                 db  3Eh ; >
.data:008F8944                 db    0
.data:008F8945                 db    0
.data:008F8946                 db  80h ; €
.data:008F8947                 db  3Fh ; ?
.data:008F8948                 db 0CDh ; Í
.data:008F8949                 db 0CCh ; Ì
.data:008F894A                 db  4Ch ; L
.data:008F894B                 db  3Eh ; >
.data:008F894C                 db    0
.data:008F894D                 db    0
.data:008F894E                 db  80h ; €
.data:008F894F                 db  3Fh ; ?
.data:008F8950                 db    0
.data:008F8951                 db    0
.data:008F8952                 db    0
.data:008F8953                 db    0
.data:008F8954                 db    0
.data:008F8955                 db    0
.data:008F8956                 db  40h ; @
.data:008F8957                 db  3Fh ; ?
.data:008F8958                 db    0
.data:008F8959                 db    0
.data:008F895A                 db    0
.data:008F895B                 db  3Fh ; ?
.data:008F895C                 db    0
.data:008F895D                 db    0
.data:008F895E                 db    0
.data:008F895F                 db    0
.data:008F8960                 db    0
.data:008F8961                 db    0
.data:008F8962                 db  80h ; €
.data:008F8963                 db  3Fh ; ?
.data:008F8964                 db    0
.data:008F8965                 db    0
.data:008F8966                 db  80h ; €
.data:008F8967                 db  3Eh ; >
.data:008F8968                 db    0
.data:008F8969                 db    0
.data:008F896A                 db    0
.data:008F896B                 db    0
.data:008F896C                 db    0
.data:008F896D                 db    0
.data:008F896E                 db 0A0h ;  
.data:008F896F                 db  3Fh ; ?
.data:008F8970                 db    0
.data:008F8971                 db    0
.data:008F8972                 db  80h ; €
.data:008F8973                 db  3Eh ; >
.data:008F8974                 db    0
.data:008F8975                 db    0
.data:008F8976                 db    0
.data:008F8977                 db    0
.data:008F8978                 db    0
.data:008F8979                 db    0
.data:008F897A                 db 0C0h ; À
.data:008F897B                 db  3Fh ; ?
.data:008F897C                 db    0
.data:008F897D                 db    0
.data:008F897E                 db    0
.data:008F897F                 db    0
.data:008F8980 flt_8F8980      dd 0.90899998           ; DATA XREF: PerfStats::Fetch(Attrib::Gen::pvehicle const &,bVector2 *,int *)+5CF↑o
.data:008F8984 ; int dword_8F8984
.data:008F8984 dword_8F8984    dd 1.045                ; DATA XREF: Physics::Info::EstimatePerformance(Attrib::Gen::pvehicle const &,Physics::Info::Performance &)+122↑o
.data:008F8988                 dd 1.09
.data:008F898C                 dd 1.09
.data:008F8990                 dd 1.09
.data:008F8994                 dd 1.09
.data:008F8998                 dd 1.09
.data:008F899C                 dd 1.045
.data:008F89A0                 dd 1.0
.data:008F89A4                 dd 1.0
.data:008F89A8                 dd 0.833
.data:008F89AC                 dd 0.958
.data:008F89B0                 dd 1.008
.data:008F89B4                 dd 1.0167
.data:008F89B8                 dd 1.033
.data:008F89BC                 dd 1.033
.data:008F89C0                 dd 1.033
.data:008F89C4                 dd 1.0167
.data:008F89C8                 dd 1.0
.data:008F89CC                 dd 1.0
.data:008F89D0 flt_8F89D0      dd 0.5                  ; DATA XREF: Chassis::DoAerodynamics(Chassis::State const &,float,float,float,float,Physics::Tunings const *)+1D9↑r
.data:008F89D4 flt_8F89D4      dd 0.40000001           ; DATA XREF: Chassis::DoAerodynamics(Chassis::State const &,float,float,float,float,Physics::Tunings const *)+1E4↑r
.data:008F89D4                                         ; Chassis::DoAerodynamics(Chassis::State const &,float,float,float,float,Physics::Tunings const *)+1F3↑r
.data:008F89D8 flt_8F89D8      dd 2.0                  ; DATA XREF: Chassis::DoAerodynamics(Chassis::State const &,float,float,float,float,Physics::Tunings const *)+4B↑r
.data:008F89DC flt_8F89DC      dd -0.1                 ; DATA XREF: Chassis::DoAerodynamics(Chassis::State const &,float,float,float,float,Physics::Tunings const *)+D2↑r
.data:008F89E0                 db    0
.data:008F89E1                 db    0
.data:008F89E2                 db    0
.data:008F89E3                 db    0
.data:008F89E4                 db  9Ah ; š
.data:008F89E5                 db  99h ; ™
.data:008F89E6                 db  99h ; ™
.data:008F89E7                 db  3Fh ; ?
.data:008F89E8                 db  33h ; 3
.data:008F89E9                 db  33h ; 3
.data:008F89EA                 db  13h
.data:008F89EB                 db  40h ; @
.data:008F89EC                 db    0
.data:008F89ED                 db    0
.data:008F89EE                 db  40h ; @
.data:008F89EF                 db  40h ; @
.data:008F89F0                 db    0
.data:008F89F1                 db    0
.data:008F89F2                 db  40h ; @
.data:008F89F3                 db  40h ; @
.data:008F89F4                 db  33h ; 3
.data:008F89F5                 db  33h ; 3
.data:008F89F6                 db  33h ; 3
.data:008F89F7                 db  40h ; @
.data:008F89F8                 db    0
.data:008F89F9                 db    0
.data:008F89FA                 db    0
.data:008F89FB                 db    0
.data:008F89FC                 db  9Ah ; š
.data:008F89FD                 db  99h ; ™
.data:008F89FE                 db 0D9h ; Ù
.data:008F89FF                 db  3Fh ; ?
.data:008F8A00                 db 0CDh ; Í
.data:008F8A01                 db 0CCh ; Ì
.data:008F8A02                 db  4Ch ; L
.data:008F8A03                 db  40h ; @
.data:008F8A04                 db  9Ah ; š
.data:008F8A05                 db  99h ; ™
.data:008F8A06                 db  89h ; ‰
.data:008F8A07                 db  40h ; @
.data:008F8A08                 db  33h ; 3
.data:008F8A09                 db  33h ; 3
.data:008F8A0A                 db 0A3h ; £
.data:008F8A0B                 db  40h ; @
.data:008F8A0C                 db  66h ; f
.data:008F8A0D                 db  66h ; f
.data:008F8A0E                 db 0A6h ; ¦
.data:008F8A0F                 db  40h ; @
.data:008F8A10                 db    0
.data:008F8A11                 db    0
.data:008F8A12                 db    0
.data:008F8A13                 db    0
.data:008F8A14                 db  66h ; f
.data:008F8A15                 db  66h ; f
.data:008F8A16                 db 0E6h ; æ
.data:008F8A17                 db  3Fh ; ?
.data:008F8A18                 db    0
.data:008F8A19                 db    0
.data:008F8A1A                 db  60h ; `
.data:008F8A1B                 db  40h ; @
.data:008F8A1C                 db 0CDh ; Í
.data:008F8A1D                 db 0CCh ; Ì
.data:008F8A1E                 db  9Ch ; œ
.data:008F8A1F                 db  40h ; @
.data:008F8A20                 db  9Ah ; š
.data:008F8A21                 db  99h ; ™
.data:008F8A22                 db 0B9h ; ¹
.data:008F8A23                 db  40h ; @
.data:008F8A24                 db  33h ; 3
.data:008F8A25                 db  33h ; 3
.data:008F8A26                 db 0C3h ; Ã
.data:008F8A27                 db  40h ; @
.data:008F8A28                 db    0
.data:008F8A29                 db    0
.data:008F8A2A                 db    0
.data:008F8A2B                 db    0
.data:008F8A2C                 db  71h ; q
.data:008F8A2D                 db  3Dh ; =
.data:008F8A2E                 db 0EAh ; ê
.data:008F8A2F                 db  3Fh ; ?
.data:008F8A30                 db  66h ; f
.data:008F8A31                 db  66h ; f
.data:008F8A32                 db  66h ; f
.data:008F8A33                 db  40h ; @
.data:008F8A34                 db    0
.data:008F8A35                 db    0
.data:008F8A36                 db 0A0h ;  
.data:008F8A37                 db  40h ; @
.data:008F8A38                 db  52h ; R
.data:008F8A39                 db 0B8h ; ¸
.data:008F8A3A                 db 0BEh ; ¾
.data:008F8A3B                 db  40h ; @
.data:008F8A3C                 db 0CDh ; Í
.data:008F8A3D                 db 0CCh ; Ì
.data:008F8A3E                 db 0CCh ; Ì
.data:008F8A3F                 db  40h ; @
.data:008F8A40                 db    0
.data:008F8A41                 db    0
.data:008F8A42                 db    0
.data:008F8A43                 db    0
.data:008F8A44                 db  7Bh ; {
.data:008F8A45                 db  14h
.data:008F8A46                 db 0EEh ; î
.data:008F8A47                 db  3Fh ; ?
.data:008F8A48                 db 0CDh ; Í
.data:008F8A49                 db 0CCh ; Ì
.data:008F8A4A                 db  6Ch ; l
.data:008F8A4B                 db  40h ; @
.data:008F8A4C                 db  33h ; 3
.data:008F8A4D                 db  33h ; 3
.data:008F8A4E                 db 0A3h ; £
.data:008F8A4F                 db  40h ; @
.data:008F8A50                 db 0F6h ; ö
.data:008F8A51                 db  28h ; (
.data:008F8A52                 db 0C4h ; Ä
.data:008F8A53                 db  40h ; @
.data:008F8A54                 db  66h ; f
.data:008F8A55                 db  66h ; f
.data:008F8A56                 db 0D6h ; Ö
.data:008F8A57                 db  40h ; @
.data:008F8A58                 db    0
.data:008F8A59                 db    0
.data:008F8A5A                 db    0
.data:008F8A5B                 db    0
.data:008F8A5C                 db  33h ; 3
.data:008F8A5D                 db  33h ; 3
.data:008F8A5E                 db 0F3h ; ó
.data:008F8A5F                 db  3Fh ; ?
.data:008F8A60                 db  33h ; 3
.data:008F8A61                 db  33h ; 3
.data:008F8A62                 db  73h ; s
.data:008F8A63                 db  40h ; @
.data:008F8A64                 db  66h ; f
.data:008F8A65                 db  66h ; f
.data:008F8A66                 db 0A6h ; ¦
.data:008F8A67                 db  40h ; @
.data:008F8A68                 db  9Ah ; š
.data:008F8A69                 db  99h ; ™
.data:008F8A6A                 db 0C9h ; É
.data:008F8A6B                 db  40h ; @
.data:008F8A6C                 db  33h ; 3
.data:008F8A6D                 db  33h ; 3
.data:008F8A6E                 db 0E3h ; ã
.data:008F8A6F                 db  40h ; @
.data:008F8A70 off_8F8A70      dd offset unk_8F8E34    ; DATA XREF: sub_68E090:loc_68E113↑r
.data:008F8A74 off_8F8A74      dd offset unk_8F8E48    ; DATA XREF: sub_68E090+99↑r
.data:008F8A78                 dd offset unk_8F8E5C
.data:008F8A7C                 dd offset unk_8F8E70
.data:008F8A80                 dd offset unk_8F8E84
.data:008F8A84                 dd offset unk_8F8E98
.data:008F8A88 off_8F8A88      dd offset unk_8F8EAC    ; DATA XREF: sub_68E090+54↑r
.data:008F8A8C                 db    0
.data:008F8A8D                 db    0
.data:008F8A8E                 db  40h ; @
.data:008F8A8F                 db  41h ; A
.data:008F8A90 flt_8F8A90      dd 4.0                  ; DATA XREF: sub_68E220+1C↑r
.data:008F8A90                                         ; sub_68E370+54↑r ...
.data:008F8A94 flt_8F8A94      dd 10.0                 ; DATA XREF: sub_68E220+31↑r
.data:008F8A94                                         ; sub_68E370+69↑r ...
.data:008F8A98 unk_8F8A98      db 0FEh ; ş             ; DATA XREF: .data:off_8F8AA8↓o
.data:008F8A99                 db 0FFh ; ÿ
.data:008F8A9A                 db 0FFh ; ÿ
.data:008F8A9B                 db 0FFh ; ÿ
.data:008F8A9C                 db 0FEh ; ş
.data:008F8A9D                 db 0FFh ; ÿ
.data:008F8A9E                 db 0FFh ; ÿ
.data:008F8A9F                 db 0FFh ; ÿ
.data:008F8AA0                 db 0FEh ; ş
.data:008F8AA1                 db 0FFh ; ÿ
.data:008F8AA2                 db 0FFh ; ÿ
.data:008F8AA3                 db 0FFh ; ÿ
.data:008F8AA4                 db 0FEh ; ş
.data:008F8AA5                 db 0FFh ; ÿ
.data:008F8AA6                 db 0FFh ; ÿ
.data:008F8AA7                 db 0FFh ; ÿ
.data:008F8AA8 ; void *off_8F8AA8
.data:008F8AA8 off_8F8AA8      dd offset unk_8F8A98    ; DATA XREF: sub_68E090+64↑r
.data:008F8AA8                                         ; sub_68E090+CF↑r ...
.data:008F8AAC flt_8F8AAC      dd 2.0                  ; DATA XREF: sub_69DA90+70A↑r
.data:008F8AB0 flt_8F8AB0      dd 10.0                 ; DATA XREF: sub_68E370+C1↑r
.data:008F8AB0                                         ; sub_690050+58↑r ...
.data:008F8AB4 flt_8F8AB4      dd 1.45                 ; DATA XREF: sub_68E9E0+86↑r
.data:008F8AB8                 db    0
.data:008F8AB9                 db    0
.data:008F8ABA                 db  34h ; 4
.data:008F8ABB                 db  42h ; B
.data:008F8ABC                 db    0
.data:008F8ABD                 db    0
.data:008F8ABE                 db  20h
.data:008F8ABF                 db  42h ; B
.data:008F8AC0                 db    0
.data:008F8AC1                 db    0
.data:008F8AC2                 db 0A0h ;  
.data:008F8AC3                 db  41h ; A
.data:008F8AC4                 db    0
.data:008F8AC5                 db    0
.data:008F8AC6                 db  20h
.data:008F8AC7                 db  41h ; A
.data:008F8AC8                 db    0
.data:008F8AC9                 db    0
.data:008F8ACA                 db 0B0h ; °
.data:008F8ACB                 db  40h ; @
.data:008F8ACC                 db    0
.data:008F8ACD                 db    0
.data:008F8ACE                 db  90h
.data:008F8ACF                 db  40h ; @
.data:008F8AD0                 db    0
.data:008F8AD1                 db    0
.data:008F8AD2                 db  50h ; P
.data:008F8AD3                 db  40h ; @
.data:008F8AD4                 db  9Ah ; š
.data:008F8AD5                 db  99h ; ™
.data:008F8AD6                 db  39h ; 9
.data:008F8AD7                 db  40h ; @
.data:008F8AD8                 db  9Ah ; š
.data:008F8AD9                 db  99h ; ™
.data:008F8ADA                 db  39h ; 9
.data:008F8ADB                 db  40h ; @
.data:008F8ADC                 db  9Ah ; š
.data:008F8ADD                 db  99h ; ™
.data:008F8ADE                 db  39h ; 9
.data:008F8ADF                 db  40h ; @
.data:008F8AE0                 db  9Ah ; š
.data:008F8AE1                 db  99h ; ™
.data:008F8AE2                 db  39h ; 9
.data:008F8AE3                 db  40h ; @
.data:008F8AE4                 db    0
.data:008F8AE5                 db    0
.data:008F8AE6                 db  80h ; €
.data:008F8AE7                 db  3Fh ; ?
.data:008F8AE8                 db    0
.data:008F8AE9                 db    0
.data:008F8AEA                 db  80h ; €
.data:008F8AEB                 db  3Fh ; ?
.data:008F8AEC                 db 0CDh ; Í
.data:008F8AED                 db 0CCh ; Ì
.data:008F8AEE                 db  8Ch ; Œ
.data:008F8AEF                 db  3Fh ; ?
.data:008F8AF0                 db  9Ah ; š
.data:008F8AF1                 db  99h ; ™
.data:008F8AF2                 db  99h ; ™
.data:008F8AF3                 db  3Fh ; ?
.data:008F8AF4                 db    0
.data:008F8AF5                 db    0
.data:008F8AF6                 db 0A0h ;  
.data:008F8AF7                 db  3Fh ; ?
.data:008F8AF8                 db 0CDh ; Í
.data:008F8AF9                 db 0CCh ; Ì
.data:008F8AFA                 db 0ACh ; ¬
.data:008F8AFB                 db  3Fh ; ?
.data:008F8AFC                 db    0
.data:008F8AFD                 db    0
.data:008F8AFE                 db  80h ; €
.data:008F8AFF                 db  3Fh ; ?
.data:008F8B00                 db    0
.data:008F8B01                 db    0
.data:008F8B02                 db  80h ; €
.data:008F8B03                 db  3Fh ; ?
.data:008F8B04                 db    0
.data:008F8B05                 db    0
.data:008F8B06                 db  80h ; €
.data:008F8B07                 db  3Fh ; ?
.data:008F8B08                 db  29h ; )
.data:008F8B09                 db  5Ch ; \
.data:008F8B0A                 db  0Fh
.data:008F8B0B                 db  3Fh ; ?
.data:008F8B0C                 db    0
.data:008F8B0D                 db    0
.data:008F8B0E                 db    0
.data:008F8B0F                 db  3Fh ; ?
.data:008F8B10                 db  33h ; 3
.data:008F8B11                 db  33h ; 3
.data:008F8B12                 db 0B3h ; ³
.data:008F8B13                 db  3Eh ; >
.data:008F8B14                 db  9Ah ; š
.data:008F8B15                 db  99h ; ™
.data:008F8B16                 db  99h ; ™
.data:008F8B17                 db  3Eh ; >
.data:008F8B18                 db  9Ah ; š
.data:008F8B19                 db  99h ; ™
.data:008F8B1A                 db  99h ; ™
.data:008F8B1B                 db  3Eh ; >
.data:008F8B1C                 db  9Ah ; š
.data:008F8B1D                 db  99h ; ™
.data:008F8B1E                 db  99h ; ™
.data:008F8B1F                 db  3Eh ; >
.data:008F8B20                 db  9Ah ; š
.data:008F8B21                 db  99h ; ™
.data:008F8B22                 db  99h ; ™
.data:008F8B23                 db  3Eh ; >
.data:008F8B24                 db    0
.data:008F8B25                 db    0
.data:008F8B26                 db  34h ; 4
.data:008F8B27                 db  42h ; B
.data:008F8B28                 db    0
.data:008F8B29                 db    0
.data:008F8B2A                 db  70h ; p
.data:008F8B2B                 db  41h ; A
.data:008F8B2C                 db    0
.data:008F8B2D                 db    0
.data:008F8B2E                 db  30h ; 0
.data:008F8B2F                 db  41h ; A
.data:008F8B30                 db    0
.data:008F8B31                 db    0
.data:008F8B32                 db    0
.data:008F8B33                 db  41h ; A
.data:008F8B34                 db    0
.data:008F8B35                 db    0
.data:008F8B36                 db 0E0h ; à
.data:008F8B37                 db  40h ; @
.data:008F8B38                 db    0
.data:008F8B39                 db    0
.data:008F8B3A                 db 0E0h ; à
.data:008F8B3B                 db  40h ; @
.data:008F8B3C                 db    0
.data:008F8B3D                 db    0
.data:008F8B3E                 db 0E0h ; à
.data:008F8B3F                 db  40h ; @
.data:008F8B40                 db    0
.data:008F8B41                 db    0
.data:008F8B42                 db 0E0h ; à
.data:008F8B43                 db  40h ; @
.data:008F8B44                 db    0
.data:008F8B45                 db    0
.data:008F8B46                 db 0E0h ; à
.data:008F8B47                 db  40h ; @
.data:008F8B48                 db    0
.data:008F8B49                 db    0
.data:008F8B4A                 db 0E0h ; à
.data:008F8B4B                 db  40h ; @
.data:008F8B4C                 db    0
.data:008F8B4D                 db    0
.data:008F8B4E                 db  80h ; €
.data:008F8B4F                 db  3Fh ; ?
.data:008F8B50                 db  66h ; f
.data:008F8B51                 db  66h ; f
.data:008F8B52                 db  86h ; †
.data:008F8B53                 db  3Fh ; ?
.data:008F8B54                 db 0CDh ; Í
.data:008F8B55                 db 0CCh ; Ì
.data:008F8B56                 db  8Ch ; Œ
.data:008F8B57                 db  3Fh ; ?
.data:008F8B58                 db    0
.data:008F8B59                 db    0
.data:008F8B5A                 db 0C0h ; À
.data:008F8B5B                 db  3Fh ; ?
.data:008F8B5C                 db 0CDh ; Í
.data:008F8B5D                 db 0CCh ; Ì
.data:008F8B5E                 db  0Ch
.data:008F8B5F                 db  40h ; @
.data:008F8B60                 db  66h ; f
.data:008F8B61                 db  66h ; f
.data:008F8B62                 db  46h ; F
.data:008F8B63                 db  40h ; @
.data:008F8B64                 db    0
.data:008F8B65                 db    0
.data:008F8B66                 db  80h ; €
.data:008F8B67                 db  3Fh ; ?
.data:008F8B68                 db  66h ; f
.data:008F8B69                 db  66h ; f
.data:008F8B6A                 db  86h ; †
.data:008F8B6B                 db  3Fh ; ?
.data:008F8B6C                 db 0CDh ; Í
.data:008F8B6D                 db 0CCh ; Ì
.data:008F8B6E                 db  8Ch ; Œ
.data:008F8B6F                 db  3Fh ; ?
.data:008F8B70                 db  9Ah ; š
.data:008F8B71                 db  99h ; ™
.data:008F8B72                 db  99h ; ™
.data:008F8B73                 db  3Fh ; ?
.data:008F8B74                 db  66h ; f
.data:008F8B75                 db  66h ; f
.data:008F8B76                 db 0A6h ; ¦
.data:008F8B77                 db  3Fh ; ?
.data:008F8B78                 db  33h ; 3
.data:008F8B79                 db  33h ; 3
.data:008F8B7A                 db 0B3h ; ³
.data:008F8B7B                 db  3Fh ; ?
.data:008F8B7C                 db    0
.data:008F8B7D                 db    0
.data:008F8B7E                 db    0
.data:008F8B7F                 db    0
.data:008F8B80                 db    0
.data:008F8B81                 db    0
.data:008F8B82                 db  80h ; €
.data:008F8B83                 db 0BFh ; ¿
.data:008F8B84                 db 0A2h ; ¢
.data:008F8B85                 db  45h ; E
.data:008F8B86                 db  36h ; 6
.data:008F8B87                 db 0BFh ; ¿
.data:008F8B88                 db  9Eh
.data:008F8B89                 db 0EFh ; ï
.data:008F8B8A                 db 0E7h ; ç
.data:008F8B8B                 db 0BEh ; ¾
.data:008F8B8C                 db 0D1h ; Ñ
.data:008F8B8D                 db  22h ; "
.data:008F8B8E                 db  9Bh ; ›
.data:008F8B8F                 db 0BEh ; ¾
.data:008F8B90                 db  1Bh
.data:008F8B91                 db  2Fh ; /
.data:008F8B92                 db  5Dh ; ]
.data:008F8B93                 db 0BEh ; ¾
.data:008F8B94                 db  50h ; P
.data:008F8B95                 db  8Dh
.data:008F8B96                 db  17h
.data:008F8B97                 db 0BEh ; ¾
.data:008F8B98                 db  68h ; h
.data:008F8B99                 db  91h ; ‘
.data:008F8B9A                 db 0EDh ; í
.data:008F8B9B                 db 0BDh ; ½
.data:008F8B9C                 db  0Ah
.data:008F8B9D                 db 0D7h ; ×
.data:008F8B9E                 db 0A3h ; £
.data:008F8B9F                 db 0BDh ; ½
.data:008F8BA0                 db  23h ; #
.data:008F8BA1                 db 0DBh ; Û
.data:008F8BA2                 db  79h ; y
.data:008F8BA3                 db 0BDh ; ½
.data:008F8BA4                 db  96h ; –
.data:008F8BA5                 db  43h ; C
.data:008F8BA6                 db  0Bh
.data:008F8BA7                 db 0BDh ; ½
.data:008F8BA8                 db    0
.data:008F8BA9                 db    0
.data:008F8BAA                 db    0
.data:008F8BAB                 db    0
.data:008F8BAC                 db  96h ; –
.data:008F8BAD                 db  43h ; C
.data:008F8BAE                 db  0Bh
.data:008F8BAF                 db  3Dh ; =
.data:008F8BB0                 db  23h ; #
.data:008F8BB1                 db 0DBh ; Û
.data:008F8BB2                 db  79h ; y
.data:008F8BB3                 db  3Dh ; =
.data:008F8BB4                 db  0Ah
.data:008F8BB5                 db 0D7h ; ×
.data:008F8BB6                 db 0A3h ; £
.data:008F8BB7                 db  3Dh ; =
.data:008F8BB8                 db  68h ; h
.data:008F8BB9                 db  91h ; ‘
.data:008F8BBA                 db 0EDh ; í
.data:008F8BBB                 db  3Dh ; =
.data:008F8BBC                 db  50h ; P
.data:008F8BBD                 db  8Dh
.data:008F8BBE                 db  17h
.data:008F8BBF                 db  3Eh ; >
.data:008F8BC0                 db  1Bh
.data:008F8BC1                 db  2Fh ; /
.data:008F8BC2                 db  5Dh ; ]
.data:008F8BC3                 db  3Eh ; >
.data:008F8BC4                 db 0D1h ; Ñ
.data:008F8BC5                 db  22h ; "
.data:008F8BC6                 db  9Bh ; ›
.data:008F8BC7                 db  3Eh ; >
.data:008F8BC8                 db  9Eh
.data:008F8BC9                 db 0EFh ; ï
.data:008F8BCA                 db 0E7h ; ç
.data:008F8BCB                 db  3Eh ; >
.data:008F8BCC                 db 0A2h ; ¢
.data:008F8BCD                 db  45h ; E
.data:008F8BCE                 db  36h ; 6
.data:008F8BCF                 db  3Fh ; ?
.data:008F8BD0                 db    0
.data:008F8BD1                 db    0
.data:008F8BD2                 db  80h ; €
.data:008F8BD3                 db  3Fh ; ?
.data:008F8BD4                 db    0
.data:008F8BD5                 db    0
.data:008F8BD6                 db    0
.data:008F8BD7                 db    0
.data:008F8BD8                 db    0
.data:008F8BD9                 db    0
.data:008F8BDA                 db  80h ; €
.data:008F8BDB                 db 0BFh ; ¿
.data:008F8BDC                 db  7Fh ; 
.data:008F8BDD                 db  6Ah ; j
.data:008F8BDE                 db  3Ch ; <
.data:008F8BDF                 db 0BFh ; ¿
.data:008F8BE0                 db  83h ; ƒ
.data:008F8BE1                 db 0C0h ; À
.data:008F8BE2                 db  0Ah
.data:008F8BE3                 db 0BFh ; ¿
.data:008F8BE4                 db 0CDh ; Í
.data:008F8BE5                 db 0CCh ; Ì
.data:008F8BE6                 db 0CCh ; Ì
.data:008F8BE7                 db 0BEh ; ¾
.data:008F8BE8                 db    6
.data:008F8BE9                 db  81h
.data:008F8BEA                 db  95h ; •
.data:008F8BEB                 db 0BEh ; ¾
.data:008F8BEC                 db 0D1h ; Ñ
.data:008F8BED                 db  22h ; "
.data:008F8BEE                 db  5Bh ; [
.data:008F8BEF                 db 0BEh ; ¾
.data:008F8BF0                 db  0Ah
.data:008F8BF1                 db 0D7h ; ×
.data:008F8BF2                 db  23h ; #
.data:008F8BF3                 db 0BEh ; ¾
.data:008F8BF4                 db  6Dh ; m
.data:008F8BF5                 db 0E7h ; ç
.data:008F8BF6                 db 0FBh ; û
.data:008F8BF7                 db 0BDh ; ½
.data:008F8BF8                 db  77h ; w
.data:008F8BF9                 db 0BEh ; ¾
.data:008F8BFA                 db  9Fh ; Ÿ
.data:008F8BFB                 db 0BDh ; ½
.data:008F8BFC                 db 0BCh ; ¼
.data:008F8BFD                 db  74h ; t
.data:008F8BFE                 db  13h
.data:008F8BFF                 db 0BDh ; ½
.data:008F8C00                 db    0
.data:008F8C01                 db    0
.data:008F8C02                 db    0
.data:008F8C03                 db    0
.data:008F8C04                 db 0BCh ; ¼
.data:008F8C05                 db  74h ; t
.data:008F8C06                 db  13h
.data:008F8C07                 db  3Dh ; =
.data:008F8C08                 db  77h ; w
.data:008F8C09                 db 0BEh ; ¾
.data:008F8C0A                 db  9Fh ; Ÿ
.data:008F8C0B                 db  3Dh ; =
.data:008F8C0C                 db  6Dh ; m
.data:008F8C0D                 db 0E7h ; ç
.data:008F8C0E                 db 0FBh ; û
.data:008F8C0F                 db  3Dh ; =
.data:008F8C10                 db  0Ah
.data:008F8C11                 db 0D7h ; ×
.data:008F8C12                 db  23h ; #
.data:008F8C13                 db  3Eh ; >
.data:008F8C14                 db 0D1h ; Ñ
.data:008F8C15                 db  22h ; "
.data:008F8C16                 db  5Bh ; [
.data:008F8C17                 db  3Eh ; >
.data:008F8C18                 db    6
.data:008F8C19                 db  81h
.data:008F8C1A                 db  95h ; •
.data:008F8C1B                 db  3Eh ; >
.data:008F8C1C                 db 0CDh ; Í
.data:008F8C1D                 db 0CCh ; Ì
.data:008F8C1E                 db 0CCh ; Ì
.data:008F8C1F                 db  3Eh ; >
.data:008F8C20                 db  83h ; ƒ
.data:008F8C21                 db 0C0h ; À
.data:008F8C22                 db  0Ah
.data:008F8C23                 db  3Fh ; ?
.data:008F8C24                 db  7Fh ; 
.data:008F8C25                 db  6Ah ; j
.data:008F8C26                 db  3Ch ; <
.data:008F8C27                 db  3Fh ; ?
.data:008F8C28                 db    0
.data:008F8C29                 db    0
.data:008F8C2A                 db  80h ; €
.data:008F8C2B                 db  3Fh ; ?
.data:008F8C2C                 db    0
.data:008F8C2D                 db    0
.data:008F8C2E                 db    0
.data:008F8C2F                 db    0
.data:008F8C30                 db    0
.data:008F8C31                 db    0
.data:008F8C32                 db  80h ; €
.data:008F8C33                 db 0BFh ; ¿
.data:008F8C34                 db 0CDh ; Í
.data:008F8C35                 db 0CCh ; Ì
.data:008F8C36                 db  4Ch ; L
.data:008F8C37                 db 0BFh ; ¿
.data:008F8C38                 db 0A4h ; ¤
.data:008F8C39                 db  70h ; p
.data:008F8C3A                 db  1Dh
.data:008F8C3B                 db 0BFh ; ¿
.data:008F8C3C                 db 0C7h ; Ç
.data:008F8C3D                 db  4Bh ; K
.data:008F8C3E                 db 0F7h ; ÷
.data:008F8C3F                 db 0BEh ; ¾
.data:008F8C40                 db 0F0h ; ğ
.data:008F8C41                 db 0A7h ; §
.data:008F8C42                 db 0C6h ; Æ
.data:008F8C43                 db 0BEh ; ¾
.data:008F8C44                 db 0BCh ; ¼
.data:008F8C45                 db  74h ; t
.data:008F8C46                 db  93h ; “
.data:008F8C47                 db 0BEh ; ¾
.data:008F8C48                 db 0AEh ; ®
.data:008F8C49                 db  47h ; G
.data:008F8C4A                 db  61h ; a
.data:008F8C4B                 db 0BEh ; ¾
.data:008F8C4C                 db  2Fh ; /
.data:008F8C4D                 db 0DDh ; İ
.data:008F8C4E                 db  24h ; $
.data:008F8C4F                 db 0BEh ; ¾
.data:008F8C50                 db 0F8h ; ø
.data:008F8C51                 db  53h ; S
.data:008F8C52                 db 0E3h ; ã
.data:008F8C53                 db 0BDh ; ½
.data:008F8C54                 db 0D5h ; Õ
.data:008F8C55                 db  78h ; x
.data:008F8C56                 db  69h ; i
.data:008F8C57                 db 0BDh ; ½
.data:008F8C58                 db    0
.data:008F8C59                 db    0
.data:008F8C5A                 db    0
.data:008F8C5B                 db    0
.data:008F8C5C                 db 0D5h ; Õ
.data:008F8C5D                 db  78h ; x
.data:008F8C5E                 db  69h ; i
.data:008F8C5F                 db  3Dh ; =
.data:008F8C60                 db 0F8h ; ø
.data:008F8C61                 db  53h ; S
.data:008F8C62                 db 0E3h ; ã
.data:008F8C63                 db  3Dh ; =
.data:008F8C64                 db  2Fh ; /
.data:008F8C65                 db 0DDh ; İ
.data:008F8C66                 db  24h ; $
.data:008F8C67                 db  3Eh ; >
.data:008F8C68                 db 0AEh ; ®
.data:008F8C69                 db  47h ; G
.data:008F8C6A                 db  61h ; a
.data:008F8C6B                 db  3Eh ; >
.data:008F8C6C                 db 0BCh ; ¼
.data:008F8C6D                 db  74h ; t
.data:008F8C6E                 db  93h ; “
.data:008F8C6F                 db  3Eh ; >
.data:008F8C70                 db 0F0h ; ğ
.data:008F8C71                 db 0A7h ; §
.data:008F8C72                 db 0C6h ; Æ
.data:008F8C73                 db  3Eh ; >
.data:008F8C74                 db 0C7h ; Ç
.data:008F8C75                 db  4Bh ; K
.data:008F8C76                 db 0F7h ; ÷
.data:008F8C77                 db  3Eh ; >
.data:008F8C78                 db 0A4h ; ¤
.data:008F8C79                 db  70h ; p
.data:008F8C7A                 db  1Dh
.data:008F8C7B                 db  3Fh ; ?
.data:008F8C7C                 db 0CDh ; Í
.data:008F8C7D                 db 0CCh ; Ì
.data:008F8C7E                 db  4Ch ; L
.data:008F8C7F                 db  3Fh ; ?
.data:008F8C80                 db    0
.data:008F8C81                 db    0
.data:008F8C82                 db  80h ; €
.data:008F8C83                 db  3Fh ; ?
.data:008F8C84                 db    0
.data:008F8C85                 db    0
.data:008F8C86                 db    0
.data:008F8C87                 db    0
.data:008F8C88                 db    0
.data:008F8C89                 db    0
.data:008F8C8A                 db  80h ; €
.data:008F8C8B                 db 0BFh ; ¿
.data:008F8C8C                 db    0
.data:008F8C8D                 db    0
.data:008F8C8E                 db  80h ; €
.data:008F8C8F                 db 0BFh ; ¿
.data:008F8C90                 db 0C5h ; Å
.data:008F8C91                 db  20h
.data:008F8C92                 db  30h ; 0
.data:008F8C93                 db 0BFh ; ¿
.data:008F8C94                 db  6Dh ; m
.data:008F8C95                 db 0E7h ; ç
.data:008F8C96                 db 0FBh ; û
.data:008F8C97                 db 0BEh ; ¾
.data:008F8C98                 db 0F8h ; ø
.data:008F8C99                 db  53h ; S
.data:008F8C9A                 db 0A3h ; £
.data:008F8C9B                 db 0BEh ; ¾
.data:008F8C9C                 db 0D5h ; Õ
.data:008F8C9D                 db  78h ; x
.data:008F8C9E                 db  69h ; i
.data:008F8C9F                 db 0BEh ; ¾
.data:008F8CA0                 db  0Ah
.data:008F8CA1                 db 0D7h ; ×
.data:008F8CA2                 db  23h ; #
.data:008F8CA3                 db 0BEh ; ¾
.data:008F8CA4                 db  6Dh ; m
.data:008F8CA5                 db 0E7h ; ç
.data:008F8CA6                 db 0FBh ; û
.data:008F8CA7                 db 0BDh ; ½
.data:008F8CA8                 db  7Bh ; {
.data:008F8CA9                 db  14h
.data:008F8CAA                 db 0AEh ; ®
.data:008F8CAB                 db 0BDh ; ½
.data:008F8CAC                 db 0CDh ; Í
.data:008F8CAD                 db 0CCh ; Ì
.data:008F8CAE                 db  4Ch ; L
.data:008F8CAF                 db 0BDh ; ½
.data:008F8CB0                 db    0
.data:008F8CB1                 db    0
.data:008F8CB2                 db    0
.data:008F8CB3                 db    0
.data:008F8CB4                 db 0CDh ; Í
.data:008F8CB5                 db 0CCh ; Ì
.data:008F8CB6                 db  4Ch ; L
.data:008F8CB7                 db  3Dh ; =
.data:008F8CB8                 db  7Bh ; {
.data:008F8CB9                 db  14h
.data:008F8CBA                 db 0AEh ; ®
.data:008F8CBB                 db  3Dh ; =
.data:008F8CBC                 db  6Dh ; m
.data:008F8CBD                 db 0E7h ; ç
.data:008F8CBE                 db 0FBh ; û
.data:008F8CBF                 db  3Dh ; =
.data:008F8CC0                 db  0Ah
.data:008F8CC1                 db 0D7h ; ×
.data:008F8CC2                 db  23h ; #
.data:008F8CC3                 db  3Eh ; >
.data:008F8CC4                 db 0D5h ; Õ
.data:008F8CC5                 db  78h ; x
.data:008F8CC6                 db  69h ; i
.data:008F8CC7                 db  3Eh ; >
.data:008F8CC8                 db 0F8h ; ø
.data:008F8CC9                 db  53h ; S
.data:008F8CCA                 db 0A3h ; £
.data:008F8CCB                 db  3Eh ; >
.data:008F8CCC                 db  6Dh ; m
.data:008F8CCD                 db 0E7h ; ç
.data:008F8CCE                 db 0FBh ; û
.data:008F8CCF                 db  3Eh ; >
.data:008F8CD0                 db 0C5h ; Å
.data:008F8CD1                 db  20h
.data:008F8CD2                 db  30h ; 0
.data:008F8CD3                 db  3Fh ; ?
.data:008F8CD4                 db    0
.data:008F8CD5                 db    0
.data:008F8CD6                 db  80h ; €
.data:008F8CD7                 db  3Fh ; ?
.data:008F8CD8                 db    0
.data:008F8CD9                 db    0
.data:008F8CDA                 db  80h ; €
.data:008F8CDB                 db  3Fh ; ?
.data:008F8CDC unk_8F8CDC      db    0                 ; DATA XREF: .data:off_8F8FA8↓o
.data:008F8CDD                 db    0
.data:008F8CDE                 db    0
.data:008F8CDF                 db    0
.data:008F8CE0                 db    0
.data:008F8CE1                 db    0
.data:008F8CE2                 db  80h ; €
.data:008F8CE3                 db  3Fh ; ?
.data:008F8CE4                 db    0
.data:008F8CE5                 db    0
.data:008F8CE6                 db 0A0h ;  
.data:008F8CE7                 db  40h ; @
.data:008F8CE8                 db 0CDh ; Í
.data:008F8CE9                 db 0CCh ; Ì
.data:008F8CEA                 db  4Ch ; L
.data:008F8CEB                 db  3Fh ; ?
.data:008F8CEC                 db    0
.data:008F8CED                 db    0
.data:008F8CEE                 db  10h
.data:008F8CEF                 db  41h ; A
.data:008F8CF0                 db  66h ; f
.data:008F8CF1                 db  66h ; f
.data:008F8CF2                 db  66h ; f
.data:008F8CF3                 db  3Fh ; ?
.data:008F8CF4                 db  9Ah ; š
.data:008F8CF5                 db  99h ; ™
.data:008F8CF6                 db  49h ; I
.data:008F8CF7                 db  41h ; A
.data:008F8CF8                 db  7Dh ; }
.data:008F8CF9                 db  3Fh ; ?
.data:008F8CFA                 db  55h ; U
.data:008F8CFB                 db  3Fh ; ?
.data:008F8CFC                 db 0CDh ; Í
.data:008F8CFD                 db 0CCh ; Ì
.data:008F8CFE                 db  88h ; ˆ
.data:008F8CFF                 db  41h ; A
.data:008F8D00                 db 0ECh ; ì
.data:008F8D01                 db  51h ; Q
.data:008F8D02                 db  38h ; 8
.data:008F8D03                 db  3Fh ; ?
.data:008F8D04                 db    0
.data:008F8D05                 db    0
.data:008F8D06                 db 0C8h ; È
.data:008F8D07                 db  41h ; A
.data:008F8D08                 db  66h ; f
.data:008F8D09                 db  66h ; f
.data:008F8D0A                 db  26h ; &
.data:008F8D0B                 db  3Fh ; ?
.data:008F8D0C flt_8F8D0C      dd 0.5                  ; DATA XREF: SuspensionRacer::Burnout::Update(float,float,float,int,float)+4B↑r
.data:008F8D10 flt_8F8D10      dd 0.5                  ; DATA XREF: SuspensionRacer::Burnout::Update(float,float,float,int,float):loc_69EAFA↑r
.data:008F8D14 flt_8F8D14      dd 1.0                  ; DATA XREF: SuspensionRacer::Burnout::Update(float,float,float,int,float)+62↑r
.data:008F8D18 flt_8F8D18      dd 20.0                 ; DATA XREF: SuspensionRacer::Burnout::Update(float,float,float,int,float):loc_69EB75↑r
.data:008F8D1C dword_8F8D1C    dd 40000000h            ; DATA XREF: SuspensionRacer::Burnout::Update(float,float,float,int,float)+A8↑r
.data:008F8D1C                                         ; SuspensionRacer::Burnout::Update(float,float,float,int,float)+11A↑r
.data:008F8D20 dword_8F8D20    dd 6                    ; DATA XREF: SuspensionRacer::Burnout::Update(float,float,float,int,float)+109↑r
.data:008F8D24 ; float EBrakeYawControlMin
.data:008F8D24 EBrakeYawControlMin dd 0.5              ; DATA XREF: SuspensionRacer::TuneWheelParams(Chassis::State &)+56↑r
.data:008F8D24                                         ; SuspensionRacer::TuneWheelParams(Chassis::State &)+65↑r
.data:008F8D28 ; float EBrakeYawControlOnSpeed
.data:008F8D28 EBrakeYawControlOnSpeed dd 1.0          ; DATA XREF: SuspensionRacer::TuneWheelParams(Chassis::State &):loc_6A9BAD↑r
.data:008F8D2C ; float EBrakeYawControlOffSpeed
.data:008F8D2C EBrakeYawControlOffSpeed dd 20.0        ; DATA XREF: SuspensionRacer::TuneWheelParams(Chassis::State &)+4C↑r
.data:008F8D30 ; float flt_8F8D30
.data:008F8D30 flt_8F8D30      dd 0.30000001           ; DATA XREF: SuspensionRacer::TuneWheelParams(Chassis::State &)+23F↑r
.data:008F8D34 ; float flt_8F8D34
.data:008F8D34 flt_8F8D34      dd 80.0                 ; DATA XREF: SuspensionRacer::TuneWheelParams(Chassis::State &)+250↑r
.data:008F8D38 ; float HighSpeedSpeed
.data:008F8D38 HighSpeedSpeed  dd 30.0                 ; DATA XREF: YawFrictionBoost(float,float,float,float,float)+47↑r
.data:008F8D3C ; float MaxYawBonus
.data:008F8D3C MaxYawBonus     dd 0.34999999           ; DATA XREF: YawFrictionBoost(float,float,float,float,float)+6F↑r
.data:008F8D3C                                         ; YawFrictionBoost(float,float,float,float,float)+7E↑r
.data:008F8D40 ; float HighSpeedYawBoost
.data:008F8D40 HighSpeedYawBoost dd 1.0                ; DATA XREF: YawFrictionBoost(float,float,float,float,float)+55↑r
.data:008F8D44 ; float YawEBrakeThreshold
.data:008F8D44 YawEBrakeThreshold dd 0.5               ; DATA XREF: YawFrictionBoost(float,float,float,float,float)+16↑r
.data:008F8D48 ; float YawAngleThreshold
.data:008F8D48 YawAngleThreshold dd 20.0               ; DATA XREF: YawFrictionBoost(float,float,float,float,float)+23↑r
.data:008F8D4C aFf@            db 'ff¶@',0
.data:008F8D51                 align 2
.data:008F8D52 aA?_3           db '€?',0
.data:008F8D55                 align 2
.data:008F8D56                 db 0C0h ; À
.data:008F8D57                 db  3Fh ; ?
.data:008F8D58                 db 0CDh ; Í
.data:008F8D59                 db 0CCh ; Ì
.data:008F8D5A                 db  8Ch ; Œ
.data:008F8D5B                 db  3Fh ; ?
.data:008F8D5C                 db  33h ; 3
.data:008F8D5D                 db  33h ; 3
.data:008F8D5E                 db  73h ; s
.data:008F8D5F                 db  3Fh ; ?
.data:008F8D60                 db  52h ; R
.data:008F8D61                 db 0B8h ; ¸
.data:008F8D62                 db  5Eh ; ^
.data:008F8D63                 db  3Fh ; ?
.data:008F8D64                 db 0B8h ; ¸
.data:008F8D65                 db  1Eh
.data:008F8D66                 db  45h ; E
.data:008F8D67                 db  3Fh ; ?
.data:008F8D68                 db  1Fh
.data:008F8D69                 db  85h ; …
.data:008F8D6A                 db  2Bh ; +
.data:008F8D6B                 db  3Fh ; ?
.data:008F8D6C                 db  9Ah ; š
.data:008F8D6D                 db  99h ; ™
.data:008F8D6E                 db  19h
.data:008F8D6F                 db  3Fh ; ?
.data:008F8D70                 db  5Ch ; \
.data:008F8D71                 db  8Fh
.data:008F8D72                 db    2
.data:008F8D73                 db  3Fh ; ?
.data:008F8D74                 db 0F6h ; ö
.data:008F8D75                 db  28h ; (
.data:008F8D76                 db 0DCh ; Ü
.data:008F8D77                 db  3Eh ; >
.data:008F8D78                 db 0A4h ; ¤
.data:008F8D79                 db  70h ; p
.data:008F8D7A                 db 0BDh ; ½
.data:008F8D7B                 db  3Eh ; >
.data:008F8D7C                 db  7Bh ; {
.data:008F8D7D                 db  14h
.data:008F8D7E                 db 0AEh ; ®
.data:008F8D7F                 db  3Eh ; >
.data:008F8D80                 db    0
.data:008F8D81                 db    0
.data:008F8D82                 db    0
.data:008F8D83                 db    0
.data:008F8D84                 db 0CDh ; Í
.data:008F8D85                 db 0CCh ; Ì
.data:008F8D86                 db 0CCh ; Ì
.data:008F8D87                 db  3Eh ; >
.data:008F8D88                 db 0CDh ; Í
.data:008F8D89                 db 0CCh ; Ì
.data:008F8D8A                 db  4Ch ; L
.data:008F8D8B                 db  3Fh ; ?
.data:008F8D8C                 db  9Ah ; š
.data:008F8D8D                 db  99h ; ™
.data:008F8D8E                 db  99h ; ™
.data:008F8D8F                 db  3Fh ; ?
.data:008F8D90                 db 0CDh ; Í
.data:008F8D91                 db 0CCh ; Ì
.data:008F8D92                 db 0CCh ; Ì
.data:008F8D93                 db  3Fh ; ?
.data:008F8D94                 db    0
.data:008F8D95                 db    0
.data:008F8D96                 db 0E0h ; à
.data:008F8D97                 db  3Fh ; ?
.data:008F8D98                 db  33h ; 3
.data:008F8D99                 db  33h ; 3
.data:008F8D9A                 db 0D3h ; Ó
.data:008F8D9B                 db  3Fh ; ?
.data:008F8D9C ClutchStiffness dd 20.0                 ; DATA XREF: EngineRacer::OnTaskSimulate(float):loc_6AFB96↑r
.data:008F8D9C                                         ; EngineRacer::OnTaskSimulate(float)+686↑r
.data:008F8DA0 unk_8F8DA0      db    0                 ; DATA XREF: .data:off_8F8FD8↓o
.data:008F8DA1                 db    0
.data:008F8DA2                 db  20h
.data:008F8DA3                 db 0C1h ; Á
.data:008F8DA4                 db    0
.data:008F8DA5                 db    0
.data:008F8DA6                 db  80h ; €
.data:008F8DA7                 db  3Fh ; ?
.data:008F8DA8                 db    0
.data:008F8DA9                 db    0
.data:008F8DAA                 db 0F0h ; ğ
.data:008F8DAB                 db 0C0h ; À
.data:008F8DAC                 db  8Fh
.data:008F8DAD                 db 0C2h ; Â
.data:008F8DAE                 db  75h ; u
.data:008F8DAF                 db  3Fh ; ?
.data:008F8DB0                 db    0
.data:008F8DB1                 db    0
.data:008F8DB2                 db  60h ; `
.data:008F8DB3                 db 0C0h ; À
.data:008F8DB4                 db 0CDh ; Í
.data:008F8DB5                 db 0CCh ; Ì
.data:008F8DB6                 db  6Ch ; l
.data:008F8DB7                 db  3Fh ; ?
.data:008F8DB8                 db  9Ah ; š
.data:008F8DB9                 db  99h ; ™
.data:008F8DBA                 db  99h ; ™
.data:008F8DBB                 db 0BEh ; ¾
.data:008F8DBC                 db    0
.data:008F8DBD                 db    0
.data:008F8DBE                 db  60h ; `
.data:008F8DBF                 db  3Fh ; ?
.data:008F8DC0                 db 0CDh ; Í
.data:008F8DC1                 db 0CCh ; Ì
.data:008F8DC2                 db  4Ch ; L
.data:008F8DC3                 db 0BDh ; ½
.data:008F8DC4                 db    0
.data:008F8DC5                 db    0
.data:008F8DC6                 db    0
.data:008F8DC7                 db    0
.data:008F8DC8 ClutchLimiter   dd 300.0                ; DATA XREF: EngineRacer::OnTaskSimulate(float)+422↑r
.data:008F8DC8                                         ; EngineRacer::OnTaskSimulate(float)+431↑r ...
.data:008F8DCC flt_8F8DCC      dd 80.0                 ; DATA XREF: AIVehicleHelicopter::OnDriving(float)+22D↑r
.data:008F8DCC                                         ; AIVehicleHelicopter::OnDriving(float)+23A↑r ...
.data:008F8DD0 flt_8F8DD0      dd 30.0                 ; DATA XREF: AIVehicleHelicopter::OnDriving(float)+223↑r
.data:008F8DD0                                         ; SimpleChopper::OnTaskSimulate(float)+14F↑r
.data:008F8DD4 flt_8F8DD4      dd 2.0                  ; DATA XREF: AIVehicleHelicopter::OnDriving(float):loc_417C63↑r
.data:008F8DD4                                         ; SimpleChopper::OnTaskSimulate(float)+1D8↑r ...
.data:008F8DD8                 dd offset sub_800000
.data:008F8DDC                 db    0
.data:008F8DDD                 db    0
.data:008F8DDE                 db  80h ; €
.data:008F8DDF                 db  7Eh ; ~
.data:008F8DE0 off_8F8DE0      dd offset nullsub_257   ; DATA XREF: .text:00887DF6↑o
.data:008F8DE4                 dd offset nullsub_672
.data:008F8DE8 dword_8F8DE8    dd 0                    ; DATA XREF: .text:00887DF1↑w
.data:008F8DEC dword_8F8DEC    dd 0                    ; DATA XREF: .text:00887DDA↑w
.data:008F8DF0 dword_8F8DF0    dd 0                    ; DATA XREF: .text:00887DE7↑w
.data:008F8DF4 unk_8F8DF4      db  0Ah                 ; DATA XREF: sub_68D640+36↑o
.data:008F8DF5                 db    0
.data:008F8DF6                 db    0
.data:008F8DF7                 db    0
.data:008F8DF8                 db    0
.data:008F8DF9                 db    0
.data:008F8DFA                 db    0
.data:008F8DFB                 db    0
.data:008F8DFC                 db    0
.data:008F8DFD                 db    0
.data:008F8DFE                 db  80h ; €
.data:008F8DFF                 db  3Fh ; ?
.data:008F8E00                 db    0
.data:008F8E01                 db    0
.data:008F8E02                 db  10h
.data:008F8E03                 db  41h ; A
.data:008F8E04                 db  80h ; €
.data:008F8E05                 db  89h ; ‰
.data:008F8E06                 db  8Fh
.data:008F8E07                 db    0
.data:008F8E08 unk_8F8E08      db  0Ah                 ; DATA XREF: sub_68D5F0+36↑o
.data:008F8E09                 db    0
.data:008F8E0A                 db    0
.data:008F8E0B                 db    0
.data:008F8E0C                 db    0
.data:008F8E0D                 db    0
.data:008F8E0E                 db    0
.data:008F8E0F                 db    0
.data:008F8E10                 db    0
.data:008F8E11                 db    0
.data:008F8E12                 db  80h ; €
.data:008F8E13                 db  3Fh ; ?
.data:008F8E14                 db    0
.data:008F8E15                 db    0
.data:008F8E16                 db  10h
.data:008F8E17                 db  41h ; A
.data:008F8E18                 db 0A8h ; ¨
.data:008F8E19                 db  89h ; ‰
.data:008F8E1A                 db  8Fh
.data:008F8E1B                 db    0
.data:008F8E1C unk_8F8E1C      db    0                 ; DATA XREF: .text:008884A2↑o
.data:008F8E1D                 db    0
.data:008F8E1E                 db    0
.data:008F8E1F                 db    0
.data:008F8E20                 db    0
.data:008F8E21                 db    0
.data:008F8E22                 db    0
.data:008F8E23                 db    0
.data:008F8E24                 db 0CDh ; Í
.data:008F8E25                 db 0CCh ; Ì
.data:008F8E26                 db 0CCh ; Ì
.data:008F8E27                 db  3Eh ; >
.data:008F8E28                 db  9Ah ; š
.data:008F8E29                 db  99h ; ™
.data:008F8E2A                 db  19h
.data:008F8E2B                 db  3Eh ; >
.data:008F8E2C                 db    0
.data:008F8E2D                 db    0
.data:008F8E2E                 db    0
.data:008F8E2F                 db  40h ; @
.data:008F8E30                 db    0
.data:008F8E31                 db    0
.data:008F8E32                 db 0A0h ;  
.data:008F8E33                 db  40h ; @
.data:008F8E34 unk_8F8E34      db    6                 ; DATA XREF: .data:off_8F8A70↑o
.data:008F8E35                 db    0
.data:008F8E36                 db    0
.data:008F8E37                 db    0
.data:008F8E38                 db    0
.data:008F8E39                 db    0
.data:008F8E3A                 db    0
.data:008F8E3B                 db    0
.data:008F8E3C                 db    0
.data:008F8E3D                 db    0
.data:008F8E3E                 db  20h
.data:008F8E3F                 db  41h ; A
.data:008F8E40                 db    0
.data:008F8E41                 db    0
.data:008F8E42                 db    0
.data:008F8E43                 db  3Fh ; ?
.data:008F8E44                 db 0D8h ; Ø
.data:008F8E45                 db  77h ; w
.data:008F8E46                 db  93h ; “
.data:008F8E47                 db    0
.data:008F8E48 unk_8F8E48      db    6                 ; DATA XREF: .data:off_8F8A74↑o
.data:008F8E49                 db    0
.data:008F8E4A                 db    0
.data:008F8E4B                 db    0
.data:008F8E4C                 db    0
.data:008F8E4D                 db    0
.data:008F8E4E                 db    0
.data:008F8E4F                 db    0
.data:008F8E50                 db    0
.data:008F8E51                 db    0
.data:008F8E52                 db  20h
.data:008F8E53                 db  41h ; A
.data:008F8E54                 db    0
.data:008F8E55                 db    0
.data:008F8E56                 db    0
.data:008F8E57                 db  3Fh ; ?
.data:008F8E58                 db 0E0h ; à
.data:008F8E59                 db  89h ; ‰
.data:008F8E5A                 db  8Fh
.data:008F8E5B                 db    0
.data:008F8E5C unk_8F8E5C      db    6                 ; DATA XREF: .data:008F8A78↑o
.data:008F8E5D                 db    0
.data:008F8E5E                 db    0
.data:008F8E5F                 db    0
.data:008F8E60                 db    0
.data:008F8E61                 db    0
.data:008F8E62                 db    0
.data:008F8E63                 db    0
.data:008F8E64                 db    0
.data:008F8E65                 db    0
.data:008F8E66                 db  20h
.data:008F8E67                 db  41h ; A
.data:008F8E68                 db    0
.data:008F8E69                 db    0
.data:008F8E6A                 db    0
.data:008F8E6B                 db  3Fh ; ?
.data:008F8E6C                 db 0F8h ; ø
.data:008F8E6D                 db  89h ; ‰
.data:008F8E6E                 db  8Fh
.data:008F8E6F                 db    0
.data:008F8E70 unk_8F8E70      db    6                 ; DATA XREF: .data:008F8A7C↑o
.data:008F8E71                 db    0
.data:008F8E72                 db    0
.data:008F8E73                 db    0
.data:008F8E74                 db    0
.data:008F8E75                 db    0
.data:008F8E76                 db    0
.data:008F8E77                 db    0
.data:008F8E78                 db    0
.data:008F8E79                 db    0
.data:008F8E7A                 db  20h
.data:008F8E7B                 db  41h ; A
.data:008F8E7C                 db    0
.data:008F8E7D                 db    0
.data:008F8E7E                 db    0
.data:008F8E7F                 db  3Fh ; ?
.data:008F8E80                 db  10h
.data:008F8E81                 db  8Ah ; Š
.data:008F8E82                 db  8Fh
.data:008F8E83                 db    0
.data:008F8E84 unk_8F8E84      db    6                 ; DATA XREF: .data:008F8A80↑o
.data:008F8E85                 db    0
.data:008F8E86                 db    0
.data:008F8E87                 db    0
.data:008F8E88                 db    0
.data:008F8E89                 db    0
.data:008F8E8A                 db    0
.data:008F8E8B                 db    0
.data:008F8E8C                 db    0
.data:008F8E8D                 db    0
.data:008F8E8E                 db  20h
.data:008F8E8F                 db  41h ; A
.data:008F8E90                 db    0
.data:008F8E91                 db    0
.data:008F8E92                 db    0
.data:008F8E93                 db  3Fh ; ?
.data:008F8E94                 db  28h ; (
.data:008F8E95                 db  8Ah ; Š
.data:008F8E96                 db  8Fh
.data:008F8E97                 db    0
.data:008F8E98 unk_8F8E98      db    6                 ; DATA XREF: .data:008F8A84↑o
.data:008F8E99                 db    0
.data:008F8E9A                 db    0
.data:008F8E9B                 db    0
.data:008F8E9C                 db    0
.data:008F8E9D                 db    0
.data:008F8E9E                 db    0
.data:008F8E9F                 db    0
.data:008F8EA0                 db    0
.data:008F8EA1                 db    0
.data:008F8EA2                 db  20h
.data:008F8EA3                 db  41h ; A
.data:008F8EA4                 db    0
.data:008F8EA5                 db    0
.data:008F8EA6                 db    0
.data:008F8EA7                 db  3Fh ; ?
.data:008F8EA8                 db  40h ; @
.data:008F8EA9                 db  8Ah ; Š
.data:008F8EAA                 db  8Fh
.data:008F8EAB                 db    0
.data:008F8EAC unk_8F8EAC      db    6                 ; DATA XREF: .data:off_8F8A88↑o
.data:008F8EAD                 db    0
.data:008F8EAE                 db    0
.data:008F8EAF                 db    0
.data:008F8EB0                 db    0
.data:008F8EB1                 db    0
.data:008F8EB2                 db    0
.data:008F8EB3                 db    0
.data:008F8EB4                 db    0
.data:008F8EB5                 db    0
.data:008F8EB6                 db  20h
.data:008F8EB7                 db  41h ; A
.data:008F8EB8                 db    0
.data:008F8EB9                 db    0
.data:008F8EBA                 db    0
.data:008F8EBB                 db  3Fh ; ?
.data:008F8EBC                 db  58h ; X
.data:008F8EBD                 db  8Ah ; Š
.data:008F8EBE                 db  8Fh
.data:008F8EBF                 db    0
.data:008F8EC0 unk_8F8EC0      db    0                 ; DATA XREF: .text:00888512↑o
.data:008F8EC1                 db    0
.data:008F8EC2                 db    0
.data:008F8EC3                 db    0
.data:008F8EC4                 db 0CDh ; Í
.data:008F8EC5                 db 0CCh ; Ì
.data:008F8EC6                 db  4Ch ; L
.data:008F8EC7                 db  3Eh ; >
.data:008F8EC8                 db 0CDh ; Í
.data:008F8EC9                 db 0CCh ; Ì
.data:008F8ECA                 db  4Ch ; L
.data:008F8ECB                 db  3Eh ; >
.data:008F8ECC                 db    0
.data:008F8ECD                 db    0
.data:008F8ECE                 db    0
.data:008F8ECF                 db  3Fh ; ?
.data:008F8ED0                 db    0
.data:008F8ED1                 db    0
.data:008F8ED2                 db    0
.data:008F8ED3                 db  3Fh ; ?
.data:008F8ED4                 db  33h ; 3
.data:008F8ED5                 db  33h ; 3
.data:008F8ED6                 db  33h ; 3
.data:008F8ED7                 db  3Fh ; ?
.data:008F8ED8                 db  33h ; 3
.data:008F8ED9                 db  33h ; 3
.data:008F8EDA                 db  33h ; 3
.data:008F8EDB                 db  3Fh ; ?
.data:008F8EDC                 db    0
.data:008F8EDD                 db    0
.data:008F8EDE                 db  80h ; €
.data:008F8EDF                 db  3Fh ; ?
.data:008F8EE0 unk_8F8EE0      db  15h                 ; DATA XREF: sub_69E820+D2↑o
.data:008F8EE1                 db    0
.data:008F8EE2                 db    0
.data:008F8EE3                 db    0
.data:008F8EE4                 db    0
.data:008F8EE5                 db    0
.data:008F8EE6                 db  80h ; €
.data:008F8EE7                 db 0BFh ; ¿
.data:008F8EE8                 db    0
.data:008F8EE9                 db    0
.data:008F8EEA                 db  80h ; €
.data:008F8EEB                 db  3Fh ; ?
.data:008F8EEC                 db    0
.data:008F8EED                 db    0
.data:008F8EEE                 db  20h
.data:008F8EEF                 db  41h ; A
.data:008F8EF0                 db  80h ; €
.data:008F8EF1                 db  8Bh ; ‹
.data:008F8EF2                 db  8Fh
.data:008F8EF3                 db    0
.data:008F8EF4                 db  15h
.data:008F8EF5                 db    0
.data:008F8EF6                 db    0
.data:008F8EF7                 db    0
.data:008F8EF8                 db    0
.data:008F8EF9                 db    0
.data:008F8EFA                 db  80h ; €
.data:008F8EFB                 db 0BFh ; ¿
.data:008F8EFC                 db    0
.data:008F8EFD                 db    0
.data:008F8EFE                 db  80h ; €
.data:008F8EFF                 db  3Fh ; ?
.data:008F8F00                 db    0
.data:008F8F01                 db    0
.data:008F8F02                 db  20h
.data:008F8F03                 db  41h ; A
.data:008F8F04                 db 0D8h ; Ø
.data:008F8F05                 db  8Bh ; ‹
.data:008F8F06                 db  8Fh
.data:008F8F07                 db    0
.data:008F8F08                 db  15h
.data:008F8F09                 db    0
.data:008F8F0A                 db    0
.data:008F8F0B                 db    0
.data:008F8F0C                 db    0
.data:008F8F0D                 db    0
.data:008F8F0E                 db  80h ; €
.data:008F8F0F                 db 0BFh ; ¿
.data:008F8F10                 db    0
.data:008F8F11                 db    0
.data:008F8F12                 db  80h ; €
.data:008F8F13                 db  3Fh ; ?
.data:008F8F14                 db    0
.data:008F8F15                 db    0
.data:008F8F16                 db  20h
.data:008F8F17                 db  41h ; A
.data:008F8F18                 db  30h ; 0
.data:008F8F19                 db  8Ch ; Œ
.data:008F8F1A                 db  8Fh
.data:008F8F1B                 db    0
.data:008F8F1C                 db  15h
.data:008F8F1D                 db    0
.data:008F8F1E                 db    0
.data:008F8F1F                 db    0
.data:008F8F20                 db    0
.data:008F8F21                 db    0
.data:008F8F22                 db  80h ; €
.data:008F8F23                 db 0BFh ; ¿
.data:008F8F24                 db    0
.data:008F8F25                 db    0
.data:008F8F26                 db  80h ; €
.data:008F8F27                 db  3Fh ; ?
.data:008F8F28                 db    0
.data:008F8F29                 db    0
.data:008F8F2A                 db  20h
.data:008F8F2B                 db  41h ; A
.data:008F8F2C                 db  88h ; ˆ
.data:008F8F2D                 db  8Ch ; Œ
.data:008F8F2E                 db  8Fh
.data:008F8F2F                 db    0
.data:008F8F30 unk_8F8F30      db  0Ah                 ; DATA XREF: sub_68E9E0+15↑o
.data:008F8F31                 db    0
.data:008F8F32                 db    0
.data:008F8F33                 db    0
.data:008F8F34                 db    0
.data:008F8F35                 db    0
.data:008F8F36                 db    0
.data:008F8F37                 db    0
.data:008F8F38                 db    0
.data:008F8F39                 db    0
.data:008F8F3A                 db  20h
.data:008F8F3B                 db  43h ; C
.data:008F8F3C                 db  66h ; f
.data:008F8F3D                 db  66h ; f
.data:008F8F3E                 db  66h ; f
.data:008F8F3F                 db  3Dh ; =
.data:008F8F40                 db 0BCh ; ¼
.data:008F8F41                 db  8Ah ; Š
.data:008F8F42                 db  8Fh
.data:008F8F43                 db    0
.data:008F8F44 unk_8F8F44      db  0Ah                 ; DATA XREF: sub_68E9E0+43↑o
.data:008F8F45                 db    0
.data:008F8F46                 db    0
.data:008F8F47                 db    0
.data:008F8F48                 db    0
.data:008F8F49                 db    0
.data:008F8F4A                 db    0
.data:008F8F4B                 db    0
.data:008F8F4C                 db    0
.data:008F8F4D                 db    0
.data:008F8F4E                 db  20h
.data:008F8F4F                 db  43h ; C
.data:008F8F50                 db  66h ; f
.data:008F8F51                 db  66h ; f
.data:008F8F52                 db  66h ; f
.data:008F8F53                 db  3Dh ; =
.data:008F8F54                 db  24h ; $
.data:008F8F55                 db  8Bh ; ‹
.data:008F8F56                 db  8Fh
.data:008F8F57                 db    0
.data:008F8F58 unk_8F8F58      db    6                 ; DATA XREF: sub_68E9E0+8D↑o
.data:008F8F59                 db    0
.data:008F8F5A                 db    0
.data:008F8F5B                 db    0
.data:008F8F5C                 db    0
.data:008F8F5D                 db    0
.data:008F8F5E                 db    0
.data:008F8F5F                 db    0
.data:008F8F60                 db    0
.data:008F8F61                 db    0
.data:008F8F62                 db  80h ; €
.data:008F8F63                 db  3Fh ; ?
.data:008F8F64                 db    0
.data:008F8F65                 db    0
.data:008F8F66                 db 0A0h ;  
.data:008F8F67                 db  40h ; @
.data:008F8F68                 db 0E4h ; ä
.data:008F8F69                 db  8Ah ; Š
.data:008F8F6A                 db  8Fh
.data:008F8F6B                 db    0
.data:008F8F6C unk_8F8F6C      db  0Ah                 ; DATA XREF: sub_68E9E0+61↑o
.data:008F8F6C                                         ; sub_68EC70+50↑o
.data:008F8F6D                 db    0
.data:008F8F6E                 db    0
.data:008F8F6F                 db    0
.data:008F8F70                 db    0
.data:008F8F71                 db    0
.data:008F8F72                 db    0
.data:008F8F73                 db    0
.data:008F8F74                 db    0
.data:008F8F75                 db    0
.data:008F8F76                 db  20h
.data:008F8F77                 db  43h ; C
.data:008F8F78                 db  66h ; f
.data:008F8F79                 db  66h ; f
.data:008F8F7A                 db  66h ; f
.data:008F8F7B                 db  3Dh ; =
.data:008F8F7C                 db 0FCh ; ü
.data:008F8F7D                 db  8Ah ; Š
.data:008F8F7E                 db  8Fh
.data:008F8F7F                 db    0
.data:008F8F80 unk_8F8F80      db    6                 ; DATA XREF: sub_68EC70+2A↑o
.data:008F8F81                 db    0
.data:008F8F82                 db    0
.data:008F8F83                 db    0
.data:008F8F84                 db    0
.data:008F8F85                 db    0
.data:008F8F86                 db    0
.data:008F8F87                 db    0
.data:008F8F88                 db    0
.data:008F8F89                 db    0
.data:008F8F8A                 db  20h
.data:008F8F8B                 db  41h ; A
.data:008F8F8C                 db    0
.data:008F8F8D                 db    0
.data:008F8F8E                 db    0
.data:008F8F8F                 db  3Fh ; ?
.data:008F8F90                 db  4Ch ; L
.data:008F8F91                 db  8Bh ; ‹
.data:008F8F92                 db  8Fh
.data:008F8F93                 db    0
.data:008F8F94 unk_8F8F94      db    6                 ; DATA XREF: sub_68EC70+61↑o
.data:008F8F95                 db    0
.data:008F8F96                 db    0
.data:008F8F97                 db    0
.data:008F8F98                 db    0
.data:008F8F99                 db    0
.data:008F8F9A                 db    0
.data:008F8F9B                 db    0
.data:008F8F9C                 db    0
.data:008F8F9D                 db    0
.data:008F8F9E                 db  80h ; €
.data:008F8F9F                 db  3Fh ; ?
.data:008F8FA0                 db    0
.data:008F8FA1                 db    0
.data:008F8FA2                 db 0A0h ;  
.data:008F8FA3                 db  40h ; @
.data:008F8FA4                 db  64h ; d
.data:008F8FA5                 db  8Bh ; ‹
.data:008F8FA6                 db  8Fh
.data:008F8FA7                 db    0
.data:008F8FA8 off_8F8FA8      dd offset unk_8F8CDC    ; DATA XREF: SuspensionRacer::Burnout::Update(float,float,float,int,float)+DD↑o
.data:008F8FAC                 db    6
.data:008F8FAD                 db    0
.data:008F8FAE                 db    0
.data:008F8FAF                 db    0
.data:008F8FB0 unk_8F8FB0      db  0Ah                 ; DATA XREF: SuspensionRacer::DoDrifting(Chassis::State const &)+37E↑o
.data:008F8FB1                 db    0
.data:008F8FB2                 db    0
.data:008F8FB3                 db    0
.data:008F8FB4                 db    0
.data:008F8FB5                 db    0
.data:008F8FB6                 db    0
.data:008F8FB7                 db    0
.data:008F8FB8                 db    0
.data:008F8FB9                 db    0
.data:008F8FBA                 db  80h ; €
.data:008F8FBB                 db  3Fh ; ?
.data:008F8FBC                 db    0
.data:008F8FBD                 db    0
.data:008F8FBE                 db  10h
.data:008F8FBF                 db  41h ; A
.data:008F8FC0                 db  58h ; X
.data:008F8FC1                 db  8Dh
.data:008F8FC2                 db  8Fh
.data:008F8FC3                 db    0
.data:008F8FC4 unk_8F8FC4      db    7                 ; DATA XREF: sub_6906B0+16B↑o
.data:008F8FC5                 db    0
.data:008F8FC6                 db    0
.data:008F8FC7                 db    0
.data:008F8FC8                 db    0
.data:008F8FC9                 db    0
.data:008F8FCA                 db    0
.data:008F8FCB                 db    0
.data:008F8FCC                 db  89h ; ‰
.data:008F8FCD                 db  88h ; ˆ
.data:008F8FCE                 db    8
.data:008F8FCF                 db  3Dh ; =
.data:008F8FD0                 db    0
.data:008F8FD1                 db    0
.data:008F8FD2                 db  34h ; 4
.data:008F8FD3                 db  43h ; C
.data:008F8FD4                 db  80h ; €
.data:008F8FD5                 db  8Dh
.data:008F8FD6                 db  8Fh
.data:008F8FD7                 db    0
.data:008F8FD8 off_8F8FD8      dd offset unk_8F8DA0    ; DATA XREF: EngineRacer::OnTaskSimulate(float)+6E5↑o
.data:008F8FDC                 db    5
.data:008F8FDD                 db    0
.data:008F8FDE                 db    0
.data:008F8FDF                 db    0
.data:008F8FE0 flt_8F8FE0      dd 100.0                ; DATA XREF: sub_68E220:loc_68E280↑r
.data:008F8FE4 flt_8F8FE4      dd 1.2                  ; DATA XREF: sub_68E220+3F↑r
.data:008F8FE8                 align 10h
.data:008F8FF0 ; int dword_8F8FF0
.data:008F8FF0 dword_8F8FF0    dd 15h                  ; DATA XREF: sub_6BFAB0:loc_6BFB37↑r
.data:008F8FF0                                         ; eInitEngine(void)+1E616C↑w
.data:008F8FF4 dword_8F8FF4    dd 100h                 ; DATA XREF: sub_6BD0D0+6↑r
.data:008F8FF4                                         ; sub_6BD0D0+274↑r ...
.data:008F8FF8 dword_8F8FF8    dd 15h                  ; DATA XREF: sub_6BD4B0:loc_6BD50B↑r
.data:008F8FF8                                         ; sub_6BD4B0+7D↑r
.data:008F8FFC dword_8F8FFC    dd 50h                  ; DATA XREF: sub_6BD4B0+D5↑r
.data:008F9000 dword_8F9000    dd 15h                  ; DATA XREF: sub_6BCDC0+5↑r
.data:008F9000                                         ; sub_6BCDC0+95↑r ...
.data:008F9004 dword_8F9004    dd 50h                  ; DATA XREF: sub_6BCDC0+5B↑r
.data:008F9004                                         ; sub_6BCFA0+3F↑r
.data:008F9008 dword_8F9008    dd 100h                 ; DATA XREF: sub_6BCFA0+22↑r
.data:008F9008                                         ; sub_6BCFA0+61↑r ...
.data:008F900C dword_8F900C    dd 100h                 ; DATA XREF: sub_6BCFA0+17↑r
.data:008F900C                                         ; sub_6BCFA0+5A↑r ...
.data:008F9010 dword_8F9010    dd 3F000000h            ; DATA XREF: sub_6DA350+1E3↑r
.data:008F9014 dword_8F9014    dd 42C80000h            ; DATA XREF: sub_6DA350+1F0↑r
.data:008F9018 dword_8F9018    dd 42000000h            ; DATA XREF: sub_6DA350+1D8↑r
.data:008F901C                 db    6
.data:008F901D                 db    0
.data:008F901E                 db    0
.data:008F901F                 db    0
.data:008F9020                 db    4
.data:008F9021                 db    0
.data:008F9022                 db    0
.data:008F9023                 db    0
.data:008F9024                 db    1
.data:008F9025                 db    0
.data:008F9026                 db    0
.data:008F9027                 db    0
.data:008F9028                 db    1
.data:008F9029                 db    0
.data:008F902A                 db    0
.data:008F902B                 db    0
.data:008F902C                 db    0
.data:008F902D                 db    0
.data:008F902E                 db    0
.data:008F902F                 db    0
.data:008F9030                 db    0
.data:008F9031                 db    0
.data:008F9032                 db    0
.data:008F9033                 db    0
.data:008F9034                 db    0
.data:008F9035                 db    0
.data:008F9036                 db    0
.data:008F9037                 db    0
.data:008F9038                 db    0
.data:008F9039                 db    0
.data:008F903A                 db    0
.data:008F903B                 db    0
.data:008F903C                 db    0
.data:008F903D                 db    0
.data:008F903E                 db    0
.data:008F903F                 db    0
.data:008F9040                 db    0
.data:008F9041                 db    0
.data:008F9042                 db    0
.data:008F9043                 db    0
.data:008F9044                 db    1
.data:008F9045                 db    0
.data:008F9046                 db    0
.data:008F9047                 db    0
.data:008F9048                 db    0
.data:008F9049                 db    0
.data:008F904A                 db    0
.data:008F904B                 db    0
.data:008F904C                 db    0
.data:008F904D                 db    0
.data:008F904E                 db    0
.data:008F904F                 db    0
.data:008F9050                 db    0
.data:008F9051                 db    0
.data:008F9052                 db    0
.data:008F9053                 db    0
.data:008F9054                 db    0
.data:008F9055                 db    0
.data:008F9056                 db    0
.data:008F9057                 db    0
.data:008F9058                 db    0
.data:008F9059                 db    0
.data:008F905A                 db    0
.data:008F905B                 db    0
.data:008F905C                 db    0
.data:008F905D                 db    0
.data:008F905E                 db    0
.data:008F905F                 db    0
.data:008F9060                 db    1
.data:008F9061                 db    0
.data:008F9062                 db    0
.data:008F9063                 db    0
.data:008F9064                 db    0
.data:008F9065                 db    0
.data:008F9066                 db    0
.data:008F9067                 db    0
.data:008F9068                 db    0
.data:008F9069                 db    0
.data:008F906A                 db    0
.data:008F906B                 db    0
.data:008F906C                 db    0
.data:008F906D                 db    0
.data:008F906E                 db    0
.data:008F906F                 db    0
.data:008F9070                 db    0
.data:008F9071                 db    0
.data:008F9072                 db    0
.data:008F9073                 db    0
.data:008F9074                 db    0
.data:008F9075                 db    0
.data:008F9076                 db    0
.data:008F9077                 db    0
.data:008F9078                 db    0
.data:008F9079                 db    0
.data:008F907A                 db    0
.data:008F907B                 db    0
.data:008F907C                 db    1
.data:008F907D                 db    0
.data:008F907E                 db    0
.data:008F907F                 db    0
.data:008F9080                 db    0
.data:008F9081                 db    0
.data:008F9082                 db    0
.data:008F9083                 db    0
.data:008F9084                 db    0
.data:008F9085                 db    0
.data:008F9086                 db    0
.data:008F9087                 db    0
.data:008F9088                 db    0
.data:008F9089                 db    0
.data:008F908A                 db    0
.data:008F908B                 db    0
.data:008F908C                 db    0
.data:008F908D                 db    0
.data:008F908E                 db    0
.data:008F908F                 db    0
.data:008F9090                 db    0
.data:008F9091                 db    0
.data:008F9092                 db    0
.data:008F9093                 db    0
.data:008F9094                 db    0
.data:008F9095                 db    0
.data:008F9096                 db    0
.data:008F9097                 db    0
.data:008F9098                 db    1
.data:008F9099                 db    0
.data:008F909A                 db    0
.data:008F909B                 db    0
.data:008F909C                 db    0
.data:008F909D                 db    0
.data:008F909E                 db    0
.data:008F909F                 db    0
.data:008F90A0                 db    0
.data:008F90A1                 db    0
.data:008F90A2                 db    0
.data:008F90A3                 db    0
.data:008F90A4                 db    0
.data:008F90A5                 db    0
.data:008F90A6                 db    0
.data:008F90A7                 db    0
.data:008F90A8                 db    0
.data:008F90A9                 db    0
.data:008F90AA                 db    0
.data:008F90AB                 db    0
.data:008F90AC                 db    0
.data:008F90AD                 db    0
.data:008F90AE                 db    0
.data:008F90AF                 db    0
.data:008F90B0                 db    0
.data:008F90B1                 db    0
.data:008F90B2                 db    0
.data:008F90B3                 db    0
.data:008F90B4                 db    1
.data:008F90B5                 db    0
.data:008F90B6                 db    0
.data:008F90B7                 db    0
.data:008F90B8                 db    1
.data:008F90B9                 db    0
.data:008F90BA                 db    0
.data:008F90BB                 db    0
.data:008F90BC                 db    1
.data:008F90BD                 db    0
.data:008F90BE                 db    0
.data:008F90BF                 db    0
.data:008F90C0                 db    0
.data:008F90C1                 db    0
.data:008F90C2                 db    0
.data:008F90C3                 db    0
.data:008F90C4                 db    0
.data:008F90C5                 db    0
.data:008F90C6                 db    0
.data:008F90C7                 db    0
.data:008F90C8                 db    1
.data:008F90C9                 db    0
.data:008F90CA                 db    0
.data:008F90CB                 db    0
.data:008F90CC                 db    0
.data:008F90CD                 db    0
.data:008F90CE                 db    0
.data:008F90CF                 db    0
.data:008F90D0                 db    1
.data:008F90D1                 db    0
.data:008F90D2                 db    0
.data:008F90D3                 db    0
.data:008F90D4                 db    0
.data:008F90D5                 db    0
.data:008F90D6                 db    0
.data:008F90D7                 db    0
.data:008F90D8                 db    1
.data:008F90D9                 db    0
.data:008F90DA                 db    0
.data:008F90DB                 db    0
.data:008F90DC                 db    0
.data:008F90DD                 db    0
.data:008F90DE                 db    0
.data:008F90DF                 db    0
.data:008F90E0                 db    1
.data:008F90E1                 db    0
.data:008F90E2                 db    0
.data:008F90E3                 db    0
.data:008F90E4                 db    0
.data:008F90E5                 db    0
.data:008F90E6                 db    0
.data:008F90E7                 db    0
.data:008F90E8                 db    1
.data:008F90E9                 db    0
.data:008F90EA                 db    0
.data:008F90EB                 db    0
.data:008F90EC                 db    0
.data:008F90ED                 db    0
.data:008F90EE                 db    0
.data:008F90EF                 db    0
.data:008F90F0                 db    0
.data:008F90F1                 db    0
.data:008F90F2                 db    0
.data:008F90F3                 db    0
.data:008F90F4                 db    1
.data:008F90F5                 db    0
.data:008F90F6                 db    0
.data:008F90F7                 db    0
.data:008F90F8                 db    1
.data:008F90F9                 db    0
.data:008F90FA                 db    0
.data:008F90FB                 db    0
.data:008F90FC                 db    0
.data:008F90FD                 db    0
.data:008F90FE                 db    0
.data:008F90FF                 db    0
.data:008F9100                 db    1
.data:008F9101                 db    0
.data:008F9102                 db    0
.data:008F9103                 db    0
.data:008F9104                 db    0
.data:008F9105                 db    0
.data:008F9106                 db    0
.data:008F9107                 db    0
.data:008F9108                 db    0
.data:008F9109                 db    0
.data:008F910A                 db    0
.data:008F910B                 db    0
.data:008F910C                 db    0
.data:008F910D                 db    0
.data:008F910E                 db    0
.data:008F910F                 db    0
.data:008F9110                 db    1
.data:008F9111                 db    0
.data:008F9112                 db    0
.data:008F9113                 db    0
.data:008F9114                 db    1
.data:008F9115                 db    0
.data:008F9116                 db    0
.data:008F9117                 db    0
.data:008F9118                 db    0
.data:008F9119                 db    0
.data:008F911A                 db    0
.data:008F911B                 db    0
.data:008F911C                 db    0
.data:008F911D                 db    0
.data:008F911E                 db    0
.data:008F911F                 db    0
.data:008F9120                 db    0
.data:008F9121                 db    0
.data:008F9122                 db    0
.data:008F9123                 db    0
.data:008F9124                 db    0
.data:008F9125                 db    0
.data:008F9126                 db    0
.data:008F9127                 db    0
.data:008F9128                 db    0
.data:008F9129                 db    0
.data:008F912A                 db    0
.data:008F912B                 db    0
.data:008F912C                 db    0
.data:008F912D                 db    0
.data:008F912E                 db    0
.data:008F912F                 db    0
.data:008F9130                 db    0
.data:008F9131                 db    0
.data:008F9132                 db    0
.data:008F9133                 db    0
.data:008F9134                 db    0
.data:008F9135                 db    0
.data:008F9136                 db    0
.data:008F9137                 db    0
.data:008F9138                 db    0
.data:008F9139                 db    0
.data:008F913A                 db    0
.data:008F913B                 db    0
.data:008F913C                 db    0
.data:008F913D                 db    0
.data:008F913E                 db    0
.data:008F913F                 db    0
.data:008F9140                 db    0
.data:008F9141                 db    0
.data:008F9142                 db    0
.data:008F9143                 db    0
.data:008F9144                 db    0
.data:008F9145                 db    0
.data:008F9146                 db    0
.data:008F9147                 db    0
.data:008F9148                 db    1
.data:008F9149                 db    0
.data:008F914A                 db    0
.data:008F914B                 db    0
.data:008F914C                 db    1
.data:008F914D                 db    0
.data:008F914E                 db    0
.data:008F914F                 db    0
.data:008F9150                 db    1
.data:008F9151                 db    0
.data:008F9152                 db    0
.data:008F9153                 db    0
.data:008F9154                 db    1
.data:008F9155                 db    0
.data:008F9156                 db    0
.data:008F9157                 db    0
.data:008F9158                 db    1
.data:008F9159                 db    0
.data:008F915A                 db    0
.data:008F915B                 db    0
.data:008F915C                 db    1
.data:008F915D                 db    0
.data:008F915E                 db    0
.data:008F915F                 db    0
.data:008F9160                 db    0
.data:008F9161                 db    0
.data:008F9162                 db    0
.data:008F9163                 db    0
.data:008F9164                 db    0
.data:008F9165                 db    0
.data:008F9166                 db    0
.data:008F9167                 db    0
.data:008F9168                 db    0
.data:008F9169                 db    0
.data:008F916A                 db    0
.data:008F916B                 db    0
.data:008F916C                 db    0
.data:008F916D                 db    0
.data:008F916E                 db    0
.data:008F916F                 db    0
.data:008F9170                 db    0
.data:008F9171                 db    0
.data:008F9172                 db    0
.data:008F9173                 db    0
.data:008F9174                 db    0
.data:008F9175                 db    0
.data:008F9176                 db    0
.data:008F9177                 db    0
.data:008F9178                 db    0
.data:008F9179                 db    0
.data:008F917A                 db    0
.data:008F917B                 db    0
.data:008F917C                 db    0
.data:008F917D                 db    0
.data:008F917E                 db    0
.data:008F917F                 db    0
.data:008F9180                 db    0
.data:008F9181                 db    0
.data:008F9182                 db    0
.data:008F9183                 db    0
.data:008F9184                 db    0
.data:008F9185                 db    0
.data:008F9186                 db    0
.data:008F9187                 db    0
.data:008F9188                 db    0
.data:008F9189                 db    0
.data:008F918A                 db    0
.data:008F918B                 db    0
.data:008F918C                 db    0
.data:008F918D                 db    0
.data:008F918E                 db    0
.data:008F918F                 db    0
.data:008F9190                 db    0
.data:008F9191                 db    0
.data:008F9192                 db    0
.data:008F9193                 db    0
.data:008F9194                 db    0
.data:008F9195                 db    0
.data:008F9196                 db    0
.data:008F9197                 db    0
.data:008F9198                 db    0
.data:008F9199                 db    0
.data:008F919A                 db    0
.data:008F919B                 db    0
.data:008F919C                 db    0
.data:008F919D                 db    0
.data:008F919E                 db    0
.data:008F919F                 db    0
.data:008F91A0                 db    0
.data:008F91A1                 db    0
.data:008F91A2                 db    0
.data:008F91A3                 db    0
.data:008F91A4                 db    0
.data:008F91A5                 db    0
.data:008F91A6                 db    0
.data:008F91A7                 db    0
.data:008F91A8                 db    0
.data:008F91A9                 db    0
.data:008F91AA                 db    0
.data:008F91AB                 db    0
.data:008F91AC                 db    0
.data:008F91AD                 db    0
.data:008F91AE                 db    0
.data:008F91AF                 db    0
.data:008F91B0                 db    0
.data:008F91B1                 db    0
.data:008F91B2                 db    0
.data:008F91B3                 db    0
.data:008F91B4                 db    0
.data:008F91B5                 db    0
.data:008F91B6                 db    0
.data:008F91B7                 db    0
.data:008F91B8                 db    0
.data:008F91B9                 db    0
.data:008F91BA                 db    0
.data:008F91BB                 db    0
.data:008F91BC                 db    0
.data:008F91BD                 db    0
.data:008F91BE                 db    0
.data:008F91BF                 db    0
.data:008F91C0                 db    0
.data:008F91C1                 db    0
.data:008F91C2                 db    0
.data:008F91C3                 db    0
.data:008F91C4                 db    0
.data:008F91C5                 db    0
.data:008F91C6                 db    0
.data:008F91C7                 db    0
.data:008F91C8                 db    0
.data:008F91C9                 db    0
.data:008F91CA                 db    0
.data:008F91CB                 db    0
.data:008F91CC                 db    0
.data:008F91CD                 db    0
.data:008F91CE                 db    0
.data:008F91CF                 db    0
.data:008F91D0                 db    0
.data:008F91D1                 db    0
.data:008F91D2                 db    0
.data:008F91D3                 db    0
.data:008F91D4                 db    0
.data:008F91D5                 db    0
.data:008F91D6                 db    0
.data:008F91D7                 db    0
.data:008F91D8                 db    2
.data:008F91D9                 db    0
.data:008F91DA                 db    0
.data:008F91DB                 db    0
.data:008F91DC                 db    1
.data:008F91DD                 db    0
.data:008F91DE                 db    0
.data:008F91DF                 db    0
.data:008F91E0                 db    2
.data:008F91E1                 db    0
.data:008F91E2                 db    0
.data:008F91E3                 db    0
.data:008F91E4 dword_8F91E4    dd 3E99999Ah            ; DATA XREF: eLightMaterialPlatInterface::UpdatePlatInfo(void):loc_6BE23A↑r
.data:008F91E8 dword_8F91E8    dd 3E99999Ah            ; DATA XREF: eLightMaterialPlatInterface::UpdatePlatInfo(void):loc_6BE25E↑r
.data:008F91EC dword_8F91EC    dd 3ECCCCCDh            ; DATA XREF: eLightMaterialPlatInterface::UpdatePlatInfo(void):loc_6BE2E6↑r
.data:008F91F0 dword_8F91F0    dd 3F000000h            ; DATA XREF: eLightMaterialPlatInterface::UpdatePlatInfo(void):loc_6BE1E8↑r
.data:008F91F4 dword_8F91F4    dd 3F800000h            ; DATA XREF: eLightMaterialPlatInterface::UpdatePlatInfo(void)+92↑r
.data:008F91F8 dword_8F91F8    dd 3F800000h            ; DATA XREF: eLightMaterialPlatInterface::UpdatePlatInfo(void)+B6↑r
.data:008F91FC dword_8F91FC    dd 3F800000h            ; DATA XREF: eLightMaterialPlatInterface::UpdatePlatInfo(void)+13E↑r
.data:008F9200 dword_8F9200    dd 3F800000h            ; DATA XREF: eLightMaterialPlatInterface::UpdatePlatInfo(void)+40↑r
.data:008F9204 dword_8F9204    dd 3ECCCCCDh            ; DATA XREF: eLightMaterialPlatInterface::UpdatePlatInfo(void):loc_6BE20C↑r
.data:008F9208 dword_8F9208    dd 3F4CCCCDh            ; DATA XREF: eLightMaterialPlatInterface::UpdatePlatInfo(void)+64↑r
.data:008F920C                 dd offset unk_989680
.data:008F9210 dword_8F9210    dd 0B62EC33Eh           ; DATA XREF: sub_6CF400+B↑w
.data:008F9210                                         ; sub_6CF400+46A↑r
.data:008F9214                 db    0
.data:008F9215                 db    0
.data:008F9216                 db  80h ; €
.data:008F9217                 db  3Fh ; ?
.data:008F9218 ; int dword_8F9218
.data:008F9218 dword_8F9218    dd 1                    ; DATA XREF: sub_6DE300+ED4↑r
.data:008F921C dword_8F921C    dd 1                    ; DATA XREF: sub_6CFCE0:loc_6CFE72↑r
.data:008F9220 unk_8F9220      db  41h ; A             ; DATA XREF: eInitEngine(void)+1E622E↑o
.data:008F9221                 db  16h
.data:008F9222                 db  24h ; $
.data:008F9223                 db  69h ; i
.data:008F9224                 db    0
.data:008F9225                 db    0
.data:008F9226                 db    0
.data:008F9227                 db    0
.data:008F9228                 db    1
.data:008F9229                 db    0
.data:008F922A                 db    0
.data:008F922B                 db    0
.data:008F922C                 db  0Ch
.data:008F922D                 db    0
.data:008F922E                 db    0
.data:008F922F                 db    0
.data:008F9230                 db  0Dh
.data:008F9231                 db    0
.data:008F9232                 db    0
.data:008F9233                 db    0
.data:008F9234                 db  0Eh
.data:008F9235                 db    0
.data:008F9236                 db    0
.data:008F9237                 db    0
.data:008F9238                 db    0
.data:008F9239                 db    0
.data:008F923A                 db    0
.data:008F923B                 db    0
.data:008F923C                 db 0FEh ; ş
.data:008F923D                 db 0FFh ; ÿ
.data:008F923E                 db 0FFh ; ÿ
.data:008F923F                 db 0FFh ; ÿ
.data:008F9240                 db    1
.data:008F9241                 db    0
.data:008F9242                 db    0
.data:008F9243                 db    0
.data:008F9244                 db    0
.data:008F9245                 db    0
.data:008F9246                 db    0
.data:008F9247                 db    0
.data:008F9248                 db    4
.data:008F9249                 db    0
.data:008F924A                 db    0
.data:008F924B                 db    0
.data:008F924C                 db    2
.data:008F924D                 db    0
.data:008F924E                 db    0
.data:008F924F                 db    0
.data:008F9250                 db    1
.data:008F9251                 db    0
.data:008F9252                 db    0
.data:008F9253                 db    0
.data:008F9254                 db    1
.data:008F9255                 db    0
.data:008F9256                 db    0
.data:008F9257                 db    0
.data:008F9258                 db    2
.data:008F9259                 db    0
.data:008F925A                 db    0
.data:008F925B                 db    0
.data:008F925C                 db    3
.data:008F925D                 db    0
.data:008F925E                 db    0
.data:008F925F                 db    0
.data:008F9260                 db    4
.data:008F9261                 db    0
.data:008F9262                 db    0
.data:008F9263                 db    0
.data:008F9264                 db    5
.data:008F9265                 db    0
.data:008F9266                 db    0
.data:008F9267                 db    0
.data:008F9268                 db    6
.data:008F9269                 db    0
.data:008F926A                 db    0
.data:008F926B                 db    0
.data:008F926C                 db    7
.data:008F926D                 db    0
.data:008F926E                 db    0
.data:008F926F                 db    0
.data:008F9270                 db    8
.data:008F9271                 db    0
.data:008F9272                 db    0
.data:008F9273                 db    0
.data:008F9274                 db    0
.data:008F9275                 db    0
.data:008F9276                 db    0
.data:008F9277                 db    0
.data:008F9278                 db  68h ; h
.data:008F9279                 db  24h ; $
.data:008F927A                 db  87h ; ‡
.data:008F927B                 db  73h ; s
.data:008F927C                 db    0
.data:008F927D                 db    0
.data:008F927E                 db    0
.data:008F927F                 db    0
.data:008F9280 unk_8F9280      db  41h ; A             ; DATA XREF: eInitEngine(void)+1E604B↑o
.data:008F9281                 db  16h
.data:008F9282                 db  24h ; $
.data:008F9283                 db  69h ; i
.data:008F9284                 db    0
.data:008F9285                 db    0
.data:008F9286                 db    0
.data:008F9287                 db    0
.data:008F9288                 db    4
.data:008F9289                 db    0
.data:008F928A                 db    0
.data:008F928B                 db    0
.data:008F928C                 db  2Ah ; *
.data:008F928D                 db    0
.data:008F928E                 db    0
.data:008F928F                 db    0
.data:008F9290                 db  2Bh ; +
.data:008F9291                 db    0
.data:008F9292                 db    0
.data:008F9293                 db    0
.data:008F9294                 db  2Ch ; ,
.data:008F9295                 db    0
.data:008F9296                 db    0
.data:008F9297                 db    0
.data:008F9298                 db    0
.data:008F9299                 db    0
.data:008F929A                 db    0
.data:008F929B                 db    0
.data:008F929C                 db    0
.data:008F929D                 db    0
.data:008F929E                 db    0
.data:008F929F                 db    0
.data:008F92A0                 db 0FFh ; ÿ
.data:008F92A1                 db 0FFh ; ÿ
.data:008F92A2                 db 0FFh ; ÿ
.data:008F92A3                 db 0FFh ; ÿ
.data:008F92A4                 db    0
.data:008F92A5                 db    0
.data:008F92A6                 db    0
.data:008F92A7                 db    0
.data:008F92A8                 db    4
.data:008F92A9                 db    0
.data:008F92AA                 db    0
.data:008F92AB                 db    0
.data:008F92AC                 db    3
.data:008F92AD                 db    0
.data:008F92AE                 db    0
.data:008F92AF                 db    0
.data:008F92B0                 db    1
.data:008F92B1                 db    0
.data:008F92B2                 db    0
.data:008F92B3                 db    0
.data:008F92B4                 db    1
.data:008F92B5                 db    0
.data:008F92B6                 db    0
.data:008F92B7                 db    0
.data:008F92B8                 db    2
.data:008F92B9                 db    0
.data:008F92BA                 db    0
.data:008F92BB                 db    0
.data:008F92BC                 db    3
.data:008F92BD                 db    0
.data:008F92BE                 db    0
.data:008F92BF                 db    0
.data:008F92C0                 db    4
.data:008F92C1                 db    0
.data:008F92C2                 db    0
.data:008F92C3                 db    0
.data:008F92C4                 db    5
.data:008F92C5                 db    0
.data:008F92C6                 db    0
.data:008F92C7                 db    0
.data:008F92C8                 db    6
.data:008F92C9                 db    0
.data:008F92CA                 db    0
.data:008F92CB                 db    0
.data:008F92CC                 db    7
.data:008F92CD                 db    0
.data:008F92CE                 db    0
.data:008F92CF                 db    0
.data:008F92D0                 db    8
.data:008F92D1                 db    0
.data:008F92D2                 db    0
.data:008F92D3                 db    0
.data:008F92D4                 db    0
.data:008F92D5                 db    0
.data:008F92D6                 db    0
.data:008F92D7                 db    0
.data:008F92D8                 db  68h ; h
.data:008F92D9                 db  24h ; $
.data:008F92DA                 db  87h ; ‡
.data:008F92DB                 db  73h ; s
.data:008F92DC                 db    0
.data:008F92DD                 db    0
.data:008F92DE                 db 0C0h ; À
.data:008F92DF                 db  3Fh ; ?
.data:008F92E0 ; float dword_8F92E0
.data:008F92E0 dword_8F92E0    dd 40800000h            ; DATA XREF: sub_6DE300+177↑r
.data:008F92E0                                         ; sub_6DE300:loc_6DE49B↑r
.data:008F92E4 ; float dword_8F92E4
.data:008F92E4 dword_8F92E4    dd 3F800000h            ; DATA XREF: sub_6DE300+A0F↑r
.data:008F92E4                                         ; sub_6DE300+113D↑r
.data:008F92E8                 db    0
.data:008F92E9                 db    0
.data:008F92EA                 db    0
.data:008F92EB                 db  3Fh ; ?
.data:008F92EC                 db    0
.data:008F92ED                 db    0
.data:008F92EE                 db  80h ; €
.data:008F92EF                 db  3Fh ; ?
.data:008F92F0                 db    0
.data:008F92F1                 db    0
.data:008F92F2                 db  80h ; €
.data:008F92F3                 db  3Fh ; ?
.data:008F92F4                 db    0
.data:008F92F5                 db    0
.data:008F92F6                 db  80h ; €
.data:008F92F7                 db  3Fh ; ?
.data:008F92F8                 db    0
.data:008F92F9                 db    0
.data:008F92FA                 db  80h ; €
.data:008F92FB                 db  3Fh ; ?
.data:008F92FC                 db  33h ; 3
.data:008F92FD                 db  33h ; 3
.data:008F92FE                 db  33h ; 3
.data:008F92FF                 db  3Fh ; ?
.data:008F9300                 db    0
.data:008F9301                 db    0
.data:008F9302                 db  80h ; €
.data:008F9303                 db  3Fh ; ?
.data:008F9304                 db    0
.data:008F9305                 db    0
.data:008F9306                 db    0
.data:008F9307                 db    0
.data:008F9308                 db    0
.data:008F9309                 db    0
.data:008F930A                 db    0
.data:008F930B                 db    0
.data:008F930C                 db    0
.data:008F930D                 db    0
.data:008F930E                 db    0
.data:008F930F                 db    0
.data:008F9310                 db    0
.data:008F9311                 db    0
.data:008F9312                 db    0
.data:008F9313                 db    0
.data:008F9314 off_8F9314      dd offset aXpos         ; DATA XREF: sub_6BF5A0:loc_6BF5C3↑r
.data:008F9314                                         ; eInitFEEnvMapPlat(void)+14↑o ...
.data:008F9314                                         ; "XPOS"
.data:008F9318                 dd offset aXneg         ; "XNEG"
.data:008F931C                 dd offset aYpos         ; "YPOS"
.data:008F9320                 dd offset aYneg         ; "YNEG"
.data:008F9324                 dd offset aZpos         ; "ZPOS"
.data:008F9328                 dd offset aZneg         ; "ZNEG"
.data:008F932C unk_8F932C      db    1                 ; DATA XREF: eInitFEEnvMapPlat(void)+75↑o
.data:008F932C                                         ; eRemoveFEEnvMapPlat(void)+E3↑o
.data:008F932D                 db    0
.data:008F932E                 db    0
.data:008F932F                 db    0
.data:008F9330 ; int dword_8F9330
.data:008F9330 dword_8F9330    dd 1                    ; DATA XREF: sub_6BFDD0+F↑r
.data:008F9334 ; int EnableEnvMap
.data:008F9334 EnableEnvMap    dd 1                    ; DATA XREF: CarRenderInfo::Render(eView *,bVector3 const *,bMatrix4 const *,bMatrix4 *,bMatrix4 *,bMatrix4 *,uint,int,int,float,CARPART_LOD,CARPART_LOD)+340↑r
.data:008F9338                 dd 100.0
.data:008F933C                 dd 50000.0
.data:008F9340 ; float flt_8F9340
.data:008F9340 flt_8F9340      dd 0.30000001           ; DATA XREF: sub_6DE300+54↑r
.data:008F9344 ; float flt_8F9344
.data:008F9344 flt_8F9344      dd 2.0                  ; DATA XREF: sub_6DE300:loc_6DE385↑r
.data:008F9348 dword_8F9348    dd 42C80000h            ; DATA XREF: sub_6D0B00+36E↑r
.data:008F934C ; int NumberOfStreamAnim
.data:008F934C NumberOfStreamAnim dd 20                ; DATA XREF: sub_6C0910+14↑r
.data:008F934C                                         ; sub_6C0910+1D↑w ...
.data:008F9350 ; int dword_8F9350
.data:008F9350 dword_8F9350    dd 4                    ; DATA XREF: sub_6C0910:loc_6C0937↑r
.data:008F9350                                         ; sub_6C0910+36↑w
.data:008F9354 ; int dword_8F9354
.data:008F9354 dword_8F9354    dd 0FFFFFFFFh           ; DATA XREF: sub_6C0910+51↑r
.data:008F9354                                         ; sub_6C0910+58↑w ...
.data:008F9358 ; char byte_8F9358
.data:008F9358 byte_8F9358     db 1                    ; DATA XREF: eViewPlatInterface::Render(ePoly *,TextureInfo *,int):loc_6D188D↑r
.data:008F9358                                         ; sub_6D1A40:loc_6D1BE0↑r
.data:008F9359                 align 4
.data:008F935C flt_8F935C      dd 1.0                  ; DATA XREF: StuffSkyLayer(eView *,SKY_LAYER)+2AE↑r
.data:008F9360 dword_8F9360    dd 1                    ; DATA XREF: StuffSkyLayer(eView *,SKY_LAYER)+2A1↑r
.data:008F9364 dword_8F9364    dd 1                    ; DATA XREF: StuffSkyLayer(eView *,SKY_LAYER)+15B↑r
.data:008F9368                 db    1
.data:008F9369                 db    0
.data:008F936A                 db    0
.data:008F936B                 db    0
.data:008F936C                 db    0
.data:008F936D                 db    0
.data:008F936E                 db    0
.data:008F936F                 db    0
.data:008F9370                 db    0
.data:008F9371                 db    0
.data:008F9372                 db  80h ; €
.data:008F9373                 db  3Fh ; ?
.data:008F9374                 db    0
.data:008F9375                 db    0
.data:008F9376                 db  80h ; €
.data:008F9377                 db  3Fh ; ?
.data:008F9378                 db    0
.data:008F9379                 db    0
.data:008F937A                 db  80h ; €
.data:008F937B                 db  3Fh ; ?
.data:008F937C                 db    0
.data:008F937D                 db    0
.data:008F937E                 db  80h ; €
.data:008F937F                 db  3Fh ; ?
.data:008F9380                 db    0
.data:008F9381                 db    0
.data:008F9382                 db  80h ; €
.data:008F9383                 db  3Fh ; ?
.data:008F9384                 db    0
.data:008F9385                 db    0
.data:008F9386                 db  80h ; €
.data:008F9387                 db  3Fh ; ?
.data:008F9388                 db    0
.data:008F9389                 db    0
.data:008F938A                 db  80h ; €
.data:008F938B                 db  3Fh ; ?
.data:008F938C                 db    0
.data:008F938D                 db    0
.data:008F938E                 db  80h ; €
.data:008F938F                 db  3Fh ; ?
.data:008F9390                 db    0
.data:008F9391                 db    0
.data:008F9392                 db  80h ; €
.data:008F9393                 db  3Fh ; ?
.data:008F9394                 db    0
.data:008F9395                 db    0
.data:008F9396                 db  80h ; €
.data:008F9397                 db  3Fh ; ?
.data:008F9398                 db    0
.data:008F9399                 db    0
.data:008F939A                 db  80h ; €
.data:008F939B                 db  3Fh ; ?
.data:008F939C                 db    0
.data:008F939D                 db    0
.data:008F939E                 db  80h ; €
.data:008F939F                 db  3Fh ; ?
.data:008F93A0                 db    0
.data:008F93A1                 db    0
.data:008F93A2                 db  80h ; €
.data:008F93A3                 db  3Fh ; ?
.data:008F93A4                 db    0
.data:008F93A5                 db    0
.data:008F93A6                 db  80h ; €
.data:008F93A7                 db  3Fh ; ?
.data:008F93A8                 db    0
.data:008F93A9                 db    0
.data:008F93AA                 db    0
.data:008F93AB                 db    0
.data:008F93AC                 db    0
.data:008F93AD                 db    0
.data:008F93AE                 db  80h ; €
.data:008F93AF                 db  3Fh ; ?
.data:008F93B0                 db    0
.data:008F93B1                 db    0
.data:008F93B2                 db  80h ; €
.data:008F93B3                 db  3Fh ; ?
.data:008F93B4                 db    0
.data:008F93B5                 db    0
.data:008F93B6                 db  80h ; €
.data:008F93B7                 db  3Fh ; ?
.data:008F93B8                 db    0
.data:008F93B9                 db    0
.data:008F93BA                 db  80h ; €
.data:008F93BB                 db  3Fh ; ?
.data:008F93BC                 db    0
.data:008F93BD                 db    0
.data:008F93BE                 db  80h ; €
.data:008F93BF                 db  3Fh ; ?
.data:008F93C0                 db    0
.data:008F93C1                 db    0
.data:008F93C2                 db  80h ; €
.data:008F93C3                 db  3Fh ; ?
.data:008F93C4                 db    0
.data:008F93C5                 db    0
.data:008F93C6                 db  80h ; €
.data:008F93C7                 db  3Fh ; ?
.data:008F93C8                 db    0
.data:008F93C9                 db    0
.data:008F93CA                 db  80h ; €
.data:008F93CB                 db  3Fh ; ?
.data:008F93CC                 db    0
.data:008F93CD                 db    0
.data:008F93CE                 db    0
.data:008F93CF                 db    0
.data:008F93D0                 db    0
.data:008F93D1                 db    0
.data:008F93D2                 db  80h ; €
.data:008F93D3                 db  3Fh ; ?
.data:008F93D4                 db    0
.data:008F93D5                 db    0
.data:008F93D6                 db  80h ; €
.data:008F93D7                 db  3Fh ; ?
.data:008F93D8                 db    0
.data:008F93D9                 db    0
.data:008F93DA                 db  80h ; €
.data:008F93DB                 db  3Fh ; ?
.data:008F93DC                 db    0
.data:008F93DD                 db    0
.data:008F93DE                 db  80h ; €
.data:008F93DF                 db  3Fh ; ?
.data:008F93E0                 db    0
.data:008F93E1                 db    0
.data:008F93E2                 db  80h ; €
.data:008F93E3                 db  3Fh ; ?
.data:008F93E4                 db    0
.data:008F93E5                 db    0
.data:008F93E6                 db  80h ; €
.data:008F93E7                 db  3Fh ; ?
.data:008F93E8                 db    0
.data:008F93E9                 db    0
.data:008F93EA                 db  80h ; €
.data:008F93EB                 db  3Fh ; ?
.data:008F93EC                 db    0
.data:008F93ED                 db    0
.data:008F93EE                 db  80h ; €
.data:008F93EF                 db  3Fh ; ?
.data:008F93F0                 db    0
.data:008F93F1                 db    0
.data:008F93F2                 db  80h ; €
.data:008F93F3                 db  3Fh ; ?
.data:008F93F4                 db    0
.data:008F93F5                 db    0
.data:008F93F6                 db  80h ; €
.data:008F93F7                 db  3Fh ; ?
.data:008F93F8                 db    0
.data:008F93F9                 db    0
.data:008F93FA                 db  80h ; €
.data:008F93FB                 db  3Fh ; ?
.data:008F93FC                 db    0
.data:008F93FD                 db    0
.data:008F93FE                 db  80h ; €
.data:008F93FF                 db  3Fh ; ?
.data:008F9400                 db    0
.data:008F9401                 db    0
.data:008F9402                 db  80h ; €
.data:008F9403                 db  3Fh ; ?
.data:008F9404                 db    0
.data:008F9405                 db    0
.data:008F9406                 db  80h ; €
.data:008F9407                 db  3Fh ; ?
.data:008F9408                 db    0
.data:008F9409                 db    0
.data:008F940A                 db  80h ; €
.data:008F940B                 db  3Fh ; ?
.data:008F940C                 db    0
.data:008F940D                 db    0
.data:008F940E                 db  80h ; €
.data:008F940F                 db  3Fh ; ?
.data:008F9410                 db  28h ; (
.data:008F9411                 db    0
.data:008F9412                 db    0
.data:008F9413                 db    0
.data:008F9414 ; int dword_8F9414[]
.data:008F9414 dword_8F9414    dd 280h                 ; DATA XREF: sub_6C1510+15B↑r
.data:008F9418                 db  20h
.data:008F9419                 db    3
.data:008F941A                 db    0
.data:008F941B                 db    0
.data:008F941C                 db    0
.data:008F941D                 db    4
.data:008F941E                 db    0
.data:008F941F                 db    0
.data:008F9420                 db    0
.data:008F9421                 db    5
.data:008F9422                 db    0
.data:008F9423                 db    0
.data:008F9424                 db    0
.data:008F9425                 db    5
.data:008F9426                 db    0
.data:008F9427                 db    0
.data:008F9428                 db  40h ; @
.data:008F9429                 db    6
.data:008F942A                 db    0
.data:008F942B                 db    0
.data:008F942C ; int dword_8F942C[]
.data:008F942C dword_8F942C    dd 1E0h                 ; DATA XREF: sub_6C1510+167↑r
.data:008F9430                 db  58h ; X
.data:008F9431                 db    2
.data:008F9432                 db    0
.data:008F9433                 db    0
.data:008F9434                 db    0
.data:008F9435                 db    3
.data:008F9436                 db    0
.data:008F9437                 db    0
.data:008F9438                 db 0C0h ; À
.data:008F9439                 db    3
.data:008F943A                 db    0
.data:008F943B                 db    0
.data:008F943C                 db    0
.data:008F943D                 db    4
.data:008F943E                 db    0
.data:008F943F                 db    0
.data:008F9440                 db 0B0h ; °
.data:008F9441                 db    4
.data:008F9442                 db    0
.data:008F9443                 db    0
.data:008F9444                 db  20h
.data:008F9445                 db    0
.data:008F9446                 db    0
.data:008F9447                 db    0
.data:008F9448                 db  64h ; d
.data:008F9449                 db    0
.data:008F944A                 db    0
.data:008F944B                 db    0
.data:008F944C dword_8F944C    dd 1                    ; DATA XREF: sub_6D2100+3C↑w
.data:008F9450                 db  64h ; d
.data:008F9451                 db    0
.data:008F9452                 db    0
.data:008F9453                 db    0
.data:008F9454                 db    1
.data:008F9455                 db    0
.data:008F9456                 db    0
.data:008F9457                 db    0
.data:008F9458                 db    1
.data:008F9459 byte_8F9459     db 1                    ; DATA XREF: .text:006C250A↑w
.data:008F9459                                         ; .text:006C2520↑r ...
.data:008F945A byte_8F945A     db 1                    ; DATA XREF: .text:006C2400↑w
.data:008F945A                                         ; .text:006C2410↑w ...
.data:008F945B byte_8F945B     db 1                    ; DATA XREF: sub_6BD4B0+34↑r
.data:008F945B                                         ; sub_6BD4B0+FA↑w ...
.data:008F945C ; char CDDrive
.data:008F945C CDDrive         db 'A'                  ; DATA XREF: CalculateMovieFilename(char *,int,char const *,eLanguages)+74↑r
.data:008F945C                                         ; sub_64D600+52↑r ...
.data:008F945D                 align 10h
.data:008F9460 flt_8F9460      dd 0.2                  ; DATA XREF: sub_6C3870+48↑r
.data:008F9460                                         ; sub_6C3870+50↑r
.data:008F9464 flt_8F9464      dd 1.0                  ; DATA XREF: sub_6C3870:loc_6C38B1↑r
.data:008F9468                 db  9Ah ; š
.data:008F9469                 db  99h ; ™
.data:008F946A                 db  19h
.data:008F946B                 db  3Eh ; >
.data:008F946C                 db    1
.data:008F946D                 db    0
.data:008F946E                 db    0
.data:008F946F                 db    0
.data:008F9470 dword_8F9470    dd 0CDh                 ; DATA XREF: sub_6C37F0+74↑w
.data:008F9474 dword_8F9474    dd 2                    ; DATA XREF: sub_6DBFE0↑r
.data:008F9474                                         ; sub_6DBFE0+46↑r ...
.data:008F9478 dword_8F9478    dd 0AAh                 ; DATA XREF: sub_6C37F0+50↑w
.data:008F947C dword_8F947C    dd 0Ah                  ; DATA XREF: sub_6C37F0+6↑r
.data:008F947C                                         ; sub_6C37F0+3F↑r
.data:008F9480 dword_8F9480    dd 48h                  ; DATA XREF: sub_6C37F0+12↑r
.data:008F9484 dword_8F9484    dd 0D8h                 ; DATA XREF: sub_6C37F0+55↑r
.data:008F9484                                         ; sub_6C37F0+69↑r
.data:008F9488 dword_8F9488    dd 3Eh                  ; DATA XREF: sub_6C37F0+4A↑r
.data:008F948C flt_8F948C      dd 0.25                 ; DATA XREF: sub_6D3850+80↑r
.data:008F9490 flt_8F9490      dd 0.5                  ; DATA XREF: sub_6D3850+A↑r
.data:008F9490                                         ; sub_6D3850+20↑r
.data:008F9494 flt_8F9494      dd 0.5                  ; DATA XREF: sub_6D3850+32↑r
.data:008F9494                                         ; sub_6D3850+3C↑r
.data:008F9498 flt_8F9498      dd 0.5                  ; DATA XREF: sub_6D3850+4E↑r
.data:008F9498                                         ; sub_6D3850+58↑r
.data:008F949C flt_8F949C      dd 0.5                  ; DATA XREF: sub_6D3850+6A↑r
.data:008F949C                                         ; sub_6D3850+74↑r
.data:008F94A0 flt_8F94A0      dd 0.2                  ; DATA XREF: sub_6D3850↑r
.data:008F94A4 flt_8F94A4      dd 0.2                  ; DATA XREF: sub_6D3850+2C↑r
.data:008F94A8 flt_8F94A8      dd 0.2                  ; DATA XREF: sub_6D3850+48↑r
.data:008F94AC flt_8F94AC      dd 0.2                  ; DATA XREF: sub_6D3850+64↑r
.data:008F94B0 ; float flt_8F94B0
.data:008F94B0 flt_8F94B0      dd 0.2                  ; DATA XREF: sub_6C3570+66↑r
.data:008F94B0                                         ; sub_6C3570+159↑r ...
.data:008F94B4 ; float flt_8F94B4
.data:008F94B4 flt_8F94B4      dd 0.2                  ; DATA XREF: sub_6C3570+95↑r
.data:008F94B4                                         ; sub_6C3570+15F↑r ...
.data:008F94B8 ; float flt_8F94B8
.data:008F94B8 flt_8F94B8      dd 0.2                  ; DATA XREF: sub_6C3570+B1↑r
.data:008F94B8                                         ; sub_6C3570+164↑r ...
.data:008F94BC ; float flt_8F94BC
.data:008F94BC flt_8F94BC      dd 0.25                 ; DATA XREF: sub_6C3570+C2↑r
.data:008F94BC                                         ; sub_6C3570+170↑r ...
.data:008F94C0 ; float flt_8F94C0
.data:008F94C0 flt_8F94C0      dd 0.1                  ; DATA XREF: sub_6D33B0+1EA↑r
.data:008F94C0                                         ; sub_6DE300+1423↑w ...
.data:008F94C4 ; float flt_8F94C4
.data:008F94C4 flt_8F94C4      dd 0.30000001           ; DATA XREF: sub_6D33B0+203↑r
.data:008F94C4                                         ; sub_6DE300+1429↑w ...
.data:008F94C8 ; float flt_8F94C8
.data:008F94C8 flt_8F94C8      dd 0.80000001           ; DATA XREF: sub_6D33B0+21A↑r
.data:008F94C8                                         ; sub_6DE300+1439↑w ...
.data:008F94CC unk_8F94CC      db    0                 ; DATA XREF: sub_6DE300+14AA↑o
.data:008F94CD                 db    0
.data:008F94CE                 db    0
.data:008F94CF                 db    0
.data:008F94D0 flt_8F94D0      dd 0.1                  ; DATA XREF: sub_6D33B0+1E4↑r
.data:008F94D0                                         ; sub_6D33B0+20D↑r ...
.data:008F94D4                 db    0
.data:008F94D5                 db    0
.data:008F94D6                 db  80h ; €
.data:008F94D7                 db  3Eh ; >
.data:008F94D8                 db    0
.data:008F94D9                 db    0
.data:008F94DA                 db  80h ; €
.data:008F94DB                 db  3Eh ; >
.data:008F94DC                 db    0
.data:008F94DD                 db    0
.data:008F94DE                 db  80h ; €
.data:008F94DF                 db  3Eh ; >
.data:008F94E0                 db    0
.data:008F94E1                 db    0
.data:008F94E2                 db  80h ; €
.data:008F94E3                 db  3Eh ; >
.data:008F94E4 ; float flt_8F94E4
.data:008F94E4 flt_8F94E4      dd 0.2                  ; DATA XREF: sub_6DE300+1529↑r
.data:008F94E4                                         ; sub_6DE300+1535↑r
.data:008F94E8 ; float flt_8F94E8
.data:008F94E8 flt_8F94E8      dd 1.0                  ; DATA XREF: sub_6DE300+151F↑r
.data:008F94EC ; int dword_8F94EC
.data:008F94EC dword_8F94EC    dd 3E99999Ah            ; DATA XREF: sub_6D33B0+1F6↑r
.data:008F94EC                                         ; sub_6DE300+142F↑w ...
.data:008F94F0 flt_8F94F0      dd 0.44999999           ; DATA XREF: sub_6C3570+1FA↑r
.data:008F94F0                                         ; sub_6C3570+269↑r ...
.data:008F94F4 unk_8F94F4      db    0                 ; DATA XREF: sub_6D40E0+A↑o
.data:008F94F4                                         ; sub_6D4620+54↑o ...
.data:008F94F5                 db    0
.data:008F94F6                 db  80h ; €
.data:008F94F7                 db  3Fh ; ?
.data:008F94F8                 db    0
.data:008F94F9                 db    0
.data:008F94FA                 db    0
.data:008F94FB                 db    0
.data:008F94FC                 db    0
.data:008F94FD                 db    0
.data:008F94FE                 db    0
.data:008F94FF                 db    0
.data:008F9500 dword_8F9500    dd 0.33333334           ; DATA XREF: sub_6D4960+28↑o
.data:008F9500                                         ; sub_6DBFE0+38↑o ...
.data:008F9504                 dd 0.0
.data:008F9508                 dd 1.5
.data:008F950C                 dd 0.16666667
.data:008F9510                 dd 0.0
.data:008F9514                 dd 3.0
.data:008F9518                 dd 0.33333334
.data:008F951C                 dd 0.0
.data:008F9520                 dd -1.5
.data:008F9524                 dd 0.16666667
.data:008F9528                 dd 0.0
.data:008F952C                 dd -3.0
.data:008F9530 dword_8F9530    dd 0.33333334           ; DATA XREF: sub_6D4960+D↑o
.data:008F9530                                         ; sub_6DBFE0+1D↑o ...
.data:008F9534                 dd 1.5
.data:008F9538                 dd 0.0
.data:008F953C                 dd 0.16666667
.data:008F9540                 dd 3.0
.data:008F9544                 dd 0.0
.data:008F9548                 dd 0.33333334
.data:008F954C                 dd -1.5
.data:008F9550                 dd 0.0
.data:008F9554                 dd 0.16666667
.data:008F9558                 dd -3.0
.data:008F955C                 dd 0.0
.data:008F9560 dword_8F9560    dd 0.25                 ; DATA XREF: sub_6DBEC0+D0↑o
.data:008F9560                                         ; sub_6DBEC0+F7↑o
.data:008F9564                 dd -0.5
.data:008F9568                 dd -0.5
.data:008F956C                 dd 0.25
.data:008F9570                 dd 0.5
.data:008F9574                 dd -0.5
.data:008F9578                 dd 0.25
.data:008F957C                 dd -0.5
.data:008F9580                 dd 0.5
.data:008F9584                 dd 0.25
.data:008F9588                 dd 0.5
.data:008F958C                 dd 0.5
.data:008F9590 flt_8F9590      dd 0.25                 ; DATA XREF: sub_6DC040+11↑o
.data:008F9594                 dd 0.0
.data:008F9598                 dd 0.0
.data:008F959C                 dd 0.25
.data:008F95A0                 dd 0.0
.data:008F95A4                 dd 0.0
.data:008F95A8                 dd 0.25
.data:008F95AC                 dd 0.0
.data:008F95B0                 dd 0.0
.data:008F95B4                 dd 0.25
.data:008F95B8                 dd 0.0
.data:008F95BC                 dd 0.0
.data:008F95C0                 dd 0.25
.data:008F95C4                 dd 0.0
.data:008F95C8                 dd 0.0
.data:008F95CC                 dd 0.25
.data:008F95D0                 dd 1.0
.data:008F95D4                 dd 1.0
.data:008F95D8                 dd 0.25
.data:008F95DC                 dd 2.0
.data:008F95E0                 dd 2.0
.data:008F95E4                 dd 0.25
.data:008F95E8                 dd 3.0
.data:008F95EC                 dd 3.0
.data:008F95F0                 dd 0.2
.data:008F95F4                 dd 0.0
.data:008F95F8                 dd 0.0
.data:008F95FC                 dd 0.2
.data:008F9600                 db    0
.data:008F9601                 db    0
.data:008F9602                 db  80h ; €
.data:008F9603                 db  40h ; @
.data:008F9604                 db    0
.data:008F9605                 db    0
.data:008F9606                 db  80h ; €
.data:008F9607                 db  40h ; @
.data:008F9608                 db 0CDh ; Í
.data:008F9609                 db 0CCh ; Ì
.data:008F960A                 db  4Ch ; L
.data:008F960B                 db  3Eh ; >
.data:008F960C                 db    0
.data:008F960D                 db    0
.data:008F960E                 db    0
.data:008F960F                 db  41h ; A
.data:008F9610                 db    0
.data:008F9611                 db    0
.data:008F9612                 db    0
.data:008F9613                 db  41h ; A
.data:008F9614                 db 0CDh ; Í
.data:008F9615                 db 0CCh ; Ì
.data:008F9616                 db  4Ch ; L
.data:008F9617                 db  3Eh ; >
.data:008F9618                 db    0
.data:008F9619                 db    0
.data:008F961A                 db  40h ; @
.data:008F961B                 db  41h ; A
.data:008F961C                 db    0
.data:008F961D                 db    0
.data:008F961E                 db  40h ; @
.data:008F961F                 db  41h ; A
.data:008F9620                 db  9Ah ; š
.data:008F9621                 db  99h ; ™
.data:008F9622                 db  19h
.data:008F9623                 db  3Eh ; >
.data:008F9624                 db    0
.data:008F9625                 db    0
.data:008F9626                 db    0
.data:008F9627                 db    0
.data:008F9628                 db    0
.data:008F9629                 db    0
.data:008F962A                 db    0
.data:008F962B                 db    0
.data:008F962C                 db  9Ah ; š
.data:008F962D                 db  99h ; ™
.data:008F962E                 db  19h
.data:008F962F                 db  3Eh ; >
.data:008F9630                 db    0
.data:008F9631                 db    0
.data:008F9632                 db  80h ; €
.data:008F9633                 db  41h ; A
.data:008F9634                 db    0
.data:008F9635                 db    0
.data:008F9636                 db  80h ; €
.data:008F9637                 db  41h ; A
.data:008F9638                 db  9Ah ; š
.data:008F9639                 db  99h ; ™
.data:008F963A                 db  19h
.data:008F963B                 db  3Eh ; >
.data:008F963C                 db    0
.data:008F963D                 db    0
.data:008F963E                 db    0
.data:008F963F                 db  42h ; B
.data:008F9640                 db    0
.data:008F9641                 db    0
.data:008F9642                 db    0
.data:008F9643                 db  42h ; B
.data:008F9644                 db  9Ah ; š
.data:008F9645                 db  99h ; ™
.data:008F9646                 db  19h
.data:008F9647                 db  3Eh ; >
.data:008F9648                 db    0
.data:008F9649                 db    0
.data:008F964A                 db  40h ; @
.data:008F964B                 db  42h ; B
.data:008F964C                 db    0
.data:008F964D                 db    0
.data:008F964E                 db  40h ; @
.data:008F964F                 db  42h ; B
.data:008F9650                 db    0
.data:008F9651                 db    0
.data:008F9652                 db  80h ; €
.data:008F9653                 db  3Eh ; >
.data:008F9654                 db    0
.data:008F9655                 db    0
.data:008F9656                 db    0
.data:008F9657                 db    0
.data:008F9658                 db    0
.data:008F9659                 db    0
.data:008F965A                 db    0
.data:008F965B                 db    0
.data:008F965C                 db    0
.data:008F965D                 db    0
.data:008F965E                 db  80h ; €
.data:008F965F                 db  3Eh ; >
.data:008F9660                 db    0
.data:008F9661                 db    0
.data:008F9662                 db  80h ; €
.data:008F9663                 db 0BFh ; ¿
.data:008F9664                 db    0
.data:008F9665                 db    0
.data:008F9666                 db  80h ; €
.data:008F9667                 db  3Fh ; ?
.data:008F9668                 db    0
.data:008F9669                 db    0
.data:008F966A                 db  80h ; €
.data:008F966B                 db  3Eh ; >
.data:008F966C                 db    0
.data:008F966D                 db    0
.data:008F966E                 db    0
.data:008F966F                 db 0C0h ; À
.data:008F9670                 db    0
.data:008F9671                 db    0
.data:008F9672                 db    0
.data:008F9673                 db  40h ; @
.data:008F9674                 db    0
.data:008F9675                 db    0
.data:008F9676                 db  80h ; €
.data:008F9677                 db  3Eh ; >
.data:008F9678                 db    0
.data:008F9679                 db    0
.data:008F967A                 db  40h ; @
.data:008F967B                 db 0C0h ; À
.data:008F967C                 db    0
.data:008F967D                 db    0
.data:008F967E                 db  40h ; @
.data:008F967F                 db  40h ; @
.data:008F9680                 db 0CDh ; Í
.data:008F9681                 db 0CCh ; Ì
.data:008F9682                 db  4Ch ; L
.data:008F9683                 db  3Eh ; >
.data:008F9684                 db    0
.data:008F9685                 db    0
.data:008F9686                 db    0
.data:008F9687                 db    0
.data:008F9688                 db    0
.data:008F9689                 db    0
.data:008F968A                 db    0
.data:008F968B                 db    0
.data:008F968C                 db 0CDh ; Í
.data:008F968D                 db 0CCh ; Ì
.data:008F968E                 db  4Ch ; L
.data:008F968F                 db  3Eh ; >
.data:008F9690                 db    0
.data:008F9691                 db    0
.data:008F9692                 db  80h ; €
.data:008F9693                 db 0C0h ; À
.data:008F9694                 db    0
.data:008F9695                 db    0
.data:008F9696                 db  80h ; €
.data:008F9697                 db  40h ; @
.data:008F9698                 db 0CDh ; Í
.data:008F9699                 db 0CCh ; Ì
.data:008F969A                 db  4Ch ; L
.data:008F969B                 db  3Eh ; >
.data:008F969C                 db    0
.data:008F969D                 db    0
.data:008F969E                 db    0
.data:008F969F                 db 0C1h ; Á
.data:008F96A0                 db    0
.data:008F96A1                 db    0
.data:008F96A2                 db    0
.data:008F96A3                 db  41h ; A
.data:008F96A4                 db 0CDh ; Í
.data:008F96A5                 db 0CCh ; Ì
.data:008F96A6                 db  4Ch ; L
.data:008F96A7                 db  3Eh ; >
.data:008F96A8                 db    0
.data:008F96A9                 db    0
.data:008F96AA                 db  40h ; @
.data:008F96AB                 db 0C1h ; Á
.data:008F96AC                 db    0
.data:008F96AD                 db    0
.data:008F96AE                 db  40h ; @
.data:008F96AF                 db  41h ; A
.data:008F96B0                 db  9Ah ; š
.data:008F96B1                 db  99h ; ™
.data:008F96B2                 db  19h
.data:008F96B3                 db  3Eh ; >
.data:008F96B4                 db    0
.data:008F96B5                 db    0
.data:008F96B6                 db    0
.data:008F96B7                 db    0
.data:008F96B8                 db    0
.data:008F96B9                 db    0
.data:008F96BA                 db    0
.data:008F96BB                 db    0
.data:008F96BC                 db  9Ah ; š
.data:008F96BD                 db  99h ; ™
.data:008F96BE                 db  19h
.data:008F96BF                 db  3Eh ; >
.data:008F96C0                 db    0
.data:008F96C1                 db    0
.data:008F96C2                 db  80h ; €
.data:008F96C3                 db 0C1h ; Á
.data:008F96C4                 db    0
.data:008F96C5                 db    0
.data:008F96C6                 db  80h ; €
.data:008F96C7                 db  41h ; A
.data:008F96C8                 db  9Ah ; š
.data:008F96C9                 db  99h ; ™
.data:008F96CA                 db  19h
.data:008F96CB                 db  3Eh ; >
.data:008F96CC                 db    0
.data:008F96CD                 db    0
.data:008F96CE                 db    0
.data:008F96CF                 db 0C2h ; Â
.data:008F96D0                 db    0
.data:008F96D1                 db    0
.data:008F96D2                 db    0
.data:008F96D3                 db  42h ; B
.data:008F96D4                 db  9Ah ; š
.data:008F96D5                 db  99h ; ™
.data:008F96D6                 db  19h
.data:008F96D7                 db  3Eh ; >
.data:008F96D8                 db    0
.data:008F96D9                 db    0
.data:008F96DA                 db  40h ; @
.data:008F96DB                 db 0C2h ; Â
.data:008F96DC                 db    0
.data:008F96DD                 db    0
.data:008F96DE                 db  40h ; @
.data:008F96DF                 db  42h ; B
.data:008F96E0                 db    0
.data:008F96E1                 db    0
.data:008F96E2                 db  80h ; €
.data:008F96E3                 db  3Eh ; >
.data:008F96E4                 db    0
.data:008F96E5                 db    0
.data:008F96E6                 db    0
.data:008F96E7                 db    0
.data:008F96E8                 db    0
.data:008F96E9                 db    0
.data:008F96EA                 db    0
.data:008F96EB                 db    0
.data:008F96EC                 db    0
.data:008F96ED                 db    0
.data:008F96EE                 db  80h ; €
.data:008F96EF                 db  3Eh ; >
.data:008F96F0                 db    0
.data:008F96F1                 db    0
.data:008F96F2                 db  80h ; €
.data:008F96F3                 db 0BFh ; ¿
.data:008F96F4                 db    0
.data:008F96F5                 db    0
.data:008F96F6                 db  80h ; €
.data:008F96F7                 db 0BFh ; ¿
.data:008F96F8                 db    0
.data:008F96F9                 db    0
.data:008F96FA                 db  80h ; €
.data:008F96FB                 db  3Eh ; >
.data:008F96FC                 db    0
.data:008F96FD                 db    0
.data:008F96FE                 db    0
.data:008F96FF                 db 0C0h ; À
.data:008F9700                 db    0
.data:008F9701                 db    0
.data:008F9702                 db    0
.data:008F9703                 db 0C0h ; À
.data:008F9704                 db    0
.data:008F9705                 db    0
.data:008F9706                 db  80h ; €
.data:008F9707                 db  3Eh ; >
.data:008F9708                 db    0
.data:008F9709                 db    0
.data:008F970A                 db  40h ; @
.data:008F970B                 db 0C0h ; À
.data:008F970C                 db    0
.data:008F970D                 db    0
.data:008F970E                 db  40h ; @
.data:008F970F                 db 0C0h ; À
.data:008F9710                 db 0CDh ; Í
.data:008F9711                 db 0CCh ; Ì
.data:008F9712                 db  4Ch ; L
.data:008F9713                 db  3Eh ; >
.data:008F9714                 db    0
.data:008F9715                 db    0
.data:008F9716                 db    0
.data:008F9717                 db    0
.data:008F9718                 db    0
.data:008F9719                 db    0
.data:008F971A                 db    0
.data:008F971B                 db    0
.data:008F971C                 db 0CDh ; Í
.data:008F971D                 db 0CCh ; Ì
.data:008F971E                 db  4Ch ; L
.data:008F971F                 db  3Eh ; >
.data:008F9720                 db    0
.data:008F9721                 db    0
.data:008F9722                 db  80h ; €
.data:008F9723                 db 0C0h ; À
.data:008F9724                 db    0
.data:008F9725                 db    0
.data:008F9726                 db  80h ; €
.data:008F9727                 db 0C0h ; À
.data:008F9728                 db 0CDh ; Í
.data:008F9729                 db 0CCh ; Ì
.data:008F972A                 db  4Ch ; L
.data:008F972B                 db  3Eh ; >
.data:008F972C                 db    0
.data:008F972D                 db    0
.data:008F972E                 db    0
.data:008F972F                 db 0C1h ; Á
.data:008F9730                 db    0
.data:008F9731                 db    0
.data:008F9732                 db    0
.data:008F9733                 db 0C1h ; Á
.data:008F9734                 db 0CDh ; Í
.data:008F9735                 db 0CCh ; Ì
.data:008F9736                 db  4Ch ; L
.data:008F9737                 db  3Eh ; >
.data:008F9738                 db    0
.data:008F9739                 db    0
.data:008F973A                 db  40h ; @
.data:008F973B                 db 0C1h ; Á
.data:008F973C                 db    0
.data:008F973D                 db    0
.data:008F973E                 db  40h ; @
.data:008F973F                 db 0C1h ; Á
.data:008F9740                 db  9Ah ; š
.data:008F9741                 db  99h ; ™
.data:008F9742                 db  19h
.data:008F9743                 db  3Eh ; >
.data:008F9744                 db    0
.data:008F9745                 db    0
.data:008F9746                 db    0
.data:008F9747                 db    0
.data:008F9748                 db    0
.data:008F9749                 db    0
.data:008F974A                 db    0
.data:008F974B                 db    0
.data:008F974C                 db  9Ah ; š
.data:008F974D                 db  99h ; ™
.data:008F974E                 db  19h
.data:008F974F                 db  3Eh ; >
.data:008F9750                 db    0
.data:008F9751                 db    0
.data:008F9752                 db  80h ; €
.data:008F9753                 db 0C1h ; Á
.data:008F9754                 db    0
.data:008F9755                 db    0
.data:008F9756                 db  80h ; €
.data:008F9757                 db 0C1h ; Á
.data:008F9758                 db  9Ah ; š
.data:008F9759                 db  99h ; ™
.data:008F975A                 db  19h
.data:008F975B                 db  3Eh ; >
.data:008F975C                 db    0
.data:008F975D                 db    0
.data:008F975E                 db    0
.data:008F975F                 db 0C2h ; Â
.data:008F9760                 db    0
.data:008F9761                 db    0
.data:008F9762                 db    0
.data:008F9763                 db 0C2h ; Â
.data:008F9764                 db  9Ah ; š
.data:008F9765                 db  99h ; ™
.data:008F9766                 db  19h
.data:008F9767                 db  3Eh ; >
.data:008F9768                 db    0
.data:008F9769                 db    0
.data:008F976A                 db  40h ; @
.data:008F976B                 db 0C2h ; Â
.data:008F976C                 db    0
.data:008F976D                 db    0
.data:008F976E                 db  40h ; @
.data:008F976F                 db 0C2h ; Â
.data:008F9770                 db    0
.data:008F9771                 db    0
.data:008F9772                 db  80h ; €
.data:008F9773                 db  3Eh ; >
.data:008F9774                 db    0
.data:008F9775                 db    0
.data:008F9776                 db    0
.data:008F9777                 db    0
.data:008F9778                 db    0
.data:008F9779                 db    0
.data:008F977A                 db    0
.data:008F977B                 db    0
.data:008F977C                 db    0
.data:008F977D                 db    0
.data:008F977E                 db  80h ; €
.data:008F977F                 db  3Eh ; >
.data:008F9780                 db    0
.data:008F9781                 db    0
.data:008F9782                 db  80h ; €
.data:008F9783                 db  3Fh ; ?
.data:008F9784                 db    0
.data:008F9785                 db    0
.data:008F9786                 db  80h ; €
.data:008F9787                 db 0BFh ; ¿
.data:008F9788                 db    0
.data:008F9789                 db    0
.data:008F978A                 db  80h ; €
.data:008F978B                 db  3Eh ; >
.data:008F978C                 db    0
.data:008F978D                 db    0
.data:008F978E                 db    0
.data:008F978F                 db  40h ; @
.data:008F9790                 db    0
.data:008F9791                 db    0
.data:008F9792                 db    0
.data:008F9793                 db 0C0h ; À
.data:008F9794                 db    0
.data:008F9795                 db    0
.data:008F9796                 db  80h ; €
.data:008F9797                 db  3Eh ; >
.data:008F9798                 db    0
.data:008F9799                 db    0
.data:008F979A                 db  40h ; @
.data:008F979B                 db  40h ; @
.data:008F979C                 db    0
.data:008F979D                 db    0
.data:008F979E                 db  40h ; @
.data:008F979F                 db 0C0h ; À
.data:008F97A0                 db 0CDh ; Í
.data:008F97A1                 db 0CCh ; Ì
.data:008F97A2                 db  4Ch ; L
.data:008F97A3                 db  3Eh ; >
.data:008F97A4                 db    0
.data:008F97A5                 db    0
.data:008F97A6                 db    0
.data:008F97A7                 db    0
.data:008F97A8                 db    0
.data:008F97A9                 db    0
.data:008F97AA                 db    0
.data:008F97AB                 db    0
.data:008F97AC                 db 0CDh ; Í
.data:008F97AD                 db 0CCh ; Ì
.data:008F97AE                 db  4Ch ; L
.data:008F97AF                 db  3Eh ; >
.data:008F97B0                 db    0
.data:008F97B1                 db    0
.data:008F97B2                 db  80h ; €
.data:008F97B3                 db  40h ; @
.data:008F97B4                 db    0
.data:008F97B5                 db    0
.data:008F97B6                 db  80h ; €
.data:008F97B7                 db 0C0h ; À
.data:008F97B8                 db 0CDh ; Í
.data:008F97B9                 db 0CCh ; Ì
.data:008F97BA                 db  4Ch ; L
.data:008F97BB                 db  3Eh ; >
.data:008F97BC                 db    0
.data:008F97BD                 db    0
.data:008F97BE                 db    0
.data:008F97BF                 db  41h ; A
.data:008F97C0                 db    0
.data:008F97C1                 db    0
.data:008F97C2                 db    0
.data:008F97C3                 db 0C1h ; Á
.data:008F97C4                 db 0CDh ; Í
.data:008F97C5                 db 0CCh ; Ì
.data:008F97C6                 db  4Ch ; L
.data:008F97C7                 db  3Eh ; >
.data:008F97C8                 db    0
.data:008F97C9                 db    0
.data:008F97CA                 db  40h ; @
.data:008F97CB                 db  41h ; A
.data:008F97CC                 db    0
.data:008F97CD                 db    0
.data:008F97CE                 db  40h ; @
.data:008F97CF                 db 0C1h ; Á
.data:008F97D0                 db  9Ah ; š
.data:008F97D1                 db  99h ; ™
.data:008F97D2                 db  19h
.data:008F97D3                 db  3Eh ; >
.data:008F97D4                 db    0
.data:008F97D5                 db    0
.data:008F97D6                 db    0
.data:008F97D7                 db    0
.data:008F97D8                 db    0
.data:008F97D9                 db    0
.data:008F97DA                 db    0
.data:008F97DB                 db    0
.data:008F97DC                 db  9Ah ; š
.data:008F97DD                 db  99h ; ™
.data:008F97DE                 db  19h
.data:008F97DF                 db  3Eh ; >
.data:008F97E0                 db    0
.data:008F97E1                 db    0
.data:008F97E2                 db  80h ; €
.data:008F97E3                 db  41h ; A
.data:008F97E4                 db    0
.data:008F97E5                 db    0
.data:008F97E6                 db  80h ; €
.data:008F97E7                 db 0C1h ; Á
.data:008F97E8                 db  9Ah ; š
.data:008F97E9                 db  99h ; ™
.data:008F97EA                 db  19h
.data:008F97EB                 db  3Eh ; >
.data:008F97EC                 db    0
.data:008F97ED                 db    0
.data:008F97EE                 db    0
.data:008F97EF                 db  42h ; B
.data:008F97F0                 db    0
.data:008F97F1                 db    0
.data:008F97F2                 db    0
.data:008F97F3                 db 0C2h ; Â
.data:008F97F4                 db  9Ah ; š
.data:008F97F5                 db  99h ; ™
.data:008F97F6                 db  19h
.data:008F97F7                 db  3Eh ; >
.data:008F97F8                 db    0
.data:008F97F9                 db    0
.data:008F97FA                 db  40h ; @
.data:008F97FB                 db  42h ; B
.data:008F97FC                 db    0
.data:008F97FD                 db    0
.data:008F97FE                 db  40h ; @
.data:008F97FF                 db 0C2h ; Â
.data:008F9800                 db    0
.data:008F9801                 db    0
.data:008F9802                 db  80h ; €
.data:008F9803                 db  3Eh ; >
.data:008F9804                 db    0
.data:008F9805                 db    0
.data:008F9806                 db  80h ; €
.data:008F9807                 db 0BFh ; ¿
.data:008F9808                 db    0
.data:008F9809                 db    0
.data:008F980A                 db  80h ; €
.data:008F980B                 db  3Fh ; ?
.data:008F980C                 db    0
.data:008F980D                 db    0
.data:008F980E                 db  80h ; €
.data:008F980F                 db  3Eh ; >
.data:008F9810                 db    0
.data:008F9811                 db    0
.data:008F9812                 db  80h ; €
.data:008F9813                 db 0BFh ; ¿
.data:008F9814                 db    0
.data:008F9815                 db    0
.data:008F9816                 db  80h ; €
.data:008F9817                 db 0BFh ; ¿
.data:008F9818                 db    0
.data:008F9819                 db    0
.data:008F981A                 db  80h ; €
.data:008F981B                 db  3Eh ; >
.data:008F981C                 db    0
.data:008F981D                 db    0
.data:008F981E                 db  80h ; €
.data:008F981F                 db  3Fh ; ?
.data:008F9820                 db    0
.data:008F9821                 db    0
.data:008F9822                 db  80h ; €
.data:008F9823                 db 0BFh ; ¿
.data:008F9824                 db    0
.data:008F9825                 db    0
.data:008F9826                 db  80h ; €
.data:008F9827                 db  3Eh ; >
.data:008F9828                 db    0
.data:008F9829                 db    0
.data:008F982A                 db  80h ; €
.data:008F982B                 db  3Fh ; ?
.data:008F982C                 db    0
.data:008F982D                 db    0
.data:008F982E                 db  80h ; €
.data:008F982F                 db  3Fh ; ?
.data:008F9830                 db    0
.data:008F9831                 db    0
.data:008F9832                 db  80h ; €
.data:008F9833                 db  3Eh ; >
.data:008F9834                 db    0
.data:008F9835                 db    0
.data:008F9836                 db    0
.data:008F9837                 db 0C0h ; À
.data:008F9838                 db    0
.data:008F9839                 db    0
.data:008F983A                 db    0
.data:008F983B                 db 0C0h ; À
.data:008F983C                 db    0
.data:008F983D                 db    0
.data:008F983E                 db  80h ; €
.data:008F983F                 db  3Eh ; >
.data:008F9840                 db    0
.data:008F9841                 db    0
.data:008F9842                 db    0
.data:008F9843                 db 0C0h ; À
.data:008F9844                 db    0
.data:008F9845                 db    0
.data:008F9846                 db    0
.data:008F9847                 db  40h ; @
.data:008F9848                 db    0
.data:008F9849                 db    0
.data:008F984A                 db  80h ; €
.data:008F984B                 db  3Eh ; >
.data:008F984C                 db    0
.data:008F984D                 db    0
.data:008F984E                 db    0
.data:008F984F                 db  40h ; @
.data:008F9850                 db    0
.data:008F9851                 db    0
.data:008F9852                 db    0
.data:008F9853                 db  40h ; @
.data:008F9854                 db    0
.data:008F9855                 db    0
.data:008F9856                 db  80h ; €
.data:008F9857                 db  3Eh ; >
.data:008F9858                 db    0
.data:008F9859                 db    0
.data:008F985A                 db    0
.data:008F985B                 db  40h ; @
.data:008F985C                 db    0
.data:008F985D                 db    0
.data:008F985E                 db    0
.data:008F985F                 db 0C0h ; À
.data:008F9860                 db    0
.data:008F9861                 db    0
.data:008F9862                 db  80h ; €
.data:008F9863                 db  3Eh ; >
.data:008F9864                 db    0
.data:008F9865                 db    0
.data:008F9866                 db  40h ; @
.data:008F9867                 db 0C0h ; À
.data:008F9868                 db    0
.data:008F9869                 db    0
.data:008F986A                 db  40h ; @
.data:008F986B                 db 0C0h ; À
.data:008F986C                 db    0
.data:008F986D                 db    0
.data:008F986E                 db  80h ; €
.data:008F986F                 db  3Eh ; >
.data:008F9870                 db    0
.data:008F9871                 db    0
.data:008F9872                 db  40h ; @
.data:008F9873                 db 0C0h ; À
.data:008F9874                 db    0
.data:008F9875                 db    0
.data:008F9876                 db  40h ; @
.data:008F9877                 db  40h ; @
.data:008F9878                 db    0
.data:008F9879                 db    0
.data:008F987A                 db  80h ; €
.data:008F987B                 db  3Eh ; >
.data:008F987C                 db    0
.data:008F987D                 db    0
.data:008F987E                 db  40h ; @
.data:008F987F                 db  40h ; @
.data:008F9880                 db    0
.data:008F9881                 db    0
.data:008F9882                 db  40h ; @
.data:008F9883                 db  40h ; @
.data:008F9884                 db    0
.data:008F9885                 db    0
.data:008F9886                 db  80h ; €
.data:008F9887                 db  3Eh ; >
.data:008F9888                 db    0
.data:008F9889                 db    0
.data:008F988A                 db  40h ; @
.data:008F988B                 db  40h ; @
.data:008F988C                 db    0
.data:008F988D                 db    0
.data:008F988E                 db  40h ; @
.data:008F988F                 db 0C0h ; À
.data:008F9890                 db    0
.data:008F9891                 db    0
.data:008F9892                 db  80h ; €
.data:008F9893                 db  3Eh ; >
.data:008F9894                 db    0
.data:008F9895                 db    0
.data:008F9896                 db    0
.data:008F9897                 db    0
.data:008F9898                 db    0
.data:008F9899                 db    0
.data:008F989A                 db    0
.data:008F989B                 db    0
.data:008F989C                 db    0
.data:008F989D                 db    0
.data:008F989E                 db  80h ; €
.data:008F989F                 db  3Eh ; >
.data:008F98A0                 db    0
.data:008F98A1                 db    0
.data:008F98A2                 db    0
.data:008F98A3                 db    0
.data:008F98A4                 db    0
.data:008F98A5                 db    0
.data:008F98A6                 db  80h ; €
.data:008F98A7                 db  3Fh ; ?
.data:008F98A8                 db    0
.data:008F98A9                 db    0
.data:008F98AA                 db  80h ; €
.data:008F98AB                 db  3Eh ; >
.data:008F98AC                 db    0
.data:008F98AD                 db    0
.data:008F98AE                 db    0
.data:008F98AF                 db    0
.data:008F98B0                 db    0
.data:008F98B1                 db    0
.data:008F98B2                 db    0
.data:008F98B3                 db  40h ; @
.data:008F98B4                 db    0
.data:008F98B5                 db    0
.data:008F98B6                 db  80h ; €
.data:008F98B7                 db  3Eh ; >
.data:008F98B8                 db    0
.data:008F98B9                 db    0
.data:008F98BA                 db    0
.data:008F98BB                 db    0
.data:008F98BC                 db    0
.data:008F98BD                 db    0
.data:008F98BE                 db  40h ; @
.data:008F98BF                 db  40h ; @
.data:008F98C0                 db    0
.data:008F98C1                 db    0
.data:008F98C2                 db  80h ; €
.data:008F98C3                 db  3Eh ; >
.data:008F98C4                 db    0
.data:008F98C5                 db    0
.data:008F98C6                 db    0
.data:008F98C7                 db    0
.data:008F98C8                 db    0
.data:008F98C9                 db    0
.data:008F98CA                 db    0
.data:008F98CB                 db    0
.data:008F98CC                 db    0
.data:008F98CD                 db    0
.data:008F98CE                 db  80h ; €
.data:008F98CF                 db  3Eh ; >
.data:008F98D0                 db    0
.data:008F98D1                 db    0
.data:008F98D2                 db    0
.data:008F98D3                 db    0
.data:008F98D4                 db    0
.data:008F98D5                 db    0
.data:008F98D6                 db  80h ; €
.data:008F98D7                 db  40h ; @
.data:008F98D8                 db    0
.data:008F98D9                 db    0
.data:008F98DA                 db  80h ; €
.data:008F98DB                 db  3Eh ; >
.data:008F98DC                 db    0
.data:008F98DD                 db    0
.data:008F98DE                 db    0
.data:008F98DF                 db    0
.data:008F98E0                 db    0
.data:008F98E1                 db    0
.data:008F98E2                 db    0
.data:008F98E3                 db  41h ; A
.data:008F98E4                 db    0
.data:008F98E5                 db    0
.data:008F98E6                 db  80h ; €
.data:008F98E7                 db  3Eh ; >
.data:008F98E8                 db    0
.data:008F98E9                 db    0
.data:008F98EA                 db    0
.data:008F98EB                 db    0
.data:008F98EC                 db    0
.data:008F98ED                 db    0
.data:008F98EE                 db  40h ; @
.data:008F98EF                 db  41h ; A
.data:008F98F0                 db    0
.data:008F98F1                 db    0
.data:008F98F2                 db  80h ; €
.data:008F98F3                 db  3Eh ; >
.data:008F98F4                 db    0
.data:008F98F5                 db    0
.data:008F98F6                 db    0
.data:008F98F7                 db    0
.data:008F98F8                 db    0
.data:008F98F9                 db    0
.data:008F98FA                 db    0
.data:008F98FB                 db    0
.data:008F98FC                 db    0
.data:008F98FD                 db    0
.data:008F98FE                 db  80h ; €
.data:008F98FF                 db  3Eh ; >
.data:008F9900                 db    0
.data:008F9901                 db    0
.data:008F9902                 db    0
.data:008F9903                 db    0
.data:008F9904                 db    0
.data:008F9905                 db    0
.data:008F9906                 db  80h ; €
.data:008F9907                 db  41h ; A
.data:008F9908                 db    0
.data:008F9909                 db    0
.data:008F990A                 db  80h ; €
.data:008F990B                 db  3Eh ; >
.data:008F990C                 db    0
.data:008F990D                 db    0
.data:008F990E                 db    0
.data:008F990F                 db    0
.data:008F9910                 db    0
.data:008F9911                 db    0
.data:008F9912                 db    0
.data:008F9913                 db  42h ; B
.data:008F9914                 db    0
.data:008F9915                 db    0
.data:008F9916                 db  80h ; €
.data:008F9917                 db  3Eh ; >
.data:008F9918                 db    0
.data:008F9919                 db    0
.data:008F991A                 db    0
.data:008F991B                 db    0
.data:008F991C                 db    0
.data:008F991D                 db    0
.data:008F991E                 db  40h ; @
.data:008F991F                 db  42h ; B
.data:008F9920                 db    0
.data:008F9921                 db    0
.data:008F9922                 db  80h ; €
.data:008F9923                 db  3Eh ; >
.data:008F9924                 db    0
.data:008F9925                 db    0
.data:008F9926                 db    0
.data:008F9927                 db    0
.data:008F9928                 db    0
.data:008F9929                 db    0
.data:008F992A                 db    0
.data:008F992B                 db    0
.data:008F992C                 db    0
.data:008F992D                 db    0
.data:008F992E                 db  80h ; €
.data:008F992F                 db  3Eh ; >
.data:008F9930                 db    0
.data:008F9931                 db    0
.data:008F9932                 db    0
.data:008F9933                 db    0
.data:008F9934                 db    0
.data:008F9935                 db    0
.data:008F9936                 db  80h ; €
.data:008F9937                 db 0BFh ; ¿
.data:008F9938                 db    0
.data:008F9939                 db    0
.data:008F993A                 db  80h ; €
.data:008F993B                 db  3Eh ; >
.data:008F993C                 db    0
.data:008F993D                 db    0
.data:008F993E                 db    0
.data:008F993F                 db    0
.data:008F9940                 db    0
.data:008F9941                 db    0
.data:008F9942                 db    0
.data:008F9943                 db 0C0h ; À
.data:008F9944                 db    0
.data:008F9945                 db    0
.data:008F9946                 db  80h ; €
.data:008F9947                 db  3Eh ; >
.data:008F9948                 db    0
.data:008F9949                 db    0
.data:008F994A                 db    0
.data:008F994B                 db    0
.data:008F994C                 db    0
.data:008F994D                 db    0
.data:008F994E                 db  40h ; @
.data:008F994F                 db 0C0h ; À
.data:008F9950                 db    0
.data:008F9951                 db    0
.data:008F9952                 db  80h ; €
.data:008F9953                 db  3Eh ; >
.data:008F9954                 db    0
.data:008F9955                 db    0
.data:008F9956                 db    0
.data:008F9957                 db    0
.data:008F9958                 db    0
.data:008F9959                 db    0
.data:008F995A                 db    0
.data:008F995B                 db    0
.data:008F995C                 db    0
.data:008F995D                 db    0
.data:008F995E                 db  80h ; €
.data:008F995F                 db  3Eh ; >
.data:008F9960                 db    0
.data:008F9961                 db    0
.data:008F9962                 db    0
.data:008F9963                 db    0
.data:008F9964                 db    0
.data:008F9965                 db    0
.data:008F9966                 db  80h ; €
.data:008F9967                 db 0C0h ; À
.data:008F9968                 db    0
.data:008F9969                 db    0
.data:008F996A                 db  80h ; €
.data:008F996B                 db  3Eh ; >
.data:008F996C                 db    0
.data:008F996D                 db    0
.data:008F996E                 db    0
.data:008F996F                 db    0
.data:008F9970                 db    0
.data:008F9971                 db    0
.data:008F9972                 db    0
.data:008F9973                 db 0C1h ; Á
.data:008F9974                 db    0
.data:008F9975                 db    0
.data:008F9976                 db  80h ; €
.data:008F9977                 db  3Eh ; >
.data:008F9978                 db    0
.data:008F9979                 db    0
.data:008F997A                 db    0
.data:008F997B                 db    0
.data:008F997C                 db    0
.data:008F997D                 db    0
.data:008F997E                 db  40h ; @
.data:008F997F                 db 0C1h ; Á
.data:008F9980                 db    0
.data:008F9981                 db    0
.data:008F9982                 db  80h ; €
.data:008F9983                 db  3Eh ; >
.data:008F9984                 db    0
.data:008F9985                 db    0
.data:008F9986                 db    0
.data:008F9987                 db    0
.data:008F9988                 db    0
.data:008F9989                 db    0
.data:008F998A                 db    0
.data:008F998B                 db    0
.data:008F998C                 db    0
.data:008F998D                 db    0
.data:008F998E                 db  80h ; €
.data:008F998F                 db  3Eh ; >
.data:008F9990                 db    0
.data:008F9991                 db    0
.data:008F9992                 db    0
.data:008F9993                 db    0
.data:008F9994                 db    0
.data:008F9995                 db    0
.data:008F9996                 db  80h ; €
.data:008F9997                 db 0C1h ; Á
.data:008F9998                 db    0
.data:008F9999                 db    0
.data:008F999A                 db  80h ; €
.data:008F999B                 db  3Eh ; >
.data:008F999C                 db    0
.data:008F999D                 db    0
.data:008F999E                 db    0
.data:008F999F                 db    0
.data:008F99A0                 db    0
.data:008F99A1                 db    0
.data:008F99A2                 db    0
.data:008F99A3                 db 0C2h ; Â
.data:008F99A4                 db    0
.data:008F99A5                 db    0
.data:008F99A6                 db  80h ; €
.data:008F99A7                 db  3Eh ; >
.data:008F99A8                 db    0
.data:008F99A9                 db    0
.data:008F99AA                 db    0
.data:008F99AB                 db    0
.data:008F99AC                 db    0
.data:008F99AD                 db    0
.data:008F99AE                 db  40h ; @
.data:008F99AF                 db 0C2h ; Â
.data:008F99B0                 db    0
.data:008F99B1                 db    0
.data:008F99B2                 db  80h ; €
.data:008F99B3                 db  3Eh ; >
.data:008F99B4                 db 0CDh ; Í
.data:008F99B5                 db 0CCh ; Ì
.data:008F99B6                 db 0CCh ; Ì
.data:008F99B7                 db 0BDh ; ½
.data:008F99B8                 db    0
.data:008F99B9                 db    0
.data:008F99BA                 db    0
.data:008F99BB                 db    0
.data:008F99BC                 db    0
.data:008F99BD                 db    0
.data:008F99BE                 db  80h ; €
.data:008F99BF                 db  3Eh ; >
.data:008F99C0                 db 0CDh ; Í
.data:008F99C1                 db 0CCh ; Ì
.data:008F99C2                 db 0CCh ; Ì
.data:008F99C3                 db  3Dh ; =
.data:008F99C4                 db    0
.data:008F99C5                 db    0
.data:008F99C6                 db  80h ; €
.data:008F99C7                 db  3Fh ; ?
.data:008F99C8                 db    0
.data:008F99C9                 db    0
.data:008F99CA                 db  80h ; €
.data:008F99CB                 db  3Eh ; >
.data:008F99CC                 db 0CDh ; Í
.data:008F99CD                 db 0CCh ; Ì
.data:008F99CE                 db 0CCh ; Ì
.data:008F99CF                 db 0BDh ; ½
.data:008F99D0                 db    0
.data:008F99D1                 db    0
.data:008F99D2                 db    0
.data:008F99D3                 db  40h ; @
.data:008F99D4                 db    0
.data:008F99D5                 db    0
.data:008F99D6                 db  80h ; €
.data:008F99D7                 db  3Eh ; >
.data:008F99D8                 db 0CDh ; Í
.data:008F99D9                 db 0CCh ; Ì
.data:008F99DA                 db 0CCh ; Ì
.data:008F99DB                 db  3Dh ; =
.data:008F99DC                 db    0
.data:008F99DD                 db    0
.data:008F99DE                 db  40h ; @
.data:008F99DF                 db  40h ; @
.data:008F99E0                 db    0
.data:008F99E1                 db    0
.data:008F99E2                 db  80h ; €
.data:008F99E3                 db  3Eh ; >
.data:008F99E4                 db 0CDh ; Í
.data:008F99E5                 db 0CCh ; Ì
.data:008F99E6                 db 0CCh ; Ì
.data:008F99E7                 db  3Eh ; >
.data:008F99E8                 db    0
.data:008F99E9                 db    0
.data:008F99EA                 db    0
.data:008F99EB                 db    0
.data:008F99EC                 db    0
.data:008F99ED                 db    0
.data:008F99EE                 db  80h ; €
.data:008F99EF                 db  3Eh ; >
.data:008F99F0                 db 0CDh ; Í
.data:008F99F1                 db 0CCh ; Ì
.data:008F99F2                 db 0CCh ; Ì
.data:008F99F3                 db 0BEh ; ¾
.data:008F99F4                 db    0
.data:008F99F5                 db    0
.data:008F99F6                 db  80h ; €
.data:008F99F7                 db  40h ; @
.data:008F99F8                 db    0
.data:008F99F9                 db    0
.data:008F99FA                 db  80h ; €
.data:008F99FB                 db  3Eh ; >
.data:008F99FC                 db 0CDh ; Í
.data:008F99FD                 db 0CCh ; Ì
.data:008F99FE                 db 0CCh ; Ì
.data:008F99FF                 db  3Eh ; >
.data:008F9A00                 db    0
.data:008F9A01                 db    0
.data:008F9A02                 db    0
.data:008F9A03                 db  41h ; A
.data:008F9A04                 db    0
.data:008F9A05                 db    0
.data:008F9A06                 db  80h ; €
.data:008F9A07                 db  3Eh ; >
.data:008F9A08                 db 0CDh ; Í
.data:008F9A09                 db 0CCh ; Ì
.data:008F9A0A                 db 0CCh ; Ì
.data:008F9A0B                 db 0BEh ; ¾
.data:008F9A0C                 db    0
.data:008F9A0D                 db    0
.data:008F9A0E                 db  40h ; @
.data:008F9A0F                 db  41h ; A
.data:008F9A10                 db    0
.data:008F9A11                 db    0
.data:008F9A12                 db  80h ; €
.data:008F9A13                 db  3Eh ; >
.data:008F9A14                 db 0CDh ; Í
.data:008F9A15                 db 0CCh ; Ì
.data:008F9A16                 db 0CCh ; Ì
.data:008F9A17                 db 0BFh ; ¿
.data:008F9A18                 db    0
.data:008F9A19                 db    0
.data:008F9A1A                 db    0
.data:008F9A1B                 db    0
.data:008F9A1C                 db    0
.data:008F9A1D                 db    0
.data:008F9A1E                 db  80h ; €
.data:008F9A1F                 db  3Eh ; >
.data:008F9A20                 db 0CDh ; Í
.data:008F9A21                 db 0CCh ; Ì
.data:008F9A22                 db 0CCh ; Ì
.data:008F9A23                 db  3Fh ; ?
.data:008F9A24                 db    0
.data:008F9A25                 db    0
.data:008F9A26                 db  80h ; €
.data:008F9A27                 db  41h ; A
.data:008F9A28                 db    0
.data:008F9A29                 db    0
.data:008F9A2A                 db  80h ; €
.data:008F9A2B                 db  3Eh ; >
.data:008F9A2C                 db 0CDh ; Í
.data:008F9A2D                 db 0CCh ; Ì
.data:008F9A2E                 db 0CCh ; Ì
.data:008F9A2F                 db 0BFh ; ¿
.data:008F9A30                 db    0
.data:008F9A31                 db    0
.data:008F9A32                 db    0
.data:008F9A33                 db  42h ; B
.data:008F9A34                 db    0
.data:008F9A35                 db    0
.data:008F9A36                 db  80h ; €
.data:008F9A37                 db  3Eh ; >
.data:008F9A38                 db 0CDh ; Í
.data:008F9A39                 db 0CCh ; Ì
.data:008F9A3A                 db 0CCh ; Ì
.data:008F9A3B                 db  3Fh ; ?
.data:008F9A3C                 db    0
.data:008F9A3D                 db    0
.data:008F9A3E                 db  40h ; @
.data:008F9A3F                 db  42h ; B
.data:008F9A40                 db    0
.data:008F9A41                 db    0
.data:008F9A42                 db  80h ; €
.data:008F9A43                 db  3Eh ; >
.data:008F9A44                 db 0CDh ; Í
.data:008F9A45                 db 0CCh ; Ì
.data:008F9A46                 db 0CCh ; Ì
.data:008F9A47                 db  3Dh ; =
.data:008F9A48                 db    0
.data:008F9A49                 db    0
.data:008F9A4A                 db    0
.data:008F9A4B                 db    0
.data:008F9A4C                 db    0
.data:008F9A4D                 db    0
.data:008F9A4E                 db  80h ; €
.data:008F9A4F                 db  3Eh ; >
.data:008F9A50                 db 0CDh ; Í
.data:008F9A51                 db 0CCh ; Ì
.data:008F9A52                 db 0CCh ; Ì
.data:008F9A53                 db 0BDh ; ½
.data:008F9A54                 db    0
.data:008F9A55                 db    0
.data:008F9A56                 db  80h ; €
.data:008F9A57                 db 0BFh ; ¿
.data:008F9A58                 db    0
.data:008F9A59                 db    0
.data:008F9A5A                 db  80h ; €
.data:008F9A5B                 db  3Eh ; >
.data:008F9A5C                 db 0CDh ; Í
.data:008F9A5D                 db 0CCh ; Ì
.data:008F9A5E                 db 0CCh ; Ì
.data:008F9A5F                 db  3Dh ; =
.data:008F9A60                 db    0
.data:008F9A61                 db    0
.data:008F9A62                 db    0
.data:008F9A63                 db 0C0h ; À
.data:008F9A64                 db    0
.data:008F9A65                 db    0
.data:008F9A66                 db  80h ; €
.data:008F9A67                 db  3Eh ; >
.data:008F9A68                 db 0CDh ; Í
.data:008F9A69                 db 0CCh ; Ì
.data:008F9A6A                 db 0CCh ; Ì
.data:008F9A6B                 db 0BDh ; ½
.data:008F9A6C                 db    0
.data:008F9A6D                 db    0
.data:008F9A6E                 db  40h ; @
.data:008F9A6F                 db 0C0h ; À
.data:008F9A70                 db    0
.data:008F9A71                 db    0
.data:008F9A72                 db  80h ; €
.data:008F9A73                 db  3Eh ; >
.data:008F9A74                 db 0CDh ; Í
.data:008F9A75                 db 0CCh ; Ì
.data:008F9A76                 db 0CCh ; Ì
.data:008F9A77                 db 0BEh ; ¾
.data:008F9A78                 db    0
.data:008F9A79                 db    0
.data:008F9A7A                 db    0
.data:008F9A7B                 db    0
.data:008F9A7C                 db    0
.data:008F9A7D                 db    0
.data:008F9A7E                 db  80h ; €
.data:008F9A7F                 db  3Eh ; >
.data:008F9A80                 db 0CDh ; Í
.data:008F9A81                 db 0CCh ; Ì
.data:008F9A82                 db 0CCh ; Ì
.data:008F9A83                 db  3Eh ; >
.data:008F9A84                 db    0
.data:008F9A85                 db    0
.data:008F9A86                 db  80h ; €
.data:008F9A87                 db 0C0h ; À
.data:008F9A88                 db    0
.data:008F9A89                 db    0
.data:008F9A8A                 db  80h ; €
.data:008F9A8B                 db  3Eh ; >
.data:008F9A8C                 db 0CDh ; Í
.data:008F9A8D                 db 0CCh ; Ì
.data:008F9A8E                 db 0CCh ; Ì
.data:008F9A8F                 db 0BEh ; ¾
.data:008F9A90                 db    0
.data:008F9A91                 db    0
.data:008F9A92                 db    0
.data:008F9A93                 db 0C1h ; Á
.data:008F9A94                 db    0
.data:008F9A95                 db    0
.data:008F9A96                 db  80h ; €
.data:008F9A97                 db  3Eh ; >
.data:008F9A98                 db 0CDh ; Í
.data:008F9A99                 db 0CCh ; Ì
.data:008F9A9A                 db 0CCh ; Ì
.data:008F9A9B                 db  3Eh ; >
.data:008F9A9C                 db    0
.data:008F9A9D                 db    0
.data:008F9A9E                 db  40h ; @
.data:008F9A9F                 db 0C1h ; Á
.data:008F9AA0                 db    0
.data:008F9AA1                 db    0
.data:008F9AA2                 db  80h ; €
.data:008F9AA3                 db  3Eh ; >
.data:008F9AA4                 db 0CDh ; Í
.data:008F9AA5                 db 0CCh ; Ì
.data:008F9AA6                 db 0CCh ; Ì
.data:008F9AA7                 db  3Fh ; ?
.data:008F9AA8                 db    0
.data:008F9AA9                 db    0
.data:008F9AAA                 db    0
.data:008F9AAB                 db    0
.data:008F9AAC                 db    0
.data:008F9AAD                 db    0
.data:008F9AAE                 db  80h ; €
.data:008F9AAF                 db  3Eh ; >
.data:008F9AB0                 db 0CDh ; Í
.data:008F9AB1                 db 0CCh ; Ì
.data:008F9AB2                 db 0CCh ; Ì
.data:008F9AB3                 db 0BFh ; ¿
.data:008F9AB4                 db    0
.data:008F9AB5                 db    0
.data:008F9AB6                 db  80h ; €
.data:008F9AB7                 db 0C1h ; Á
.data:008F9AB8                 db    0
.data:008F9AB9                 db    0
.data:008F9ABA                 db  80h ; €
.data:008F9ABB                 db  3Eh ; >
.data:008F9ABC                 db 0CDh ; Í
.data:008F9ABD                 db 0CCh ; Ì
.data:008F9ABE                 db 0CCh ; Ì
.data:008F9ABF                 db  3Fh ; ?
.data:008F9AC0                 db    0
.data:008F9AC1                 db    0
.data:008F9AC2                 db    0
.data:008F9AC3                 db 0C2h ; Â
.data:008F9AC4                 db    0
.data:008F9AC5                 db    0
.data:008F9AC6                 db  80h ; €
.data:008F9AC7                 db  3Eh ; >
.data:008F9AC8                 db 0CDh ; Í
.data:008F9AC9                 db 0CCh ; Ì
.data:008F9ACA                 db 0CCh ; Ì
.data:008F9ACB                 db 0BFh ; ¿
.data:008F9ACC                 db    0
.data:008F9ACD                 db    0
.data:008F9ACE                 db  40h ; @
.data:008F9ACF                 db 0C2h ; Â
.data:008F9AD0 dword_8F9AD0    dd 442h                 ; DATA XREF: sub_6D2840+38C↑r
.data:008F9AD4                 db    2
.data:008F9AD5                 db    2
.data:008F9AD6                 db    0
.data:008F9AD7                 db    0
.data:008F9AD8 dword_8F9AD8    dd 15h                  ; DATA XREF: sub_6C2E60:loc_6C2EF0↑r
.data:008F9AD8                                         ; sub_6C2E60:loc_6C31C0↑r
.data:008F9ADC dword_8F9ADC    dd 0FFFFFFFFh           ; DATA XREF: sub_6C2E60+D↑w
.data:008F9ADC                                         ; sub_6C2E60+B2↑r
.data:008F9AE0 dword_8F9AE0    dd 0FFFFFFFFh           ; DATA XREF: sub_6C2E60+1B↑w
.data:008F9AE0                                         ; sub_6C2E60+A7↑r
.data:008F9AE4                 db  15h
.data:008F9AE5                 db    0
.data:008F9AE6                 db    0
.data:008F9AE7                 db    0
.data:008F9AE8 dword_8F9AE8    dd 15h                  ; DATA XREF: sub_6C2E60+1A0↑r
.data:008F9AE8                                         ; sub_6C2E60+23D↑r
.data:008F9AEC dword_8F9AEC    dd 15h                  ; DATA XREF: sub_6C2E60+450↑r
.data:008F9AEC                                         ; sub_6C2E60+47C↑r
.data:008F9AF0 dword_8F9AF0    dd 20h                  ; DATA XREF: sub_6C2E60+466↑r
.data:008F9AF0                                         ; sub_6C3570+1D↑r ...
.data:008F9AF4                 db  15h
.data:008F9AF5                 db    0
.data:008F9AF6                 db    0
.data:008F9AF7                 db    0
.data:008F9AF8 dword_8F9AF8    dd 100h                 ; DATA XREF: sub_6C2E60+492↑r
.data:008F9AF8                                         ; sub_6C3570+19D↑r ...
.data:008F9AFC dword_8F9AFC    dd 15h                  ; DATA XREF: sub_6C2E60+4A2↑r
.data:008F9B00 dword_8F9B00    dd 100h                 ; DATA XREF: sub_6C2E60+4B8↑r
.data:008F9B04 dword_8F9B04    dd 15h                  ; DATA XREF: sub_6C2E60+4C8↑r
.data:008F9B08 dword_8F9B08    dd 100h                 ; DATA XREF: sub_6C2E60+4DE↑r
.data:008F9B08                                         ; sub_6C3B20+25↑r ...
.data:008F9B0C dword_8F9B0C    dd 0.40000001           ; DATA XREF: sub_6C3440:loc_6C3473↑r
.data:008F9B0C                                         ; sub_6C3440+46↑w ...
.data:008F9B10 dword_8F9B10    dd 25.0                 ; DATA XREF: sub_6DBB20:loc_6DBC71↑r
.data:008F9B14 dword_8F9B14    dd 125.0                ; DATA XREF: sub_6DBB20+15A↑r
.data:008F9B18 flt_8F9B18      dd 0.64999998           ; DATA XREF: sub_6DBB20+1B2↑r
.data:008F9B18                                         ; sub_6DBB20+1BF↑r
.data:008F9B1C flt_8F9B1C      dd 0.0062500001         ; DATA XREF: sub_6DBB20+20F↑r
.data:008F9B20 flt_8F9B20      dd 0.02                 ; DATA XREF: sub_6DBB20+5E↑r
.data:008F9B24 flt_8F9B24      dd 0.0099999998         ; DATA XREF: sub_6DBB20+78↑r
.data:008F9B28 byte_8F9B28     db 1                    ; DATA XREF: sub_46E2D0+30↑w
.data:008F9B28                                         ; sub_473CD0+5F↑w ...
.data:008F9B29                 align 4
.data:008F9B2C flt_8F9B2C      dd 0.80000001           ; DATA XREF: sub_6DC040+99↑r
.data:008F9B30 flt_8F9B30      dd 0.60000002           ; DATA XREF: sub_6DC040:loc_6DC0E1↑r
.data:008F9B34                 db  60h ; `
.data:008F9B35                 db    0
.data:008F9B36                 db    0
.data:008F9B37                 db    0
.data:008F9B38 unk_8F9B38      db    0                 ; DATA XREF: sub_6D2CD0+553↑o
.data:008F9B39                 db    0
.data:008F9B3A                 db    0
.data:008F9B3B                 db  3Fh ; ?
.data:008F9B3C                 db    0
.data:008F9B3D                 db    0
.data:008F9B3E                 db    0
.data:008F9B3F                 db  3Fh ; ?
.data:008F9B40                 db    0
.data:008F9B41                 db    0
.data:008F9B42                 db    0
.data:008F9B43                 db  3Fh ; ?
.data:008F9B44                 db    0
.data:008F9B45                 db    0
.data:008F9B46                 db    0
.data:008F9B47                 db  3Fh ; ?
.data:008F9B48 flt_8F9B48      dd 0.30000001           ; DATA XREF: sub_6D2CD0+588↑r
.data:008F9B4C flt_8F9B4C      dd 0.2                  ; DATA XREF: sub_6D2CD0+57C↑r
.data:008F9B50 flt_8F9B50      dd 0.34999999           ; DATA XREF: sub_6D2CD0+5BC↑r
.data:008F9B54 flt_8F9B54      dd 0.5                  ; DATA XREF: sub_6D2CD0:loc_6D327B↑r
.data:008F9B58 flt_8F9B58      dd 0.2                  ; DATA XREF: sub_6C3870+17↑r
.data:008F9B5C ; char byte_8F9B5C
.data:008F9B5C byte_8F9B5C     db 1                    ; DATA XREF: .text:006D57C0↑r
.data:008F9B5C                                         ; sub_6DE300+1570↑r
.data:008F9B5D byte_8F9B5D     db 1                    ; DATA XREF: sub_6D4EF0+3A↑r
.data:008F9B5D                                         ; sub_6D4EF0+165↑r ...
.data:008F9B5E                 align 10h
.data:008F9B60 ; char *off_8F9B60
.data:008F9B60 off_8F9B60      dd offset aWorldshader  ; DATA XREF: sub_6C5760+13↑o
.data:008F9B60                                         ; sub_6D5C10+39↑o
.data:008F9B60                                         ; "WorldShader"
.data:008F9B64                 db    2
.data:008F9B65                 db    0
.data:008F9B66                 db    0
.data:008F9B67                 db    0
.data:008F9B68                 db    0
.data:008F9B69                 db    0
.data:008F9B6A                 db    0
.data:008F9B6B                 db    0
.data:008F9B6C                 db    2
.data:008F9B6D                 db    0
.data:008F9B6E                 db    0
.data:008F9B6F                 db    0
.data:008F9B70                 db    1
.data:008F9B71                 db    0
.data:008F9B72                 db    0
.data:008F9B73                 db    0
.data:008F9B74                 db    4
.data:008F9B75                 db    0
.data:008F9B76                 db    0
.data:008F9B77                 db    0
.data:008F9B78                 db    2
.data:008F9B79                 db    0
.data:008F9B7A                 db    0
.data:008F9B7B                 db    0
.data:008F9B7C                 db    1
.data:008F9B7D                 db    0
.data:008F9B7E                 db    0
.data:008F9B7F                 db    0
.data:008F9B80                 db    3
.data:008F9B81                 db    0
.data:008F9B82                 db    0
.data:008F9B83                 db    0
.data:008F9B84                 db    8
.data:008F9B85                 db    0
.data:008F9B86                 db    0
.data:008F9B87                 db    0
.data:008F9B88                 db  0Ah
.data:008F9B89                 db    0
.data:008F9B8A                 db    0
.data:008F9B8B                 db    0
.data:008F9B8C                 db    0
.data:008F9B8D                 db    0
.data:008F9B8E                 db    0
.data:008F9B8F                 db    0
.data:008F9B90                 db    0
.data:008F9B91                 db    0
.data:008F9B92                 db    0
.data:008F9B93                 db    0
.data:008F9B94                 db    0
.data:008F9B95                 db    0
.data:008F9B96                 db    0
.data:008F9B97                 db    0
.data:008F9B98                 db    0
.data:008F9B99                 db    0
.data:008F9B9A                 db    0
.data:008F9B9B                 db    0
.data:008F9B9C                 db    0
.data:008F9B9D                 db    0
.data:008F9B9E                 db    0
.data:008F9B9F                 db    0
.data:008F9BA0                 db    0
.data:008F9BA1                 db    0
.data:008F9BA2                 db    0
.data:008F9BA3                 db    0
.data:008F9BA4                 db    0
.data:008F9BA5                 db    0
.data:008F9BA6                 db    0
.data:008F9BA7                 db    0
.data:008F9BA8                 db    0
.data:008F9BA9                 db    0
.data:008F9BAA                 db    0
.data:008F9BAB                 db    0
.data:008F9BAC                 db    0
.data:008F9BAD                 db    0
.data:008F9BAE                 db    0
.data:008F9BAF                 db    0
.data:008F9BB0                 db    0
.data:008F9BB1                 db    0
.data:008F9BB2                 db    0
.data:008F9BB3                 db    0
.data:008F9BB4                 db    0
.data:008F9BB5                 db    0
.data:008F9BB6                 db    0
.data:008F9BB7                 db    0
.data:008F9BB8                 db    0
.data:008F9BB9                 db    0
.data:008F9BBA                 db    0
.data:008F9BBB                 db    0
.data:008F9BBC                 db    0
.data:008F9BBD                 db    0
.data:008F9BBE                 db    0
.data:008F9BBF                 db    0
.data:008F9BC0                 db    0
.data:008F9BC1                 db    0
.data:008F9BC2                 db    0
.data:008F9BC3                 db    0
.data:008F9BC4                 db    0
.data:008F9BC5                 db    0
.data:008F9BC6                 db    0
.data:008F9BC7                 db    0
.data:008F9BC8                 db    0
.data:008F9BC9                 db    0
.data:008F9BCA                 db    0
.data:008F9BCB                 db    0
.data:008F9BCC                 db    0
.data:008F9BCD                 db    0
.data:008F9BCE                 db    0
.data:008F9BCF                 db    0
.data:008F9BD0                 db    0
.data:008F9BD1                 db    0
.data:008F9BD2                 db    0
.data:008F9BD3                 db    0
.data:008F9BD4                 db    0
.data:008F9BD5                 db    0
.data:008F9BD6                 db    0
.data:008F9BD7                 db    0
.data:008F9BD8                 db    0
.data:008F9BD9                 db    0
.data:008F9BDA                 db    0
.data:008F9BDB                 db    0
.data:008F9BDC                 db    0
.data:008F9BDD                 db    0
.data:008F9BDE                 db    0
.data:008F9BDF                 db    0
.data:008F9BE0                 db    0
.data:008F9BE1                 db    0
.data:008F9BE2                 db    0
.data:008F9BE3                 db    0
.data:008F9BE4                 db 0D4h ; Ô
.data:008F9BE5                 db 0E7h ; ç
.data:008F9BE6                 db  8Ah ; Š
.data:008F9BE7                 db    0
.data:008F9BE8 off_8F9BE8      dd offset aIdi_world_fx ; DATA XREF: sub_6C6080+48↑r
.data:008F9BE8                                         ; "IDI_WORLD_FX"
.data:008F9BEC                 align 10h
.data:008F9BF0                 dd offset aWorldreflectsh ; "WorldReflectShader"
.data:008F9BF4                 db    2
.data:008F9BF5                 db    0
.data:008F9BF6                 db    0
.data:008F9BF7                 db    0
.data:008F9BF8                 db    0
.data:008F9BF9                 db    0
.data:008F9BFA                 db    0
.data:008F9BFB                 db    0
.data:008F9BFC                 db    2
.data:008F9BFD                 db    0
.data:008F9BFE                 db    0
.data:008F9BFF                 db    0
.data:008F9C00                 db    1
.data:008F9C01                 db    0
.data:008F9C02                 db    0
.data:008F9C03                 db    0
.data:008F9C04                 db    4
.data:008F9C05                 db    0
.data:008F9C06                 db    0
.data:008F9C07                 db    0
.data:008F9C08                 db    2
.data:008F9C09                 db    0
.data:008F9C0A                 db    0
.data:008F9C0B                 db    0
.data:008F9C0C                 db    1
.data:008F9C0D                 db    0
.data:008F9C0E                 db    0
.data:008F9C0F                 db    0
.data:008F9C10                 db    3
.data:008F9C11                 db    0
.data:008F9C12                 db    0
.data:008F9C13                 db    0
.data:008F9C14                 db    1
.data:008F9C15                 db    0
.data:008F9C16                 db    0
.data:008F9C17                 db    0
.data:008F9C18                 db    4
.data:008F9C19                 db    0
.data:008F9C1A                 db    0
.data:008F9C1B                 db    0
.data:008F9C1C                 db    3
.data:008F9C1D                 db    0
.data:008F9C1E                 db    0
.data:008F9C1F                 db    0
.data:008F9C20                 db    9
.data:008F9C21                 db    0
.data:008F9C22                 db    0
.data:008F9C23                 db    0
.data:008F9C24                 db    8
.data:008F9C25                 db    0
.data:008F9C26                 db    0
.data:008F9C27                 db    0
.data:008F9C28                 db  0Ah