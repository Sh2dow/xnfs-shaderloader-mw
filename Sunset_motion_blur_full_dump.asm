
==========================================================================================
FUNCTION 0x10004430 ??0memory_pointer_tr@injector@@QAE@I@Z
==========================================================================================
0x10004430  mov     eax, large fs:2Ch
0x10004436  mov     edx, __tls_index
0x1000443C  push    esi
0x1000443D  mov     esi, this
0x1000443F  mov     edx, [eax+edx*4]
0x10004442  mov     eax, ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA
0x10004447  cmp     eax, [edx+4]
0x1000444D  jle     short loc_10004495
0x1000444F  push    offset ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA; pOnce
0x10004454  call    __Init_thread_header
0x10004459  add     esp, 4
0x1000445C  cmp     ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA, 0FFFFFFFFh
0x10004463  jnz     short loc_10004495
0x10004465  mov     ?m@?1??singleton@address_manager@injector@@SAAAV23@XZ@4V23@A, offset aUnknownPluginN; "Unknown Plugin Name"
0x1000446F  mov     dword_1011AB24, 0
0x10004479  mov     dword_1011AB20, 0
0x10004483  call    ?Detect@game_version_manager@injector@@QAE_NXZ; injector::game_version_manager::Detect(void)
0x10004488  push    offset ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA; pOnce
0x1000448D  call    __Init_thread_footer
0x10004492  add     esp, 4
0x10004495  mov     eax, [esp+4+x]
0x10004499  mov     [esi], eax
0x1000449B  mov     eax, esi
0x1000449D  pop     esi
0x1000449E  retn    4

==========================================================================================
FUNCTION 0x10004540 ?GetBranchDestination@injector@@YA?AT?$basic_memory_pointer@Ufn_mem_translator_nop@address_manager@injector@@@1@Tmemory_pointer_tr@1@_N@Z
==========================================================================================
0x10004540  sub     esp, 20h
0x10004543  mov     eax, ___security_cookie
0x10004548  xor     eax, esp
0x1000454A  mov     [esp+20h+var_4], eax
0x1000454E  push    ebx
0x1000454F  mov     ebx, ds:__imp__VirtualProtect@16; VirtualProtect(x,x,x,x)
0x10004555  lea     eax, [esp+24h+flOldProtect]
0x10004559  push    ebp
0x1000455A  push    esi
0x1000455B  mov     esi, dword ptr [esp+2Ch+at]
0x1000455F  push    edi
0x10004560  push    eax; lpflOldProtect
0x10004561  push    40h ; '@'; flNewProtect
0x10004563  push    1; dwSize
0x10004565  push    esi; lpAddress
0x10004566  mov     edi, ecx
0x10004568  mov     [esp+40h+dwSize], 1
0x10004570  mov     [esp+40h+lpAddress], esi
0x10004574  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10004576  test    eax, eax
0x10004578  setnz   [esp+30h+var_8]
0x1000457D  mov     cl, [esi]
0x1000457F  mov     [esp+30h+var_19], cl
0x10004583  test    eax, eax
0x10004585  jz      short loc_100045A3
0x10004587  mov     eax, [esp+30h+flOldProtect]
0x1000458B  lea     ecx, [esp+30h+var_18]
0x1000458F  push    ecx; lpflOldProtect
0x10004590  push    eax; flNewProtect
0x10004591  push    [esp+38h+dwSize]; dwSize
0x10004595  mov     [esp+3Ch+var_18], eax
0x10004599  push    [esp+3Ch+lpAddress]; lpAddress
0x1000459D  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x1000459F  mov     cl, [esp+30h+var_19]
0x100045A3  movzx   eax, cl
0x100045A6  sub     eax, 0E8h
0x100045AB  jz      loc_1000468F
0x100045B1  sub     eax, 1
0x100045B4  jz      loc_1000468F
0x100045BA  sub     eax, 16h
0x100045BD  jnz     loc_10004674
0x100045C3  mov     esi, dword ptr [esp+30h+at]
0x100045C7  lea     eax, [esp+30h+flOldProtect]
0x100045CB  push    eax; lpflOldProtect
0x100045CC  push    40h ; '@'; flNewProtect
0x100045CE  inc     esi
0x100045CF  mov     [esp+38h+dwSize], 1
0x100045D7  push    1; dwSize
0x100045D9  push    esi; lpAddress
0x100045DA  mov     [esp+40h+lpAddress], esi
0x100045DE  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x100045E0  test    eax, eax
0x100045E2  setnz   [esp+30h+var_8]
0x100045E7  mov     cl, [esi]
0x100045E9  mov     [esp+30h+var_19], cl
0x100045ED  test    eax, eax
0x100045EF  jz      short loc_1000460D
0x100045F1  mov     eax, [esp+30h+flOldProtect]
0x100045F5  lea     ecx, [esp+30h+var_18]
0x100045F9  push    ecx; lpflOldProtect
0x100045FA  push    eax; flNewProtect
0x100045FB  push    [esp+38h+dwSize]; dwSize
0x100045FF  mov     [esp+3Ch+var_18], eax
0x10004603  push    [esp+3Ch+lpAddress]; lpAddress
0x10004607  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10004609  mov     cl, [esp+30h+var_19]
0x1000460D  cmp     cl, 15h
0x10004610  jz      short loc_10004617
0x10004612  cmp     cl, 25h ; '%'
0x10004615  jnz     short loc_10004674
0x10004617  mov     esi, dword ptr [esp+30h+at]
0x1000461B  lea     eax, [esp+30h+flOldProtect]
0x1000461F  push    eax; lpflOldProtect
0x10004620  push    40h ; '@'; flNewProtect
0x10004622  add     esi, 2
0x10004625  mov     [esp+38h+dwSize], 4
0x1000462D  push    4; dwSize
0x1000462F  push    esi; lpAddress
0x10004630  mov     [esp+40h+lpAddress], esi
0x10004634  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10004636  test    eax, eax
0x10004638  setnz   [esp+30h+var_8]
0x1000463D  mov     esi, [esi]
0x1000463F  test    eax, eax
0x10004641  jz      short loc_1000465B
0x10004643  mov     eax, [esp+30h+flOldProtect]
0x10004647  lea     ecx, [esp+30h+var_18]
0x1000464B  push    ecx; lpflOldProtect
0x1000464C  push    eax; flNewProtect
0x1000464D  push    [esp+38h+dwSize]; dwSize
0x10004651  mov     [esp+3Ch+var_18], eax
0x10004655  push    [esp+3Ch+lpAddress]; lpAddress
0x10004659  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x1000465B  mov     ecx, [esi]
0x1000465D  mov     eax, edi
0x1000465F  mov     [edi], ecx
0x10004661  pop     edi
0x10004662  pop     esi
0x10004663  pop     ebp
0x10004664  pop     ebx
0x10004665  mov     ecx, [esp+20h+var_4]
0x10004669  xor     ecx, esp; StackCookie
0x1000466B  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004670  add     esp, 20h
0x10004673  retn
0x10004674  mov     dword ptr [edi], 0
0x1000467A  mov     eax, edi
0x1000467C  pop     edi
0x1000467D  pop     esi
0x1000467E  pop     ebp
0x1000467F  pop     ebx
0x10004680  mov     ecx, [esp+20h+var_4]
0x10004684  xor     ecx, esp; StackCookie
0x10004686  call    @__security_check_cookie@4; __security_check_cookie(x)
0x1000468B  add     esp, 20h
0x1000468E  retn
0x1000468F  mov     esi, dword ptr [esp+30h+at]
0x10004693  lea     eax, [esp+30h+flOldProtect]
0x10004697  push    eax; lpflOldProtect
0x10004698  push    40h ; '@'; flNewProtect
0x1000469A  inc     esi
0x1000469B  mov     [esp+38h+dwSize], 4
0x100046A3  push    4; dwSize
0x100046A5  push    esi; lpAddress
0x100046A6  mov     [esp+40h+lpAddress], esi
0x100046AA  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x100046AC  test    eax, eax
0x100046AE  setnz   [esp+30h+var_8]
0x100046B3  mov     ebp, [esi]
0x100046B5  test    eax, eax
0x100046B7  jz      short loc_100046D1
0x100046B9  mov     eax, [esp+30h+flOldProtect]
0x100046BD  lea     ecx, [esp+30h+var_18]
0x100046C1  push    ecx; lpflOldProtect
0x100046C2  push    eax; flNewProtect
0x100046C3  push    [esp+38h+dwSize]; dwSize
0x100046C7  mov     [esp+3Ch+var_18], eax
0x100046CB  push    [esp+3Ch+lpAddress]; lpAddress
0x100046CF  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x100046D1  mov     ecx, [esp+30h+var_4]
0x100046D5  lea     eax, [esi+4]
0x100046D8  add     eax, ebp
0x100046DA  mov     [edi], eax
0x100046DC  mov     eax, edi
0x100046DE  pop     edi
0x100046DF  pop     esi
0x100046E0  pop     ebp
0x100046E1  pop     ebx
0x100046E2  xor     ecx, esp; StackCookie
0x100046E4  call    @__security_check_cookie@4; __security_check_cookie(x)
0x100046E9  add     esp, 20h
0x100046EC  retn

==========================================================================================
FUNCTION 0x100046F0 ?MakeJMP@injector@@YA?AT?$basic_memory_pointer@Ufn_mem_translator_nop@address_manager@injector@@@1@Tmemory_pointer_tr@1@T21@_N@Z
==========================================================================================
0x100046F0  sub     esp, 1Ch
0x100046F3  mov     eax, ___security_cookie
0x100046F8  xor     eax, esp
0x100046FA  mov     [esp+1Ch+var_8], eax
0x100046FE  mov     eax, dword ptr [esp+1Ch+at]
0x10004702  push    ebx
0x10004703  push    ebp
0x10004704  push    esi; at
0x10004705  push    edi; at
0x10004706  push    p; result
0x10004707  mov     ebp, p
0x10004709  mov     p, esp
0x1000470B  mov     [p], eax
0x1000470D  mov     p, ebp
0x1000470F  call    ?GetBranchDestination@injector@@YA?AT?$basic_memory_pointer@Ufn_mem_translator_nop@address_manager@injector@@@1@Tmemory_pointer_tr@1@_N@Z; injector::GetBranchDestination(injector::memory_pointer_tr,bool)
0x10004714  mov     esi, dword ptr [esp+30h+at]
0x10004718  lea     eax, [esp+30h+flOldProtect]
0x1000471C  mov     ebx, ds:__imp__VirtualProtect@16; VirtualProtect(x,x,x,x)
0x10004722  add     esp, 4
0x10004725  mov     [esp+2Ch+dwSize], 1
0x1000472D  mov     [esp+2Ch+lpAddress], esi
0x10004731  push    eax; lpflOldProtect
0x10004732  push    40h ; '@'; flNewProtect
0x10004734  push    1; dwSize
0x10004736  push    esi; lpAddress
0x10004737  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10004739  test    eax, eax
0x1000473B  setnz   [esp+2Ch+var_C]
0x10004740  mov     byte ptr [esi], 0E9h
0x10004743  test    eax, eax
0x10004745  jz      short loc_1000475F
0x10004747  mov     eax, [esp+2Ch+flOldProtect]
0x1000474B  lea     p, [esp+2Ch+var_1C]
0x1000474F  push    p; lpflOldProtect
0x10004750  push    eax; flNewProtect
0x10004751  push    [esp+34h+dwSize]; dwSize
0x10004755  mov     [esp+38h+var_1C], eax
0x10004759  push    [esp+38h+lpAddress]; lpAddress
0x1000475D  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x1000475F  mov     esi, dword ptr [esp+2Ch+at]
0x10004763  mov     edi, dword ptr [esp+2Ch+dest]
0x10004767  inc     esi
0x10004768  mov     [esp+2Ch+dwSize], 4
0x10004770  mov     [esp+2Ch+lpAddress], esi
0x10004774  lea     eax, [esi+4]
0x10004777  sub     edi, eax
0x10004779  lea     eax, [esp+2Ch+flOldProtect]
0x1000477D  push    eax; lpflOldProtect
0x1000477E  push    40h ; '@'; flNewProtect
0x10004780  push    4; dwSize
0x10004782  push    esi; lpAddress
0x10004783  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10004785  test    eax, eax
0x10004787  setnz   [esp+2Ch+var_C]
0x1000478C  mov     [esi], edi
0x1000478E  test    eax, eax
0x10004790  jz      short loc_100047AA
0x10004792  mov     p, [esp+2Ch+flOldProtect]
0x10004796  lea     eax, [esp+2Ch+var_1C]
0x1000479A  push    eax; lpflOldProtect
0x1000479B  push    p; flNewProtect
0x1000479C  push    [esp+34h+dwSize]; dwSize
0x100047A0  mov     [esp+38h+var_1C], p
0x100047A4  push    [esp+38h+lpAddress]; lpAddress
0x100047A8  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x100047AA  mov     p, [esp+2Ch+var_8]
0x100047AE  mov     eax, ebp
0x100047B0  pop     edi
0x100047B1  pop     esi
0x100047B2  pop     ebp
0x100047B3  pop     ebx
0x100047B4  xor     p, esp; StackCookie
0x100047B6  call    @__security_check_cookie@4; __security_check_cookie(x)
0x100047BB  add     esp, 1Ch
0x100047BE  retn

==========================================================================================
FUNCTION 0x100047C0 ?MakeCALL@injector@@YA?AT?$basic_memory_pointer@Ufn_mem_translator_nop@address_manager@injector@@@1@Tmemory_pointer_tr@1@T21@_N@Z
==========================================================================================
0x100047C0  sub     esp, 1Ch
0x100047C3  mov     eax, ___security_cookie
0x100047C8  xor     eax, esp
0x100047CA  mov     [esp+1Ch+var_8], eax
0x100047CE  mov     eax, dword ptr [esp+1Ch+at]
0x100047D2  push    ebx
0x100047D3  push    ebp
0x100047D4  push    esi; at
0x100047D5  push    edi; at
0x100047D6  push    p; result
0x100047D7  mov     ebp, p
0x100047D9  mov     p, esp
0x100047DB  mov     [p], eax
0x100047DD  mov     p, ebp
0x100047DF  call    ?GetBranchDestination@injector@@YA?AT?$basic_memory_pointer@Ufn_mem_translator_nop@address_manager@injector@@@1@Tmemory_pointer_tr@1@_N@Z; injector::GetBranchDestination(injector::memory_pointer_tr,bool)
0x100047E4  mov     esi, dword ptr [esp+30h+at]
0x100047E8  lea     eax, [esp+30h+flOldProtect]
0x100047EC  mov     ebx, ds:__imp__VirtualProtect@16; VirtualProtect(x,x,x,x)
0x100047F2  add     esp, 4
0x100047F5  mov     [esp+2Ch+dwSize], 1
0x100047FD  mov     [esp+2Ch+lpAddress], esi
0x10004801  push    eax; lpflOldProtect
0x10004802  push    40h ; '@'; flNewProtect
0x10004804  push    1; dwSize
0x10004806  push    esi; lpAddress
0x10004807  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10004809  test    eax, eax
0x1000480B  setnz   [esp+2Ch+var_C]
0x10004810  mov     byte ptr [esi], 0E8h
0x10004813  test    eax, eax
0x10004815  jz      short loc_1000482F
0x10004817  mov     eax, [esp+2Ch+flOldProtect]
0x1000481B  lea     p, [esp+2Ch+var_1C]
0x1000481F  push    p; lpflOldProtect
0x10004820  push    eax; flNewProtect
0x10004821  push    [esp+34h+dwSize]; dwSize
0x10004825  mov     [esp+38h+var_1C], eax
0x10004829  push    [esp+38h+lpAddress]; lpAddress
0x1000482D  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x1000482F  mov     esi, dword ptr [esp+2Ch+at]
0x10004833  mov     edi, dword ptr [esp+2Ch+dest]
0x10004837  inc     esi
0x10004838  mov     [esp+2Ch+dwSize], 4
0x10004840  mov     [esp+2Ch+lpAddress], esi
0x10004844  lea     eax, [esi+4]
0x10004847  sub     edi, eax
0x10004849  lea     eax, [esp+2Ch+flOldProtect]
0x1000484D  push    eax; lpflOldProtect
0x1000484E  push    40h ; '@'; flNewProtect
0x10004850  push    4; dwSize
0x10004852  push    esi; lpAddress
0x10004853  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10004855  test    eax, eax
0x10004857  setnz   [esp+2Ch+var_C]
0x1000485C  mov     [esi], edi
0x1000485E  test    eax, eax
0x10004860  jz      short loc_1000487A
0x10004862  mov     p, [esp+2Ch+flOldProtect]
0x10004866  lea     eax, [esp+2Ch+var_1C]
0x1000486A  push    eax; lpflOldProtect
0x1000486B  push    p; flNewProtect
0x1000486C  push    [esp+34h+dwSize]; dwSize
0x10004870  mov     [esp+38h+var_1C], p
0x10004874  push    [esp+38h+lpAddress]; lpAddress
0x10004878  call    ebx ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x1000487A  mov     p, [esp+2Ch+var_8]
0x1000487E  mov     eax, ebp
0x10004880  pop     edi
0x10004881  pop     esi
0x10004882  pop     ebp
0x10004883  pop     ebx
0x10004884  xor     p, esp; StackCookie
0x10004886  call    @__security_check_cookie@4; __security_check_cookie(x)
0x1000488B  add     esp, 1Ch
0x1000488E  retn

==========================================================================================
FUNCTION 0x10004920 ?Detect@game_version_manager@injector@@QAE_NXZ
==========================================================================================
0x10004920  sub     esp, 18h
0x10004923  mov     eax, ___security_cookie
0x10004928  xor     eax, esp
0x1000492A  mov     [esp+18h+var_4], eax
0x1000492E  push    ebx
0x1000492F  push    esi
0x10004930  push    edi
0x10004931  push    0; lpModuleName
0x10004933  mov     dword_1011AB24, 0
0x1000493D  mov     dword_1011AB20, 0
0x10004947  call    ds:__imp__GetModuleHandleA@4; GetModuleHandleA(x)
0x1000494D  mov     ecx, [eax+3Ch]
0x10004950  mov     eax, [ecx+eax+28h]
0x10004954  add     eax, 400000h
0x10004959  cmp     eax, 8252FCh
0x1000495E  ja      loc_10004B7D
0x10004964  jz      loc_10004B58
0x1000496A  cmp     eax, 667C40h
0x1000496F  ja      loc_10004A44
0x10004975  jz      loc_10004A1F
0x1000497B  cmp     eax, 5C6FD0h
0x10004980  ja      short loc_100049E4
0x10004982  jz      loc_10004BCD
0x10004988  cmp     eax, 5C1E70h
0x1000498D  jz      short loc_100049BF
0x1000498F  cmp     eax, 5C2130h
0x10004994  jnz     $LN25; jumptable 10004A59 default case, cases 8537457-8537467,8537469-8537519,8537521-8537531
0x1000499A  mov     dword_1011AB20, 1010033h
0x100049A4  mov     al, 1
0x100049A6  mov     byte ptr dword_1011AB24+3, 0
0x100049AD  pop     edi
0x100049AE  pop     esi
0x100049AF  pop     ebx
0x100049B0  mov     ecx, [esp+18h+var_4]
0x100049B4  xor     ecx, esp; StackCookie
0x100049B6  call    @__security_check_cookie@4; __security_check_cookie(x)
0x100049BB  add     esp, 18h
0x100049BE  retn
0x100049BF  mov     dword_1011AB20, 10033h
0x100049C9  mov     al, 1
0x100049CB  mov     byte ptr dword_1011AB24+3, 0
0x100049D2  pop     edi
0x100049D3  pop     esi
0x100049D4  pop     ebx
0x100049D5  mov     ecx, [esp+18h+var_4]
0x100049D9  xor     ecx, esp; StackCookie
0x100049DB  call    @__security_check_cookie@4; __security_check_cookie(x)
0x100049E0  add     esp, 18h
0x100049E3  retn
0x100049E4  cmp     eax, 666BA0h
0x100049E9  jz      loc_10004C1C
0x100049EF  cmp     eax, 667BF0h
0x100049F4  jnz     $LN25; jumptable 10004A59 default case, cases 8537457-8537467,8537469-8537519,8537521-8537531
0x100049FA  mov     dword_1011AB20, 10056h
0x10004A04  mov     al, 1
0x10004A06  mov     byte ptr dword_1011AB24+3, 0
0x10004A0D  pop     edi
0x10004A0E  pop     esi
0x10004A0F  pop     ebx
0x10004A10  mov     ecx, [esp+18h+var_4]
0x10004A14  xor     ecx, esp; StackCookie
0x10004A16  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004A1B  add     esp, 18h
0x10004A1E  retn
0x10004A1F  mov     dword_1011AB20, 1010056h
0x10004A29  mov     al, 1
0x10004A2B  mov     byte ptr dword_1011AB24+3, 0
0x10004A32  pop     edi
0x10004A33  pop     esi
0x10004A34  pop     ebx
0x10004A35  mov     ecx, [esp+18h+var_4]
0x10004A39  xor     ecx, esp; StackCookie
0x10004A3B  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004A40  add     esp, 18h
0x10004A43  retn
0x10004A44  sub     eax, 824570h; switch 77 cases
0x10004A49  cmp     eax, 4Ch
0x10004A4C  ja      $LN25; jumptable 10004A59 default case, cases 8537457-8537467,8537469-8537519,8537521-8537531
0x10004A52  movzx   eax, ds:byte_10004D50[eax]
0x10004A59  jmp     ds:jpt_10004A59[eax*4]; switch jump
0x10004A60  mov     edi, ds:__imp__VirtualProtect@16; jumptable 10004A59 cases 8537456,8537468
0x10004A66  lea     eax, [esp+24h+flOldProtect]
0x10004A6A  push    eax; lpflOldProtect
0x10004A6B  push    40h ; '@'; flNewProtect
0x10004A6D  mov     esi, 406A20h
0x10004A72  mov     dword_1011AB20, 15553h
0x10004A7C  push    1; dwSize
0x10004A7E  push    esi; lpAddress
0x10004A7F  mov     byte ptr dword_1011AB24+3, 0
0x10004A86  mov     [esp+34h+dwSize], 1
0x10004A8E  mov     [esp+34h+lpAddress], esi
0x10004A92  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10004A94  test    eax, eax
0x10004A96  setnz   [esp+24h+var_8]
0x10004A9B  mov     bl, [esi]
0x10004A9D  test    eax, eax
0x10004A9F  jz      short loc_10004AB9
0x10004AA1  mov     eax, [esp+24h+flOldProtect]
0x10004AA5  lea     ecx, [esp+24h+var_18]
0x10004AA9  push    ecx; lpflOldProtect
0x10004AAA  push    eax; flNewProtect
0x10004AAB  push    [esp+2Ch+dwSize]; dwSize
0x10004AAF  mov     [esp+30h+var_18], eax
0x10004AB3  push    [esp+30h+lpAddress]; lpAddress
0x10004AB7  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10004AB9  cmp     bl, 0E9h
0x10004ABC  setnz   al
0x10004ABF  dec     al
0x10004AC1  and     al, 48h
0x10004AC3  mov     byte ptr dword_1011AB24+2, al
0x10004AC8  mov     al, 1
0x10004ACA  pop     edi
0x10004ACB  pop     esi
0x10004ACC  pop     ebx
0x10004ACD  mov     ecx, [esp+18h+var_4]
0x10004AD1  xor     ecx, esp; StackCookie
0x10004AD3  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004AD8  add     esp, 18h
0x10004ADB  retn
0x10004ADC  mov     edi, ds:__imp__VirtualProtect@16; jumptable 10004A59 cases 8537520,8537532
0x10004AE2  lea     eax, [esp+24h+flOldProtect]
0x10004AE6  push    eax; lpflOldProtect
0x10004AE7  push    40h ; '@'; flNewProtect
0x10004AE9  mov     esi, 406A20h
0x10004AEE  mov     dword_1011AB20, 14553h
0x10004AF8  push    1; dwSize
0x10004AFA  push    esi; lpAddress
0x10004AFB  mov     byte ptr dword_1011AB24+3, 0
0x10004B02  mov     [esp+34h+dwSize], 1
0x10004B0A  mov     [esp+34h+lpAddress], esi
0x10004B0E  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10004B10  test    eax, eax
0x10004B12  setnz   [esp+24h+var_8]
0x10004B17  mov     bl, [esi]
0x10004B19  test    eax, eax
0x10004B1B  jz      short loc_10004AB9
0x10004B1D  mov     eax, [esp+24h+flOldProtect]
0x10004B21  lea     ecx, [esp+24h+var_18]
0x10004B25  push    ecx; lpflOldProtect
0x10004B26  push    eax; flNewProtect
0x10004B27  push    [esp+2Ch+dwSize]; dwSize
0x10004B2B  mov     [esp+30h+var_18], eax
0x10004B2F  push    [esp+30h+lpAddress]; lpAddress
0x10004B33  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10004B35  cmp     bl, 0E9h
0x10004B38  setnz   al
0x10004B3B  dec     al
0x10004B3D  and     al, 48h
0x10004B3F  mov     byte ptr dword_1011AB24+2, al
0x10004B44  mov     al, 1
0x10004B46  pop     edi
0x10004B47  pop     esi
0x10004B48  pop     ebx
0x10004B49  mov     ecx, [esp+18h+var_4]
0x10004B4D  xor     ecx, esp; StackCookie
0x10004B4F  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004B54  add     esp, 18h
0x10004B57  retn
0x10004B58  mov     dword_1011AB20, 1015553h
0x10004B62  mov     al, 1
0x10004B64  mov     byte ptr dword_1011AB24+3, 0
0x10004B6B  pop     edi
0x10004B6C  pop     esi
0x10004B6D  pop     ebx
0x10004B6E  mov     ecx, [esp+18h+var_4]
0x10004B72  xor     ecx, esp; StackCookie
0x10004B74  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004B79  add     esp, 18h
0x10004B7C  retn
0x10004B7D  cmp     eax, 0CF4BADh
0x10004B82  ja      loc_10004C59
0x10004B88  jz      loc_10004C2B
0x10004B8E  cmp     eax, 9912EDh
0x10004B93  ja      short loc_10004BDC
0x10004B95  jz      short loc_10004BCD
0x10004B97  cmp     eax, 82533Ch
0x10004B9C  jz      short loc_10004BA8
0x10004B9E  cmp     eax, 85EC4Ah
0x10004BA3  jmp     loc_10004D07
0x10004BA8  mov     dword_1011AB20, 1014553h
0x10004BB2  mov     al, 1
0x10004BB4  mov     byte ptr dword_1011AB24+3, 0
0x10004BBB  pop     edi
0x10004BBC  pop     esi
0x10004BBD  pop     ebx
0x10004BBE  mov     ecx, [esp+18h+var_4]
0x10004BC2  xor     ecx, esp; StackCookie
0x10004BC4  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004BC9  add     esp, 18h
0x10004BCC  retn
0x10004BCD  mov     dword_1011AB20, 1010033h
0x10004BD7  jmp     loc_10004D27
0x10004BDC  cmp     eax, 0A402EDh
0x10004BE1  jz      short loc_10004C1C
0x10004BE3  cmp     eax, 0C965ADh
0x10004BE8  jnz     $LN25; jumptable 10004A59 default case, cases 8537457-8537467,8537469-8537519,8537521-8537531
0x10004BEE  mov     dword_1011AB20, 15549h
0x10004BF8  mov     al, 1
0x10004BFA  mov     word ptr dword_1011AB24, 400h
0x10004C03  mov     byte ptr dword_1011AB24+3, 0
0x10004C0A  pop     edi
0x10004C0B  pop     esi
0x10004C0C  pop     ebx
0x10004C0D  mov     ecx, [esp+18h+var_4]
0x10004C11  xor     ecx, esp; StackCookie
0x10004C13  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004C18  add     esp, 18h
0x10004C1B  retn
0x10004C1C  mov     dword_1011AB20, 1010056h
0x10004C26  jmp     loc_10004D27
0x10004C2B  mov     dword_1011AB20, 1015545h
0x10004C35  mov     al, 1
0x10004C37  mov     word ptr dword_1011AB24, 3
0x10004C40  mov     byte ptr dword_1011AB24+3, 0
0x10004C47  pop     edi
0x10004C48  pop     esi
0x10004C49  pop     ebx
0x10004C4A  mov     ecx, [esp+18h+var_4]
0x10004C4E  xor     ecx, esp; StackCookie
0x10004C50  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004C55  add     esp, 18h
0x10004C58  retn
0x10004C59  cmp     eax, 0D0D011h
0x10004C5E  ja      loc_10004D02
0x10004C64  jz      short loc_10004CD4
0x10004C66  cmp     eax, 0CF529Eh
0x10004C6B  jz      short loc_10004CA6
0x10004C6D  cmp     eax, 0D0AF06h
0x10004C72  jnz     $LN25; jumptable 10004A59 default case, cases 8537457-8537467,8537469-8537519,8537521-8537531
0x10004C78  mov     dword_1011AB20, 1015545h
0x10004C82  mov     al, 1
0x10004C84  mov     word ptr dword_1011AB24, 2
0x10004C8D  mov     byte ptr dword_1011AB24+3, 0
0x10004C94  pop     edi
0x10004C95  pop     esi
0x10004C96  pop     ebx
0x10004C97  mov     ecx, [esp+18h+var_4]
0x10004C9B  xor     ecx, esp; StackCookie
0x10004C9D  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004CA2  add     esp, 18h
0x10004CA5  retn
0x10004CA6  mov     dword_1011AB20, 15549h
0x10004CB0  mov     al, 1
0x10004CB2  mov     word ptr dword_1011AB24, 800h
0x10004CBB  mov     byte ptr dword_1011AB24+3, 0
0x10004CC2  pop     edi
0x10004CC3  pop     esi
0x10004CC4  pop     ebx
0x10004CC5  mov     ecx, [esp+18h+var_4]
0x10004CC9  xor     ecx, esp; StackCookie
0x10004CCB  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004CD0  add     esp, 18h
0x10004CD3  retn
0x10004CD4  mov     dword_1011AB20, 15549h
0x10004CDE  mov     al, 1
0x10004CE0  mov     word ptr dword_1011AB24, 700h
0x10004CE9  mov     byte ptr dword_1011AB24+3, 0
0x10004CF0  pop     edi
0x10004CF1  pop     esi
0x10004CF2  pop     ebx
0x10004CF3  mov     ecx, [esp+18h+var_4]
0x10004CF7  xor     ecx, esp; StackCookie
0x10004CF9  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004CFE  add     esp, 18h
0x10004D01  retn
0x10004D02  cmp     eax, 0D3C3DBh
0x10004D07  jz      short loc_10004D1D
0x10004D09  xor     al, al; jumptable 10004A59 default case, cases 8537457-8537467,8537469-8537519,8537521-8537531
0x10004D0B  pop     edi
0x10004D0C  pop     esi
0x10004D0D  pop     ebx
0x10004D0E  mov     ecx, [esp+18h+var_4]
0x10004D12  xor     ecx, esp; StackCookie
0x10004D14  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004D19  add     esp, 18h
0x10004D1C  retn
0x10004D1D  mov     dword_1011AB20, 30053h
0x10004D27  mov     ecx, [esp+24h+var_4]
0x10004D2B  mov     al, 1
0x10004D2D  pop     edi
0x10004D2E  pop     esi
0x10004D2F  pop     ebx
0x10004D30  xor     ecx, esp; StackCookie
0x10004D32  mov     byte ptr dword_1011AB24+3, 1
0x10004D39  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10004D3E  add     esp, 18h
0x10004D41  retn

==========================================================================================
FUNCTION 0x10010800 ?DrawFullScreenQuad@eEffect@@QAEXPAUIDirect3DTexture9@@_N@Z
==========================================================================================
0x10010800  sub     esp, 8Ch
0x10010806  mov     eax, ___security_cookie
0x1001080B  xor     eax, esp
0x1001080D  mov     [esp+8Ch+var_8], eax
0x10010814  mov     eax, ds:982BDCh
0x10010819  push    esi
0x1001081A  push    edi
0x1001081B  mov     edi, [esp+94h+texture]
0x10010822  mov     esi, this
0x10010824  mov     edx, [eax]
0x10010826  push    1
0x10010828  push    16h
0x1001082A  push    eax
0x1001082B  call    dword ptr [edx+0E4h]
0x10010831  mov     eax, [edi]
0x10010833  lea     this, [esp+94h+var_8C]
0x10010837  push    this
0x10010838  push    0
0x1001083A  push    edi
0x1001083B  call    dword ptr [eax+44h]
0x1001083E  mov     this, [esp+94h+var_74]
0x10010842  mov     eax, this
0x10010844  mov     edx, [esp+94h+var_70]
0x10010848  movss   xmm2, ds:__real@4f800000
0x10010850  shr     eax, 1Fh
0x10010853  movd    xmm0, this
0x10010857  cvtdq2pd xmm0, xmm0
0x1001085B  addsd   xmm0, ds:__xmm@41f00000000000000000000000000000[eax*8]
0x10010864  mov     eax, edx
0x10010866  shr     eax, 1Fh
0x10010869  cvtpd2ps xmm1, xmm0
0x1001086D  movd    xmm0, edx
0x10010871  cvtdq2pd xmm0, xmm0
0x10010875  addsd   xmm0, ds:__xmm@41f00000000000000000000000000000[eax*8]
0x1001087E  cvtpd2ps xmm0, xmm0
0x10010882  test    this, this
0x10010884  jns     short loc_1001088A
0x10010886  addss   xmm1, xmm2
0x1001088A  test    edx, edx
0x1001088C  jns     short loc_10010892
0x1001088E  addss   xmm0, xmm2
0x10010892  movss   xmm2, ds:__real@3f000000
0x1001089A  movaps  xmm4, xmm2
0x1001089D  mov     eax, [esi+44h]
0x100108A0  divss   xmm4, xmm1
0x100108A4  mov     [esp+94h+var_6C], 0BF800000h
0x100108AC  mov     [esp+94h+var_68], 3F800000h
0x100108B4  mov     [esp+94h+var_64], 0
0x100108BC  mov     [esp+94h+var_60], 0FFFFFFFFh
0x100108C4  mov     [esp+94h+var_54], 3F800000h
0x100108CC  mov     [esp+94h+var_50], 3F800000h
0x100108D4  mov     [esp+94h+var_4C], 0
0x100108DC  mov     [esp+94h+var_48], 0FFFFFFFFh
0x100108E4  mov     [esp+94h+var_3C], 3F800000h
0x100108EC  mov     [esp+94h+var_38], 0BF800000h
0x100108F4  mov     [esp+94h+var_34], 0
0x100108FC  mov     [esp+94h+var_30], 0FFFFFFFFh
0x10010904  mov     [esp+94h+var_24], 0BF800000h
0x1001090C  mov     [esp+94h+var_20], 0BF800000h
0x10010914  mov     [esp+94h+var_1C], 0
0x1001091C  mov     [esp+94h+var_18], 0FFFFFFFFh
0x10010924  divss   xmm2, xmm0
0x10010928  movaps  xmm1, xmm4
0x1001092B  movss   [esp+94h+var_5C], xmm4
0x10010931  addss   xmm1, ds:__real@3f800000
0x10010939  movaps  xmm3, xmm2
0x1001093C  movss   [esp+94h+var_58], xmm2
0x10010942  addss   xmm3, ds:__real@3f800000
0x1001094A  movss   [esp+94h+var_40], xmm2
0x10010950  movss   [esp+94h+var_14], xmm4
0x10010959  movss   [esp+94h+var_44], xmm1
0x1001095F  movss   [esp+94h+var_2C], xmm1
0x10010965  movss   [esp+94h+var_28], xmm3
0x1001096B  movss   [esp+94h+var_10], xmm3
0x10010974  mov     edx, [eax+22Ch]
0x1001097A  test    edx, edx
0x1001097C  jz      short loc_1001098C
0x1001097E  mov     eax, [esi+48h]
0x10010981  push    edi
0x10010982  push    edx
0x10010983  push    eax
0x10010984  mov     this, [eax]
0x10010986  call    dword ptr [this+0D0h]
0x1001098C  mov     eax, [esi+48h]
0x1001098F  push    eax
0x10010990  mov     this, [eax]
0x10010992  call    dword ptr [this+104h]
0x10010998  mov     eax, ds:982BDCh
0x1001099D  lea     edx, [esp+94h+var_6C]
0x100109A1  push    18h
0x100109A3  push    edx
0x100109A4  push    2
0x100109A6  mov     this, [eax]
0x100109A8  push    6
0x100109AA  push    eax
0x100109AB  call    dword ptr [this+14Ch]
0x100109B1  mov     this, [esp+94h+var_8]
0x100109B8  pop     edi
0x100109B9  pop     esi
0x100109BA  xor     this, esp; StackCookie
0x100109BC  call    @__security_check_cookie@4; __security_check_cookie(x)
0x100109C1  add     esp, 8Ch
0x100109C7  retn    8

==========================================================================================
FUNCTION 0x10014730 ?GetSunScreenUV@@YA?AUD3DXVECTOR4@@XZ
==========================================================================================
0x10014730  sub     esp, 88h
0x10014736  mov     eax, ___security_cookie
0x1001473B  xor     eax, esp
0x1001473D  mov     [esp+88h+var_8], eax
0x10014744  push    esi
0x10014745  mov     eax, 46EB40h
0x1001474A  mov     esi, ecx
0x1001474C  call    eax
0x1001474E  movss   xmm1, ds:__real@461c4000
0x10014756  mov     ecx, 919650h
0x1001475B  push    0
0x1001475D  movq    xmm0, qword ptr [eax+40h]
0x10014762  mov     eax, [eax+48h]
0x10014765  mov     dword ptr [esp+90h+var_78+8], eax
0x10014769  mov     eax, ds:9B392Ch
0x1001476E  movq    qword ptr [esp+90h+var_78], xmm0
0x10014774  movss   xmm0, dword ptr [eax+180h]
0x1001477C  movss   xmm3, dword ptr [eax+188h]
0x10014784  movss   xmm2, dword ptr [eax+184h]
0x1001478C  lea     eax, [esp+90h+var_78+0Ch]
0x10014790  mulss   xmm0, xmm1
0x10014794  push    eax
0x10014795  mulss   xmm2, xmm1
0x10014799  mov     eax, 6BE8F0h
0x1001479E  addss   xmm0, dword ptr [esp+94h+var_78]
0x100147A4  movss   [esp+94h+var_84], xmm3
0x100147AA  addss   xmm2, dword ptr [esp+94h+var_78+4]
0x100147B0  movss   dword ptr [esp+94h+var_78+0Ch], xmm0
0x100147B6  movaps  xmm0, xmm3
0x100147B9  mulss   xmm0, xmm1
0x100147BD  movss   [esp+94h+var_68], xmm2
0x100147C3  addss   xmm0, dword ptr [esp+94h+var_78+8]
0x100147C9  movss   [esp+94h+var_64], xmm0
0x100147CF  call    eax
0x100147D1  test    al, al
0x100147D3  jnz     short loc_10014808
0x100147D5  mov     dword ptr [esi], 0
0x100147DB  mov     eax, esi
0x100147DD  mov     dword ptr [esi+4], 0
0x100147E4  mov     dword ptr [esi+8], 0
0x100147EB  mov     dword ptr [esi+0Ch], 0
0x100147F2  pop     esi
0x100147F3  mov     ecx, [esp+88h+var_8]
0x100147FA  xor     ecx, esp; StackCookie
0x100147FC  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10014801  add     esp, 88h
0x10014807  retn
0x10014808  mov     eax, ds:919650h
0x1001480D  movups  xmm0, xmmword ptr [eax+100h]
0x10014814  movups  [esp+8Ch+var_54+8], xmm0
0x10014819  movups  xmm0, xmmword ptr [eax+110h]
0x10014820  movups  [esp+8Ch+var_44+8], xmm0
0x10014825  movups  xmm0, xmmword ptr [eax+120h]
0x1001482C  movups  [esp+8Ch+var_34+8], xmm0
0x10014831  movups  xmm0, xmmword ptr [eax+130h]
0x10014838  lea     eax, [esp+8Ch+var_54+8]
0x1001483C  push    eax
0x1001483D  lea     eax, [esp+90h+var_78+0Ch]
0x10014841  push    eax
0x10014842  lea     eax, [esp+94h+var_5C]
0x10014846  push    eax
0x10014847  movups  [esp+98h+var_24+8], xmm0
0x1001484C  call    ds:__imp__D3DXVec3Transform@12; D3DXVec3Transform(x,x,x)
0x10014852  movss   xmm0, [esp+8Ch+var_84]
0x10014858  lea     eax, [esp+8Ch+var_84]
0x1001485C  subss   xmm0, ds:__real@be4ccccd
0x10014864  movss   xmm2, [esp+8Ch+var_58]
0x1001486A  lea     ecx, [esp+8Ch+var_80]
0x1001486E  movss   xmm3, [esp+8Ch+var_5C]
0x10014874  xorps   xmm7, xmm7
0x10014877  divss   xmm3, dword ptr [esp+8Ch+var_54+4]
0x1001487D  mov     [esp+8Ch+var_7C], 3F800000h
0x10014885  mov     [esp+8Ch+var_80], 0
0x1001488D  mov     [esp+8Ch+var_60], 0
0x10014895  divss   xmm0, ds:__real@3e4ccccd
0x1001489D  divss   xmm2, dword ptr [esp+8Ch+var_54+4]
0x100148A3  movss   xmm5, ds:__real@3f800000
0x100148AB  movss   xmm1, ds:__real@3f000000
0x100148B3  movaps  xmm4, xmm5
0x100148B6  comiss  xmm7, xmm0
0x100148B9  mulss   xmm3, xmm1
0x100148BD  cmovbe  ecx, eax
0x100148C0  mulss   xmm2, xmm1
0x100148C4  lea     eax, [esp+8Ch+var_7C]
0x100148C8  comiss  xmm0, xmm5
0x100148CB  movss   [esp+8Ch+var_84], xmm0
0x100148D1  addss   xmm3, xmm1
0x100148D5  addss   xmm2, xmm1
0x100148D9  cmovbe  eax, ecx
0x100148DC  mov     ecx, 1
0x100148E1  movss   dword ptr [esp+8Ch+var_78+0Ch], xmm3
0x100148E7  movss   xmm1, dword ptr [eax]
0x100148EB  subss   xmm4, xmm2
0x100148EF  movss   xmm2, ds:__real@40400000
0x100148F7  movaps  xmm0, xmm1
0x100148FA  addss   xmm0, xmm1
0x100148FE  movaps  xmm6, xmm2
0x10014901  mulss   xmm1, xmm1
0x10014905  movss   [esp+8Ch+var_68], xmm4
0x1001490B  subss   xmm6, xmm0
0x1001490F  movss   xmm0, ?g_Weather@@3VWeather@@A.NightFactor; Weather g_Weather ...
0x10014917  ucomiss xmm0, xmm5
0x1001491A  movaps  xmm0, xmm5
0x1001491D  mulss   xmm6, xmm1
0x10014921  subss   xmm0, xmm3
0x10014925  lahf
0x10014926  xor     edx, edx
0x10014928  test    ah, 44h
0x1001492B  cmovnp  edx, ecx
0x1001492E  minss   xmm3, xmm0
0x10014932  movaps  xmm0, xmm5
0x10014935  subss   xmm0, xmm4
0x10014939  minss   xmm4, xmm0
0x1001493D  comiss  xmm4, xmm3
0x10014940  ja      short loc_10014945
0x10014942  movaps  xmm3, xmm4
0x10014945  divss   xmm3, ds:__real@3e99999a
0x1001494D  lea     eax, [esp+8Ch+var_80]
0x10014951  mov     [esp+8Ch+var_84], 3F800000h
0x10014959  lea     ecx, [esp+8Ch+var_7C]
0x1001495D  mov     [esp+8Ch+var_7C], 0
0x10014965  comiss  xmm7, xmm3
0x10014968  movss   [esp+8Ch+var_80], xmm3
0x1001496E  cmovbe  ecx, eax
0x10014971  lea     eax, [esp+8Ch+var_84]
0x10014975  comiss  xmm3, xmm5
0x10014978  cmovbe  eax, ecx
0x1001497B  mov     ecx, [esp+8Ch+var_8]
0x10014982  movss   xmm1, dword ptr [eax]
0x10014986  mov     eax, esi
0x10014988  movaps  xmm0, xmm1
0x1001498B  addss   xmm0, xmm1
0x1001498F  mulss   xmm1, xmm1
0x10014993  subss   xmm2, xmm0
0x10014997  movd    xmm0, edx
0x1001499B  cvtdq2ps xmm0, xmm0
0x1001499E  mulss   xmm2, xmm1
0x100149A2  mulss   xmm0, xmm6
0x100149A6  mulss   xmm2, xmm0
0x100149AA  movss   [esp+8Ch+var_64], xmm2
0x100149B0  movups  xmm0, [esp+8Ch+var_78+0Ch]
0x100149B5  movups  xmmword ptr [esi], xmm0
0x100149B8  pop     esi
0x100149B9  xor     ecx, esp; StackCookie
0x100149BB  call    @__security_check_cookie@4; __security_check_cookie(x)
0x100149C0  add     esp, 88h
0x100149C6  retn

==========================================================================================
FUNCTION 0x100149D0 ?DrawGodRays@@YAXXZ
==========================================================================================
0x100149D0  push    ebp
0x100149D1  mov     ebp, esp
0x100149D3  and     esp, 0FFFFFFF8h
0x100149D6  sub     esp, 24h
0x100149D9  mov     eax, ___security_cookie
0x100149DE  xor     eax, esp
0x100149E0  mov     [esp+24h+var_4], eax
0x100149E4  cmp     dword ptr ds:925E90h, 6
0x100149EB  push    ebx
0x100149EC  push    esi
0x100149ED  push    edi; result
0x100149EE  jnz     loc_10014B71
0x100149F4  cmp     ?g_Config@@3UConfig@@A.GodRays, 0; Config g_Config ...
0x100149FB  jz      loc_10014B71
0x10014A01  lea     ecx, [esp+30h+sunPos]
0x10014A05  call    ?GetSunScreenUV@@YA?AUD3DXVECTOR4@@XZ; GetSunScreenUV(void)
0x10014A0A  movss   xmm0, [esp+30h+sunPos.z]
0x10014A10  ucomiss xmm0, ds:__real@00000000
0x10014A17  lahf
0x10014A18  test    ah, 44h
0x10014A1B  jnp     loc_10014B71
0x10014A21  mov     eax, ds:982BDCh
0x10014A26  lea     edx, [esp+30h+rtBackup]
0x10014A2A  push    edx
0x10014A2B  push    0
0x10014A2D  push    eax
0x10014A2E  mov     ecx, [eax]
0x10014A30  call    dword ptr [ecx+98h]
0x10014A36  mov     eax, ds:982BDCh
0x10014A3B  lea     edx, [esp+30h+ppZStencilSurface]
0x10014A3F  push    edx
0x10014A40  push    eax
0x10014A41  mov     ecx, [eax]
0x10014A43  call    dword ptr [ecx+0A0h]
0x10014A49  mov     eax, ds:982BDCh
0x10014A4E  push    dword ptr ds:93DE60h
0x10014A54  push    0
0x10014A56  mov     ecx, [eax]
0x10014A58  push    eax
0x10014A59  call    dword ptr [ecx+94h]
0x10014A5F  mov     eax, ds:982BDCh
0x10014A64  push    0
0x10014A66  push    eax
0x10014A67  mov     ecx, [eax]
0x10014A69  call    dword ptr [ecx+9Ch]
0x10014A6F  mov     eax, ds:982BDCh
0x10014A74  push    0
0x10014A76  push    0
0x10014A78  push    dword ptr ds:93DE64h
0x10014A7E  mov     ecx, [eax]
0x10014A80  push    0
0x10014A82  push    dword ptr ds:982A28h
0x10014A88  push    eax
0x10014A89  call    dword ptr [ecx+88h]
0x10014A8F  mov     esi, ds:93DEACh
0x10014A95  mov     edi, [esi+48h]
0x10014A98  mov     [esp+30h+passes], 0
0x10014AA0  mov     eax, ds:982BDCh
0x10014AA5  push    dword ptr [esi+4Ch]
0x10014AA8  push    eax
0x10014AA9  mov     ecx, [eax]
0x10014AAB  call    dword ptr [ecx+15Ch]
0x10014AB1  mov     eax, [edi]
0x10014AB3  lea     ecx, [esp+30h+passes]
0x10014AB7  push    0
0x10014AB9  push    ecx
0x10014ABA  push    edi
0x10014ABB  call    dword ptr [eax+0FCh]
0x10014AC1  mov     eax, [edi]
0x10014AC3  push    1
0x10014AC5  push    edi
0x10014AC6  call    dword ptr [eax+100h]
0x10014ACC  mov     eax, [esi+44h]
0x10014ACF  mov     edx, [eax+8BCh]
0x10014AD5  test    edx, edx
0x10014AD7  jz      short loc_10014AEB
0x10014AD9  mov     eax, [esi+48h]
0x10014ADC  lea     ebx, [esp+30h+sunPos]
0x10014AE0  push    ebx
0x10014AE1  push    edx
0x10014AE2  push    eax
0x10014AE3  mov     ecx, [eax]
0x10014AE5  call    dword ptr [ecx+88h]
0x10014AEB  mov     eax, [esi+44h]
0x10014AEE  mov     ecx, [eax+844h]
0x10014AF4  test    ecx, ecx
0x10014AF6  jz      short loc_10014B0B
0x10014AF8  mov     eax, [esi+48h]
0x10014AFB  push    ?DepthTexture@@3PAUIDirect3DTexture9@@A; IDirect3DTexture9 * DepthTexture
0x10014B01  push    ecx
0x10014B02  mov     edx, [eax]
0x10014B04  push    eax
0x10014B05  call    dword ptr [edx+0D0h]
0x10014B0B  push    ecx; texture
0x10014B0C  push    dword ptr ds:93DEFCh; texture
0x10014B12  mov     ecx, esi; this
0x10014B14  call    ?DrawFullScreenQuad@eEffect@@QAEXPAUIDirect3DTexture9@@_N@Z; eEffect::DrawFullScreenQuad(IDirect3DTexture9 *,bool)
0x10014B19  mov     eax, [edi]
0x10014B1B  push    edi
0x10014B1C  call    dword ptr [eax+108h]
0x10014B22  mov     eax, [edi]
0x10014B24  push    edi
0x10014B25  call    dword ptr [eax+10Ch]
0x10014B2B  mov     eax, ds:982BDCh
0x10014B30  push    0
0x10014B32  push    0
0x10014B34  push    dword ptr ds:982A28h
0x10014B3A  mov     ecx, [eax]
0x10014B3C  push    0
0x10014B3E  push    dword ptr ds:93DE60h
0x10014B44  push    eax
0x10014B45  call    dword ptr [ecx+88h]
0x10014B4B  mov     eax, ds:982BDCh
0x10014B50  push    [esp+30h+rtBackup]
0x10014B54  push    0
0x10014B56  mov     ecx, [eax]
0x10014B58  push    eax
0x10014B59  call    dword ptr [ecx+94h]
0x10014B5F  mov     eax, ds:982BDCh
0x10014B64  push    [esp+30h+ppZStencilSurface]
0x10014B68  push    eax
0x10014B69  mov     ecx, [eax]
0x10014B6B  call    dword ptr [ecx+9Ch]
0x10014B71  mov     ecx, [esp+30h+var_4]
0x10014B75  pop     edi
0x10014B76  pop     esi
0x10014B77  pop     ebx
0x10014B78  xor     ecx, esp; StackCookie
0x10014B7A  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10014B7F  mov     esp, ebp
0x10014B81  pop     ebp
0x10014B82  retn

==========================================================================================
FUNCTION 0x10014B90 ?RenderCars@@YAXPAUeView@@H@Z
==========================================================================================
0x10014B90  cmp     dword ptr ds:9017DCh, 0
0x10014B97  jnz     short locret_10014BA0
0x10014B99  mov     eax, 750B10h
0x10014B9E  jmp     eax
0x10014BA0  retn

==========================================================================================
FUNCTION 0x10014BB0 ?DrawBlur@@YAXXZ
==========================================================================================
0x10014BB0  cmp     dword ptr ds:9017DCh, 0
0x10014BB7  jz      short loc_10014BD8
0x10014BB9  mov     eax, 6DBB20h
0x10014BBE  call    eax
0x10014BC0  push    0
0x10014BC2  push    919650h
0x10014BC7  mov     eax, 750B10h
0x10014BCC  call    eax
0x10014BCE  add     esp, 8
0x10014BD1  mov     eax, 6E2F50h
0x10014BD6  call    eax
0x10014BD8  jmp     ?DrawGodRays@@YAXXZ; DrawGodRays(void)

==========================================================================================
FUNCTION 0x10014BE0 ?CopyBufferForBlur@@YGXPAUIDirect3DDevice9@@PAUIDirect3DSurface9@@PAUtagRECT@@12W4_D3DTEXTUREFILTERTYPE@@@Z
==========================================================================================
0x10014BE0  sub     esp, 8
0x10014BE3  mov     eax, ___security_cookie
0x10014BE8  xor     eax, esp
0x10014BEA  mov     [esp+8+var_4], eax
0x10014BEE  cmp     dword ptr ds:925E90h, 3
0x10014BF5  mov     ecx, [esp+8+device]
0x10014BF9  mov     edx, [esp+8+backBuffer]
0x10014BFD  push    ebx
0x10014BFE  mov     ebx, [esp+0Ch+pDestRect]
0x10014C02  push    esi
0x10014C03  mov     esi, [esp+10h+filterSurface0]
0x10014C07  push    edi
0x10014C08  mov     edi, [esp+14h+pSourceRect]
0x10014C0C  jnz     short loc_10014C33
0x10014C0E  push    [esp+14h+Filter]
0x10014C12  mov     eax, [ecx]
0x10014C14  push    ebx
0x10014C15  push    esi
0x10014C16  push    edi
0x10014C17  push    edx
0x10014C18  push    ecx
0x10014C19  call    dword ptr [eax+88h]
0x10014C1F  pop     edi
0x10014C20  pop     esi
0x10014C21  pop     ebx
0x10014C22  mov     ecx, [esp+8+var_4]
0x10014C26  xor     ecx, esp; StackCookie
0x10014C28  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10014C2D  add     esp, 8
0x10014C30  retn    18h
0x10014C33  mov     eax, ds:982BDCh
0x10014C38  push    0
0x10014C3A  push    0
0x10014C3C  push    dword ptr ds:93DE64h
0x10014C42  mov     ecx, [eax]
0x10014C44  push    0
0x10014C46  push    edx
0x10014C47  push    eax
0x10014C48  call    dword ptr [ecx+88h]
0x10014C4E  mov     eax, ds:982BDCh
0x10014C53  push    esi
0x10014C54  push    0
0x10014C56  push    eax
0x10014C57  mov     ecx, [eax]
0x10014C59  call    dword ptr [ecx+94h]
0x10014C5F  mov     eax, ds:982BDCh
0x10014C64  push    0
0x10014C66  push    eax
0x10014C67  mov     ecx, [eax]
0x10014C69  call    dword ptr [ecx+9Ch]
0x10014C6F  mov     edi, ds:93DEACh
0x10014C75  mov     esi, [edi+48h]
0x10014C78  mov     [esp+14h+var_8], 0
0x10014C80  mov     eax, ds:982BDCh
0x10014C85  push    dword ptr [edi+4Ch]
0x10014C88  push    eax
0x10014C89  mov     ecx, [eax]
0x10014C8B  call    dword ptr [ecx+15Ch]
0x10014C91  mov     eax, [esi]
0x10014C93  lea     ecx, [esp+14h+var_8]
0x10014C97  push    0
0x10014C99  push    ecx
0x10014C9A  push    esi
0x10014C9B  call    dword ptr [eax+0FCh]
0x10014CA1  mov     eax, [esi]
0x10014CA3  push    0
0x10014CA5  push    esi
0x10014CA6  call    dword ptr [eax+100h]
0x10014CAC  mov     eax, [edi+44h]
0x10014CAF  mov     edx, [eax+844h]
0x10014CB5  test    edx, edx
0x10014CB7  jz      short loc_10014CCC
0x10014CB9  mov     eax, [edi+48h]
0x10014CBC  push    ?DepthTexture@@3PAUIDirect3DTexture9@@A; IDirect3DTexture9 * DepthTexture
0x10014CC2  push    edx
0x10014CC3  mov     ecx, [eax]
0x10014CC5  push    eax
0x10014CC6  call    dword ptr [ecx+0D0h]
0x10014CCC  mov     eax, [edi+44h]
0x10014CCF  mov     ecx, [eax+4D4h]
0x10014CD5  test    ecx, ecx
0x10014CD7  jz      short loc_10014CF1
0x10014CD9  mov     eax, [edi+48h]
0x10014CDC  movss   xmm0, ?g_Config@@3UConfig@@A.BlurDepth; Config g_Config ...
0x10014CE4  push    ecx
0x10014CE5  movss   [esp+18h+var_18], xmm0
0x10014CEA  mov     edx, [eax]
0x10014CEC  push    ecx
0x10014CED  push    eax
0x10014CEE  call    dword ptr [edx+78h]
0x10014CF1  push    ecx; texture
0x10014CF2  push    dword ptr ds:93DEFCh; texture
0x10014CF8  mov     ecx, edi; this
0x10014CFA  call    ?DrawFullScreenQuad@eEffect@@QAEXPAUIDirect3DTexture9@@_N@Z; eEffect::DrawFullScreenQuad(IDirect3DTexture9 *,bool)
0x10014CFF  mov     eax, [esi]
0x10014D01  push    esi
0x10014D02  call    dword ptr [eax+108h]
0x10014D08  mov     eax, [esi]
0x10014D0A  push    esi
0x10014D0B  call    dword ptr [eax+10Ch]
0x10014D11  mov     ecx, [esp+14h+var_4]
0x10014D15  pop     edi
0x10014D16  pop     esi
0x10014D17  pop     ebx
0x10014D18  xor     ecx, esp; StackCookie
0x10014D1A  call    @__security_check_cookie@4; __security_check_cookie(x)
0x10014D1F  add     esp, 8
0x10014D22  retn    18h

==========================================================================================
FUNCTION 0x10014D30 ?AddToggleOptionHook@@YIXPAUUIWidgetMenu@@HPAUFEToggleWidget@@_N@Z
==========================================================================================
0x10014D30  push    ebx
0x10014D31  push    ebp
0x10014D32  push    esi
0x10014D33  push    [esp+0Ch+a4]
0x10014D37  mov     ebp, 588570h
0x10014D3C  mov     ebx, widgetMenu
0x10014D3E  push    [esp+10h+toggleWidget]
0x10014D42  call    ebp
0x10014D44  push    5Ch ; '\'
0x10014D46  mov     eax, 652AD0h
0x10014D4B  call    eax
0x10014D4D  add     esp, 4
0x10014D50  mov     esi, eax
0x10014D52  mov     eax, 589300h
0x10014D57  mov     widgetMenu, esi
0x10014D59  push    1
0x10014D5B  call    eax
0x10014D5D  push    [esp+0Ch+a4]
0x10014D61  mov     widgetMenu, ebx
0x10014D63  mov     dword ptr [esi], offset ?BlurToggleWidgetVT@@3PAIA; uint * BlurToggleWidgetVT
0x10014D69  push    esi
0x10014D6A  call    ebp
0x10014D6C  pop     esi
0x10014D6D  pop     ebp
0x10014D6E  pop     ebx
0x10014D6F  retn    8

==========================================================================================
FUNCTION 0x10014D80 ?DisableBlurResetHook@@YAXXZ
==========================================================================================
0x10014D80  mov     eax, 91CAE4h
0x10014D85  mov     eax, [eax]
0x10014D87  test    eax, eax
0x10014D89  jz      short updateEnabled
0x10014D8B  jmp     ds:cExit2
0x10014D91  fld     dword ptr ds:89096Ch
0x10014D98  jmp     ds:cExit1

==========================================================================================
FUNCTION 0x10014DA0 ?InitMotionBlur@@YAXXZ
==========================================================================================
0x10014DA0  sub     esp, 20h
0x10014DA3  mov     eax, ___security_cookie
0x10014DA8  xor     eax, esp
0x10014DAA  mov     [esp+20h+var_8], eax
0x10014DAE  push    ebx
0x10014DAF  push    esi; at
0x10014DB0  push    edi; dest
0x10014DB1  push    ecx; at
0x10014DB2  mov     eax, esp
0x10014DB4  push    ecx; result
0x10014DB5  mov     ecx, esp; this
0x10014DB7  push    6DEE3Fh; x
0x10014DBC  mov     dword ptr [eax], offset ?RenderCars@@YAXPAUeView@@H@Z; RenderCars(eView *,int)
0x10014DC2  call    ??0memory_pointer_tr@injector@@QAE@I@Z; injector::memory_pointer_tr::memory_pointer_tr(uint)
0x10014DC7  lea     ecx, [esp+34h+var_1C]
0x10014DCB  call    ?MakeCALL@injector@@YA?AT?$basic_memory_pointer@Ufn_mem_translator_nop@address_manager@injector@@@1@Tmemory_pointer_tr@1@T21@_N@Z; injector::MakeCALL(injector::memory_pointer_tr,injector::basic_memory_pointer<injector::address_manager::fn_mem_translator_nop>,bool)
0x10014DD0  add     esp, 4
0x10014DD3  mov     eax, esp
0x10014DD5  push    ecx; result
0x10014DD6  mov     ecx, esp; this
0x10014DD8  mov     dword ptr [eax], offset ?DrawBlur@@YAXXZ; DrawBlur(void)
0x10014DDE  push    6DF1DCh; x
0x10014DE3  call    ??0memory_pointer_tr@injector@@QAE@I@Z; injector::memory_pointer_tr::memory_pointer_tr(uint)
0x10014DE8  lea     ecx, [esp+34h+var_1C]
0x10014DEC  call    ?MakeCALL@injector@@YA?AT?$basic_memory_pointer@Ufn_mem_translator_nop@address_manager@injector@@@1@Tmemory_pointer_tr@1@T21@_N@Z; injector::MakeCALL(injector::memory_pointer_tr,injector::basic_memory_pointer<injector::address_manager::fn_mem_translator_nop>,bool)
0x10014DF1  movss   xmm0, ?g_Config@@3UConfig@@A.BlurMinSpeed; Config g_Config ...
0x10014DF9  lea     ecx, [esp+34h+var_20]; this
0x10014DFD  add     esp, 8
0x10014E00  movss   dword ptr [esp+2Ch+var_1C], xmm0
0x10014E06  push    8F9B10h; x
0x10014E0B  call    ??0memory_pointer_tr@injector@@QAE@I@Z; injector::memory_pointer_tr::memory_pointer_tr(uint)
0x10014E10  mov     esi, dword ptr [esp+2Ch+var_20]
0x10014E14  lea     eax, [esp+2Ch+flOldProtect]
0x10014E18  mov     edi, ds:__imp__VirtualProtect@16; VirtualProtect(x,x,x,x)
0x10014E1E  push    eax; lpflOldProtect
0x10014E1F  push    40h ; '@'; flNewProtect
0x10014E21  push    4; dwSize
0x10014E23  push    esi; lpAddress
0x10014E24  mov     [esp+3Ch+dwSize], 4
0x10014E2C  mov     [esp+3Ch+lpAddress], esi
0x10014E30  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10014E32  movss   xmm0, dword ptr [esp+2Ch+var_1C]
0x10014E38  test    eax, eax
0x10014E3A  setnz   [esp+2Ch+var_C]
0x10014E3F  movss   dword ptr [esi], xmm0
0x10014E43  cmp     [esp+2Ch+var_C], 0
0x10014E48  jz      short loc_10014E62
0x10014E4A  mov     eax, [esp+2Ch+flOldProtect]
0x10014E4E  lea     ecx, [esp+2Ch+var_1C]
0x10014E52  push    ecx; lpflOldProtect
0x10014E53  push    eax; flNewProtect
0x10014E54  push    [esp+34h+dwSize]; dwSize
0x10014E58  mov     dword ptr [esp+38h+var_1C], eax
0x10014E5C  push    [esp+38h+lpAddress]; lpAddress
0x10014E60  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10014E62  movss   xmm0, ?g_Config@@3UConfig@@A.BlurMaxSpeed; Config g_Config ...
0x10014E6A  lea     ecx, [esp+2Ch+var_1C]; this
0x10014E6E  push    8F9B14h; x
0x10014E73  movss   dword ptr [esp+30h+var_20], xmm0
0x10014E79  call    ??0memory_pointer_tr@injector@@QAE@I@Z; injector::memory_pointer_tr::memory_pointer_tr(uint)
0x10014E7E  mov     esi, dword ptr [esp+2Ch+var_1C]
0x10014E82  lea     eax, [esp+2Ch+flOldProtect]
0x10014E86  push    eax; lpflOldProtect
0x10014E87  push    40h ; '@'; flNewProtect
0x10014E89  push    4; dwSize
0x10014E8B  push    esi; lpAddress
0x10014E8C  mov     [esp+3Ch+dwSize], 4
0x10014E94  mov     [esp+3Ch+lpAddress], esi
0x10014E98  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10014E9A  movss   xmm0, dword ptr [esp+2Ch+var_20]
0x10014EA0  test    eax, eax
0x10014EA2  setnz   [esp+2Ch+var_C]
0x10014EA7  movss   dword ptr [esi], xmm0
0x10014EAB  cmp     [esp+2Ch+var_C], 0
0x10014EB0  jz      short loc_10014ECA
0x10014EB2  mov     eax, [esp+2Ch+flOldProtect]
0x10014EB6  lea     ecx, [esp+2Ch+var_1C]
0x10014EBA  push    ecx; lpflOldProtect
0x10014EBB  push    eax; flNewProtect
0x10014EBC  push    [esp+34h+dwSize]; dwSize
0x10014EC0  mov     dword ptr [esp+38h+var_1C], eax
0x10014EC4  push    [esp+38h+lpAddress]; lpAddress
0x10014EC8  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10014ECA  push    6DF1D2h; x
0x10014ECF  lea     ecx, [esp+30h+var_1C]; this
0x10014ED3  call    ??0memory_pointer_tr@injector@@QAE@I@Z; injector::memory_pointer_tr::memory_pointer_tr(uint)
0x10014ED8  mov     esi, dword ptr [esp+2Ch+var_1C]
0x10014EDC  lea     eax, [esp+2Ch+flOldProtect]
0x10014EE0  push    eax; lpflOldProtect
0x10014EE1  push    40h ; '@'; flNewProtect
0x10014EE3  push    2; dwSize
0x10014EE5  push    esi; lpAddress
0x10014EE6  mov     [esp+3Ch+dwSize], 2
0x10014EEE  mov     [esp+3Ch+lpAddress], esi
0x10014EF2  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10014EF4  test    eax, eax
0x10014EF6  mov     ebx, 90909090h
0x10014EFB  setnz   [esp+2Ch+var_C]
0x10014F00  mov     [esi], bx
0x10014F03  cmp     [esp+2Ch+var_C], 0
0x10014F08  jz      short loc_10014F22
0x10014F0A  mov     eax, [esp+2Ch+flOldProtect]
0x10014F0E  lea     ecx, [esp+2Ch+var_1C]
0x10014F12  push    ecx; lpflOldProtect
0x10014F13  push    eax; flNewProtect
0x10014F14  push    [esp+34h+dwSize]; dwSize
0x10014F18  mov     dword ptr [esp+38h+var_1C], eax
0x10014F1C  push    [esp+38h+lpAddress]; lpAddress
0x10014F20  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10014F22  push    6DBE28h; x
0x10014F27  lea     ecx, [esp+30h+var_1C]; this
0x10014F2B  call    ??0memory_pointer_tr@injector@@QAE@I@Z; injector::memory_pointer_tr::memory_pointer_tr(uint)
0x10014F30  mov     esi, dword ptr [esp+2Ch+var_1C]
0x10014F34  lea     eax, [esp+2Ch+flOldProtect]
0x10014F38  push    eax; lpflOldProtect
0x10014F39  push    40h ; '@'; flNewProtect
0x10014F3B  push    2; dwSize
0x10014F3D  push    esi; lpAddress
0x10014F3E  mov     [esp+3Ch+dwSize], 2
0x10014F46  mov     [esp+3Ch+lpAddress], esi
0x10014F4A  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10014F4C  test    eax, eax
0x10014F4E  setnz   [esp+2Ch+var_C]
0x10014F53  mov     [esi], bx
0x10014F56  cmp     [esp+2Ch+var_C], 0
0x10014F5B  jz      short loc_10014F75
0x10014F5D  mov     eax, [esp+2Ch+flOldProtect]
0x10014F61  lea     ecx, [esp+2Ch+var_1C]
0x10014F65  push    ecx; lpflOldProtect
0x10014F66  push    eax; flNewProtect
0x10014F67  push    [esp+34h+dwSize]; dwSize
0x10014F6B  mov     dword ptr [esp+38h+var_1C], eax
0x10014F6F  push    [esp+38h+lpAddress]; lpAddress
0x10014F73  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10014F75  push    6DF449h; x
0x10014F7A  lea     ecx, [esp+30h+var_1C]; this
0x10014F7E  call    ??0memory_pointer_tr@injector@@QAE@I@Z; injector::memory_pointer_tr::memory_pointer_tr(uint)
0x10014F83  mov     esi, dword ptr [esp+2Ch+var_1C]
0x10014F87  lea     eax, [esp+2Ch+flOldProtect]
0x10014F8B  push    eax; lpflOldProtect
0x10014F8C  push    40h ; '@'; flNewProtect
0x10014F8E  push    5; dwSize
0x10014F90  push    esi; lpAddress
0x10014F91  mov     [esp+3Ch+dwSize], 5
0x10014F99  mov     [esp+3Ch+lpAddress], esi
0x10014F9D  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10014F9F  test    eax, eax
0x10014FA1  setnz   [esp+2Ch+var_C]
0x10014FA6  mov     [esi], ebx
0x10014FA8  mov     byte ptr [esi+4], 90h
0x10014FAC  cmp     [esp+2Ch+var_C], 0
0x10014FB1  jz      short loc_10014FCB
0x10014FB3  mov     eax, [esp+2Ch+flOldProtect]
0x10014FB7  lea     ecx, [esp+2Ch+var_1C]
0x10014FBB  push    ecx; lpflOldProtect
0x10014FBC  push    eax; flNewProtect
0x10014FBD  push    [esp+34h+dwSize]; dwSize
0x10014FC1  mov     dword ptr [esp+38h+var_1C], eax
0x10014FC5  push    [esp+38h+lpAddress]; lpAddress
0x10014FC9  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10014FCB  cmp     ?g_Config@@3UConfig@@A.DepthPrepass, 0; Config g_Config ...
0x10014FD2  jz      loc_100150F8
0x10014FD8  push    6DBD7Ch; x
0x10014FDD  lea     ecx, [esp+30h+var_1C]; this
0x10014FE1  call    ??0memory_pointer_tr@injector@@QAE@I@Z; injector::memory_pointer_tr::memory_pointer_tr(uint)
0x10014FE6  mov     esi, dword ptr [esp+2Ch+var_1C]
0x10014FEA  lea     eax, [esp+2Ch+flOldProtect]
0x10014FEE  push    eax; lpflOldProtect
0x10014FEF  push    40h ; '@'; flNewProtect
0x10014FF1  push    6; dwSize
0x10014FF3  push    esi; lpAddress
0x10014FF4  mov     [esp+3Ch+dwSize], 6
0x10014FFC  mov     [esp+3Ch+lpAddress], esi
0x10015000  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10015002  test    eax, eax
0x10015004  mov     eax, ebx
0x10015006  setnz   [esp+2Ch+var_C]
0x1001500B  mov     [esi], eax
0x1001500D  mov     [esi+4], ax
0x10015011  cmp     [esp+2Ch+var_C], 0
0x10015016  jz      short loc_10015030
0x10015018  mov     eax, [esp+2Ch+flOldProtect]
0x1001501C  lea     ecx, [esp+2Ch+var_1C]
0x10015020  push    ecx; lpflOldProtect
0x10015021  push    eax; flNewProtect
0x10015022  push    [esp+34h+dwSize]; dwSize
0x10015026  mov     dword ptr [esp+38h+var_1C], eax
0x1001502A  push    [esp+38h+lpAddress]; lpAddress
0x1001502E  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10015030  push    ecx; at
0x10015031  mov     eax, esp
0x10015033  push    ecx; result
0x10015034  mov     ecx, esp; this
0x10015036  push    6DBD7Ch; x
0x1001503B  mov     dword ptr [eax], offset ?CopyBufferForBlur@@YGXPAUIDirect3DDevice9@@PAUIDirect3DSurface9@@PAUtagRECT@@12W4_D3DTEXTUREFILTERTYPE@@@Z; CopyBufferForBlur(IDirect3DDevice9 *,IDirect3DSurface9 *,tagRECT *,IDirect3DSurface9 *,tagRECT *,_D3DTEXTUREFILTERTYPE)
0x10015041  call    ??0memory_pointer_tr@injector@@QAE@I@Z; injector::memory_pointer_tr::memory_pointer_tr(uint)
0x10015046  lea     ecx, [esp+34h+var_1C]
0x1001504A  call    ?MakeCALL@injector@@YA?AT?$basic_memory_pointer@Ufn_mem_translator_nop@address_manager@injector@@@1@Tmemory_pointer_tr@1@T21@_N@Z; injector::MakeCALL(injector::memory_pointer_tr,injector::basic_memory_pointer<injector::address_manager::fn_mem_translator_nop>,bool)
0x1001504F  add     esp, 8
0x10015052  lea     ecx, [esp+2Ch+var_1C]; this
0x10015056  push    6DBE9Eh; x
0x1001505B  call    ??0memory_pointer_tr@injector@@QAE@I@Z; injector::memory_pointer_tr::memory_pointer_tr(uint)
0x10015060  mov     esi, dword ptr [esp+2Ch+var_1C]
0x10015064  lea     eax, [esp+2Ch+flOldProtect]
0x10015068  push    eax; lpflOldProtect
0x10015069  push    40h ; '@'; flNewProtect
0x1001506B  push    1; dwSize
0x1001506D  push    esi; lpAddress
0x1001506E  mov     [esp+3Ch+dwSize], 1
0x10015076  mov     [esp+3Ch+lpAddress], esi
0x1001507A  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x1001507C  test    eax, eax
0x1001507E  setnz   [esp+2Ch+var_C]
0x10015083  mov     byte ptr [esi], 6
0x10015086  cmp     [esp+2Ch+var_C], 0
0x1001508B  jz      short loc_100150A5
0x1001508D  mov     eax, [esp+2Ch+flOldProtect]
0x10015091  lea     ecx, [esp+2Ch+var_1C]
0x10015095  push    ecx; lpflOldProtect
0x10015096  push    eax; flNewProtect
0x10015097  push    [esp+34h+dwSize]; dwSize
0x1001509B  mov     dword ptr [esp+38h+var_1C], eax
0x1001509F  push    [esp+38h+lpAddress]; lpAddress
0x100150A3  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x100150A5  push    6DBEA0h; x
0x100150AA  lea     ecx, [esp+30h+var_1C]; this
0x100150AE  call    ??0memory_pointer_tr@injector@@QAE@I@Z; injector::memory_pointer_tr::memory_pointer_tr(uint)
0x100150B3  mov     esi, dword ptr [esp+2Ch+var_1C]
0x100150B7  lea     eax, [esp+2Ch+flOldProtect]
0x100150BB  push    eax; lpflOldProtect
0x100150BC  push    40h ; '@'; flNewProtect
0x100150BE  push    1; dwSize
0x100150C0  push    esi; lpAddress
0x100150C1  mov     [esp+3Ch+dwSize], 1
0x100150C9  mov     [esp+3Ch+lpAddress], esi
0x100150CD  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x100150CF  test    eax, eax
0x100150D1  setnz   [esp+2Ch+var_C]
0x100150D6  mov     byte ptr [esi], 5
0x100150D9  cmp     [esp+2Ch+var_C], 0
0x100150DE  jz      short loc_100150F8
0x100150E0  mov     eax, [esp+2Ch+flOldProtect]
0x100150E4  lea     ecx, [esp+2Ch+var_1C]
0x100150E8  push    ecx; lpflOldProtect
0x100150E9  push    eax; flNewProtect
0x100150EA  push    [esp+34h+dwSize]; dwSize
0x100150EE  mov     dword ptr [esp+38h+var_1C], eax
0x100150F2  push    [esp+38h+lpAddress]; lpAddress
0x100150F6  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x100150F8  push    ecx; at
0x100150F9  mov     eax, esp
0x100150FB  push    ecx; result
0x100150FC  mov     ecx, esp; this
0x100150FE  push    529B29h; x
0x10015103  mov     dword ptr [eax], offset ?AddToggleOptionHook@@YIXPAUUIWidgetMenu@@HPAUFEToggleWidget@@_N@Z; AddToggleOptionHook(UIWidgetMenu *,int,FEToggleWidget *,bool)
0x10015109  call    ??0memory_pointer_tr@injector@@QAE@I@Z; injector::memory_pointer_tr::memory_pointer_tr(uint)
0x1001510E  lea     ecx, [esp+34h+var_1C]
0x10015112  call    ?MakeCALL@injector@@YA?AT?$basic_memory_pointer@Ufn_mem_translator_nop@address_manager@injector@@@1@Tmemory_pointer_tr@1@T21@_N@Z; injector::MakeCALL(injector::memory_pointer_tr,injector::basic_memory_pointer<injector::address_manager::fn_mem_translator_nop>,bool)
0x10015117  mov     ecx, offset ?BlurToggleWidgetVT@@3PAIA; uint * BlurToggleWidgetVT
0x1001511C  mov     edx, 89BDB0h
0x10015121  add     esp, 8
0x10015124  sub     edx, ecx
0x10015126  mov     eax, [ecx+edx]
0x10015129  mov     [ecx], eax
0x1001512B  add     ecx, 4
0x1001512E  cmp     ecx, offset ?DepthPrePass@@3_NA; bool DepthPrePass
0x10015134  jl      short loc_10015126
0x10015136  mov     eax, large fs:2Ch
0x1001513C  mov     ecx, __tls_index
0x10015142  mov     ?BlurToggleWidgetVT@@3PAIA+4, 50FC00h; uint * BlurToggleWidgetVT
0x1001514C  mov     ?BlurToggleWidgetVT@@3PAIA+0Ch, 51BB80h; uint * BlurToggleWidgetVT
0x10015156  mov     ebx, [eax+ecx*4]
0x10015159  mov     eax, ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA
0x1001515E  cmp     eax, [ebx+4]
0x10015164  jle     short loc_100151AC
0x10015166  push    offset ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA; pOnce
0x1001516B  call    __Init_thread_header
0x10015170  add     esp, 4
0x10015173  cmp     ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA, 0FFFFFFFFh
0x1001517A  jnz     short loc_100151AC
0x1001517C  mov     ?m@?1??singleton@address_manager@injector@@SAAAV23@XZ@4V23@A, offset aUnknownPluginN; "Unknown Plugin Name"
0x10015186  mov     dword_1011AB24, 0
0x10015190  mov     dword_1011AB20, 0
0x1001519A  call    ?Detect@game_version_manager@injector@@QAE_NXZ; injector::game_version_manager::Detect(void)
0x1001519F  push    offset ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA; pOnce
0x100151A4  call    __Init_thread_footer
0x100151A9  add     esp, 4
0x100151AC  lea     eax, [esp+2Ch+flOldProtect]
0x100151B0  mov     [esp+2Ch+dwSize], 1
0x100151B8  push    eax; lpflOldProtect
0x100151B9  push    40h ; '@'; flNewProtect
0x100151BB  mov     esi, 50FC20h
0x100151C0  push    1; dwSize
0x100151C2  push    esi; lpAddress
0x100151C3  mov     [esp+3Ch+lpAddress], esi
0x100151C7  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x100151C9  test    eax, eax
0x100151CB  setnz   [esp+2Ch+var_C]
0x100151D0  mov     byte ptr [esi], 2Ch ; ','
0x100151D3  cmp     [esp+2Ch+var_C], 0
0x100151D8  jz      short loc_100151F2
0x100151DA  mov     eax, [esp+2Ch+flOldProtect]
0x100151DE  lea     ecx, [esp+2Ch+var_1C]
0x100151E2  push    ecx; lpflOldProtect
0x100151E3  push    eax; flNewProtect
0x100151E4  push    [esp+34h+dwSize]; dwSize
0x100151E8  mov     dword ptr [esp+38h+var_1C], eax
0x100151EC  push    [esp+38h+lpAddress]; lpAddress
0x100151F0  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x100151F2  mov     eax, ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA
0x100151F7  cmp     eax, [ebx+4]
0x100151FD  jle     short loc_10015245
0x100151FF  push    offset ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA; pOnce
0x10015204  call    __Init_thread_header
0x10015209  add     esp, 4
0x1001520C  cmp     ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA, 0FFFFFFFFh
0x10015213  jnz     short loc_10015245
0x10015215  mov     ?m@?1??singleton@address_manager@injector@@SAAAV23@XZ@4V23@A, offset aUnknownPluginN; "Unknown Plugin Name"
0x1001521F  mov     dword_1011AB24, 0
0x10015229  mov     dword_1011AB20, 0
0x10015233  call    ?Detect@game_version_manager@injector@@QAE_NXZ; injector::game_version_manager::Detect(void)
0x10015238  push    offset ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA; pOnce
0x1001523D  call    __Init_thread_footer
0x10015242  add     esp, 4
0x10015245  lea     eax, [esp+2Ch+flOldProtect]
0x10015249  mov     [esp+2Ch+dwSize], 1
0x10015251  push    eax; lpflOldProtect
0x10015252  push    40h ; '@'; flNewProtect
0x10015254  mov     esi, 50FC2Bh
0x10015259  push    1; dwSize
0x1001525B  push    esi; lpAddress
0x1001525C  mov     [esp+3Ch+lpAddress], esi
0x10015260  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10015262  test    eax, eax
0x10015264  setnz   [esp+2Ch+var_C]
0x10015269  mov     byte ptr [esi], 2Ch ; ','
0x1001526C  cmp     [esp+2Ch+var_C], 0
0x10015271  jz      short loc_1001528B
0x10015273  mov     eax, [esp+2Ch+flOldProtect]
0x10015277  lea     ecx, [esp+2Ch+var_1C]
0x1001527B  push    ecx; lpflOldProtect
0x1001527C  push    eax; flNewProtect
0x1001527D  push    [esp+34h+dwSize]; dwSize
0x10015281  mov     dword ptr [esp+38h+var_1C], eax
0x10015285  push    [esp+38h+lpAddress]; lpAddress
0x10015289  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x1001528B  mov     eax, ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA
0x10015290  cmp     eax, [ebx+4]
0x10015296  jle     short loc_100152DE
0x10015298  push    offset ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA; pOnce
0x1001529D  call    __Init_thread_header
0x100152A2  add     esp, 4
0x100152A5  cmp     ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA, 0FFFFFFFFh
0x100152AC  jnz     short loc_100152DE
0x100152AE  mov     ?m@?1??singleton@address_manager@injector@@SAAAV23@XZ@4V23@A, offset aUnknownPluginN; "Unknown Plugin Name"
0x100152B8  mov     dword_1011AB24, 0
0x100152C2  mov     dword_1011AB20, 0
0x100152CC  call    ?Detect@game_version_manager@injector@@QAE_NXZ; injector::game_version_manager::Detect(void)
0x100152D1  push    offset ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA; pOnce
0x100152D6  call    __Init_thread_footer
0x100152DB  add     esp, 4
0x100152DE  lea     eax, [esp+2Ch+flOldProtect]
0x100152E2  mov     [esp+2Ch+dwSize], 1
0x100152EA  push    eax; lpflOldProtect
0x100152EB  push    40h ; '@'; flNewProtect
0x100152ED  mov     esi, 51BBC1h
0x100152F2  push    1; dwSize
0x100152F4  push    esi; lpAddress
0x100152F5  mov     [esp+3Ch+lpAddress], esi
0x100152F9  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x100152FB  test    eax, eax
0x100152FD  setnz   [esp+2Ch+var_C]
0x10015302  mov     byte ptr [esi], 2Ch ; ','
0x10015305  cmp     [esp+2Ch+var_C], 0
0x1001530A  jz      short loc_10015324
0x1001530C  mov     eax, [esp+2Ch+flOldProtect]
0x10015310  lea     ecx, [esp+2Ch+var_1C]
0x10015314  push    ecx; lpflOldProtect
0x10015315  push    eax; flNewProtect
0x10015316  push    [esp+34h+dwSize]; dwSize
0x1001531A  mov     dword ptr [esp+38h+var_1C], eax
0x1001531E  push    [esp+38h+lpAddress]; lpAddress
0x10015322  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10015324  mov     eax, ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA
0x10015329  cmp     eax, [ebx+4]
0x1001532F  jle     short loc_10015377
0x10015331  push    offset ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA; pOnce
0x10015336  call    __Init_thread_header
0x1001533B  add     esp, 4
0x1001533E  cmp     ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA, 0FFFFFFFFh
0x10015345  jnz     short loc_10015377
0x10015347  mov     ?m@?1??singleton@address_manager@injector@@SAAAV23@XZ@4V23@A, offset aUnknownPluginN; "Unknown Plugin Name"
0x10015351  mov     dword_1011AB24, 0
0x1001535B  mov     dword_1011AB20, 0
0x10015365  call    ?Detect@game_version_manager@injector@@QAE_NXZ; injector::game_version_manager::Detect(void)
0x1001536A  push    offset ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA; pOnce
0x1001536F  call    __Init_thread_footer
0x10015374  add     esp, 4
0x10015377  lea     eax, [esp+2Ch+flOldProtect]
0x1001537B  mov     [esp+2Ch+dwSize], 4
0x10015383  push    eax; lpflOldProtect
0x10015384  push    40h ; '@'; flNewProtect
0x10015386  mov     esi, 51BB95h
0x1001538B  push    4; dwSize
0x1001538D  push    esi; lpAddress
0x1001538E  mov     [esp+3Ch+lpAddress], esi
0x10015392  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x10015394  test    eax, eax
0x10015396  setnz   [esp+2Ch+var_C]
0x1001539B  mov     dword ptr [esi], 1C7D9D8Dh
0x100153A1  cmp     [esp+2Ch+var_C], 0
0x100153A6  jz      short loc_100153C0
0x100153A8  mov     eax, [esp+2Ch+flOldProtect]
0x100153AC  lea     ecx, [esp+2Ch+var_1C]
0x100153B0  push    ecx; lpflOldProtect
0x100153B1  push    eax; flNewProtect
0x100153B2  push    [esp+34h+dwSize]; dwSize
0x100153B6  mov     dword ptr [esp+38h+var_1C], eax
0x100153BA  push    [esp+38h+lpAddress]; lpAddress
0x100153BE  call    edi ; VirtualProtect(x,x,x,x); VirtualProtect(x,x,x,x)
0x100153C0  push    ecx; at
0x100153C1  mov     eax, esp
0x100153C3  push    ecx; result
0x100153C4  mov     esi, esp
0x100153C6  mov     dword ptr [eax], offset ?DisableBlurResetHook@@YAXXZ; DisableBlurResetHook(void)
0x100153CC  mov     eax, ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA
0x100153D1  cmp     eax, [ebx+4]
0x100153D7  jle     short loc_1001541F
0x100153D9  push    offset ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA; pOnce
0x100153DE  call    __Init_thread_header
0x100153E3  add     esp, 4
0x100153E6  cmp     ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA, 0FFFFFFFFh
0x100153ED  jnz     short loc_1001541F
0x100153EF  mov     ?m@?1??singleton@address_manager@injector@@SAAAV23@XZ@4V23@A, offset aUnknownPluginN; "Unknown Plugin Name"
0x100153F9  mov     dword_1011AB24, 0
0x10015403  mov     dword_1011AB20, 0
0x1001540D  call    ?Detect@game_version_manager@injector@@QAE_NXZ; injector::game_version_manager::Detect(void)
0x10015412  push    offset ?$TSS0@?1??singleton@address_manager@injector@@SAAAV23@XZ@4HA; pOnce
0x10015417  call    __Init_thread_footer
0x1001541C  add     esp, 4
0x1001541F  lea     ecx, [esp+34h+var_1C]
0x10015423  mov     dword ptr [esi], 470282h
0x10015429  call    ?MakeJMP@injector@@YA?AT?$basic_memory_pointer@Ufn_mem_translator_nop@address_manager@injector@@@1@Tmemory_pointer_tr@1@T21@_N@Z; injector::MakeJMP(injector::memory_pointer_tr,injector::basic_memory_pointer<injector::address_manager::fn_mem_translator_nop>,bool)
0x1001542E  mov     ecx, [esp+34h+var_8]
0x10015432  add     esp, 8
0x10015435  pop     edi
0x10015436  pop     esi
0x10015437  pop     ebx
0x10015438  xor     ecx, esp; StackCookie
0x1001543A  call    @__security_check_cookie@4; __security_check_cookie(x)
0x1001543F  add     esp, 20h
0x10015442  retn

==========================================================================================
FUNCTION 0x1009CD5D @__security_check_cookie@4
==========================================================================================
0x1009CD5D  cmp     cookie, ___security_cookie
0x1009CD63  jnz     short failure
0x1009CD65  retn
0x1009CD66  jmp     ___report_gsfailure

==========================================================================================
FUNCTION 0x1009CD9E __Init_thread_footer
==========================================================================================
0x1009CD9E  push    ebp
0x1009CD9F  mov     ebp, esp
0x1009CDA1  push    esi
0x1009CDA2  mov     esi, offset g_tss_srw
0x1009CDA7  push    esi; SRWLock
0x1009CDA8  call    ds:__imp__AcquireSRWLockExclusive@4; AcquireSRWLockExclusive(x)
0x1009CDAE  mov     ecx, __Init_global_epoch
0x1009CDB4  mov     eax, [ebp+pOnce]
0x1009CDB7  inc     ecx
0x1009CDB8  mov     __Init_global_epoch, ecx
0x1009CDBE  push    esi; SRWLock
0x1009CDBF  mov     [eax], ecx
0x1009CDC1  mov     eax, large fs:2Ch
0x1009CDC7  mov     ecx, __tls_index
0x1009CDCD  mov     ecx, [eax+ecx*4]
0x1009CDD0  mov     eax, __Init_global_epoch
0x1009CDD5  mov     [ecx+4], eax
0x1009CDDB  call    ds:__imp__ReleaseSRWLockExclusive@4; ReleaseSRWLockExclusive(x)
0x1009CDE1  push    offset g_tss_cv; ConditionVariable
0x1009CDE6  call    ds:__imp__WakeAllConditionVariable@4; WakeAllConditionVariable(x)
0x1009CDEC  pop     esi
0x1009CDED  pop     ebp
0x1009CDEE  retn

==========================================================================================
FUNCTION 0x1009CDEF __Init_thread_header
==========================================================================================
0x1009CDEF  push    ebp
0x1009CDF0  mov     ebp, esp
0x1009CDF2  push    esi
0x1009CDF3  push    edi
0x1009CDF4  mov     edi, offset g_tss_srw
0x1009CDF9  push    edi; SRWLock
0x1009CDFA  call    ds:__imp__AcquireSRWLockExclusive@4; AcquireSRWLockExclusive(x)
0x1009CE00  mov     esi, [ebp+pOnce]
0x1009CE03  cmp     dword ptr [esi], 0
0x1009CE06  jnz     short loc_1009CE17
0x1009CE08  mov     dword ptr [esi], 0FFFFFFFFh
0x1009CE0E  jmp     short loc_1009CE36
0x1009CE10  call    __Init_thread_wait_v2
0x1009CE15  jmp     short loc_1009CE03
0x1009CE17  cmp     dword ptr [esi], 0FFFFFFFFh
0x1009CE1A  jz      short loc_1009CE10
0x1009CE1C  mov     eax, large fs:2Ch
0x1009CE22  mov     ecx, __tls_index
0x1009CE28  mov     ecx, [eax+ecx*4]
0x1009CE2B  mov     eax, __Init_global_epoch
0x1009CE30  mov     [ecx+4], eax
0x1009CE36  push    edi; SRWLock
0x1009CE37  call    ds:__imp__ReleaseSRWLockExclusive@4; ReleaseSRWLockExclusive(x)
0x1009CE3D  pop     edi
0x1009CE3E  pop     esi
0x1009CE3F  pop     ebp
0x1009CE40  retn

==========================================================================================
FUNCTION 0x1009CE41 __Init_thread_wait_v2
==========================================================================================
0x1009CE41  push    0; Flags
0x1009CE43  push    0FFFFFFFFh; dwMilliseconds
0x1009CE45  push    offset g_tss_srw; SRWLock
0x1009CE4A  push    offset g_tss_cv; ConditionVariable
0x1009CE4F  call    ds:__imp__SleepConditionVariableSRW@16; SleepConditionVariableSRW(x,x,x,x)
0x1009CE55  retn

==========================================================================================
FUNCTION 0x1009CE56 ___raise_securityfailure
==========================================================================================
0x1009CE56  push    ebp
0x1009CE57  mov     ebp, esp
0x1009CE59  push    0; lpTopLevelExceptionFilter
0x1009CE5B  call    ds:__imp__SetUnhandledExceptionFilter@4; SetUnhandledExceptionFilter(x)
0x1009CE61  push    [ebp+exception_pointers]; ExceptionInfo
0x1009CE64  call    ds:__imp__UnhandledExceptionFilter@4; UnhandledExceptionFilter(x)
0x1009CE6A  push    0C0000409h; uExitCode
0x1009CE6F  call    ds:__imp__GetCurrentProcess@0; GetCurrentProcess()
0x1009CE75  push    eax; hProcess
0x1009CE76  call    ds:__imp__TerminateProcess@8; TerminateProcess(x,x)
0x1009CE7C  pop     ebp
0x1009CE7D  retn

==========================================================================================
FUNCTION 0x1009CE7E ___report_gsfailure
==========================================================================================
0x1009CE7E  push    ebp
0x1009CE7F  mov     ebp, esp
0x1009CE81  sub     esp, 324h
0x1009CE87  push    17h; ProcessorFeature
0x1009CE89  call    ds:__imp__IsProcessorFeaturePresent@4; IsProcessorFeaturePresent(x)
0x1009CE8F  test    eax, eax
0x1009CE91  jz      short loc_1009CE98
0x1009CE93  push    2
0x1009CE95  pop     ecx
0x1009CE96  int     29h; Win8: RtlFailFast(ecx)
0x1009CE98  mov     GS_ContextRecord.Eax, eax
0x1009CE9D  mov     GS_ContextRecord.Ecx, ecx
0x1009CEA3  mov     GS_ContextRecord.Edx, edx
0x1009CEA9  mov     GS_ContextRecord.Ebx, ebx
0x1009CEAF  mov     GS_ContextRecord.Esi, esi
0x1009CEB5  mov     GS_ContextRecord.Edi, edi
0x1009CEBB  mov     word ptr GS_ContextRecord.SegSs, ss
0x1009CEC2  mov     word ptr GS_ContextRecord.SegCs, cs
0x1009CEC9  mov     word ptr GS_ContextRecord.SegDs, ds
0x1009CED0  mov     word ptr GS_ContextRecord.SegEs, es
0x1009CED7  mov     word ptr GS_ContextRecord.SegFs, fs
0x1009CEDE  mov     word ptr GS_ContextRecord.SegGs, gs
0x1009CEE5  pushf
0x1009CEE6  pop     GS_ContextRecord.EFlags
0x1009CEEC  mov     eax, [ebp+var_s0]
0x1009CEEF  mov     GS_ContextRecord.Ebp, eax
0x1009CEF4  mov     eax, [ebp+4]
0x1009CEF7  mov     GS_ContextRecord.Eip, eax
0x1009CEFC  lea     eax, [ebp+arg_0]
0x1009CEFF  mov     GS_ContextRecord.Esp, eax
0x1009CF04  mov     eax, [ebp+dw]
0x1009CF0A  mov     GS_ContextRecord.ContextFlags, 10001h
0x1009CF14  mov     eax, GS_ContextRecord.Eip
0x1009CF19  mov     GS_ExceptionRecord.ExceptionAddress, eax
0x1009CF1E  mov     GS_ExceptionRecord.ExceptionCode, 0C0000409h
0x1009CF28  mov     GS_ExceptionRecord.ExceptionFlags, 1
0x1009CF32  mov     GS_ExceptionRecord.NumberParameters, 1
0x1009CF3C  push    4
0x1009CF3E  pop     eax
0x1009CF3F  imul    eax, 0
0x1009CF42  mov     GS_ExceptionRecord.ExceptionInformation[eax], 2
0x1009CF4C  push    4
0x1009CF4E  pop     eax
0x1009CF4F  imul    eax, 0
0x1009CF52  mov     ecx, ___security_cookie
0x1009CF58  mov     [ebp+eax+cookie], ecx
0x1009CF5C  push    4
0x1009CF5E  pop     eax
0x1009CF5F  shl     eax, 0
0x1009CF62  mov     ecx, ___security_cookie_complement
0x1009CF68  mov     [ebp+eax+cookie], ecx
0x1009CF6C  push    offset GS_ExceptionPointers; exception_pointers
0x1009CF71  call    ___raise_securityfailure
0x1009CF76  nop
0x1009CF77  leave
0x1009CF78  retn