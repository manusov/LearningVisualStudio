;
; https://programminghaven.home.blog/2020/02/16/setup-an-assembly-project-on-visual-studio-2019/
; https://blogs.kpi.kharkov.ua/v2/asm/knigi/.
; Kn1-Rysovanyj-AN-Sistemnoe-programmirovanie-2015.pdf
; Kn2-Rysovanyj-AN-Sistemnoe-programmirovanie-2015.pdf
;
; https://learn.microsoft.com/ru-ru/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibrarya
; https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-freelibrary
; https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getprocaddress
; https://learn.microsoft.com/ru-ru/windows/win32/dlls/dllmain
;
; Application use 64-bit DLL, variant for dynamical import.
; Use "Release" and "x64" settings for Library64.asm, DynamicLoader64.asm, StaticLoader64,asm.
; Library file Library64.dll dynamically loaded by DynamicLoader64.exe, must be at same directory.
;


OPTION casemap:none
LoadLibraryA PROTO:PTR
FreeLibrary PROTO:PTR
GetProcAddress PROTO:PTR, :PTR
GetLastError PROTO
ExitProcess PROTO:DWORD

.CONST
szNameDll    db  "Library64.dll" , 0
szNameFnc    db  "GetKeyValue" , 0

.DATA
handleDll    dq  ?
ptrGetValue  dq  ?

.CODE
main PROC
sub rsp,28h

lea rcx,szNameDll
call LoadLibraryA
test rax,rax
jz exitLoader          ; Go error branch for return 0 if library not found.
mov handleDll,rax

xchg rcx,rax
lea rdx,szNameFnc
call GetProcAddress
test rax,rax
jz exitLoader          ; Go error branch for return 0 if library function not found.
mov ptrGetValue,rax

mov ecx,1              ; Input ECX = 1.
call ptrGetValue       ; Return EAX = ECX + 1986.
xchg ebx,eax           ; Save at EBX.

mov rcx,handleDll
call FreeLibrary
test rax,rax
jz exitLoader          ; Go error branch for return 0 if library unload failed.

mov ecx,ebx            ; ECX = returned result ( 1986 + 1 = 1987 ) if function called OK.
jmp returnLoader

exitLoader:
xor ecx,ecx
returnLoader:
call ExitProcess       ; Here ECX = return code used for library call validity.
main ENDP

END
