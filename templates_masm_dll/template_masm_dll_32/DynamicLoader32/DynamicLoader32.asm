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
; Application use 32-bit DLL, variant for dynamical import.
; Use "Release" and "x86" settings for Library32.asm, DynamicLoader32.asm, StaticLoader32,asm.
; Library file Library32.dll dynamically loaded by DynamicLoader32.exe, must be at same directory.
;

.686
.MODEL flat, stdcall
OPTION casemap:none
LoadLibraryA PROTO:PTR
FreeLibrary PROTO:PTR
GetProcAddress PROTO:PTR, :PTR
GetLastError PROTO
ExitProcess PROTO:DWORD

.CONST
szNameDll    db  "library32.dll" , 0
szNameFnc    db  "_GetKeyValue@0" , 0

.DATA
handleDll    dd  ?
ptrGetValue  dd  ?

.CODE
main PROC

push offset szNameDll
call LoadLibraryA
test eax,eax
jz exitLoader          ; Go error branch for return 0 if library not found.
mov handleDll,eax

push offset szNameFnc
push eax
call GetProcAddress
test eax,eax
jz exitLoader          ; Go error branch for return 0 if library function not found.
mov ptrGetValue,eax

mov ecx,1              ; Input ECX = 1.
call ptrGetValue       ; Return EAX = ECX + 1988.
xchg ebx,eax           ; Save at EBX.

push handleDll
call FreeLibrary
test eax,eax
jz exitLoader          ; Go error branch for return 0 if library unload failed.

mov ecx,ebx            ; ECX = returned result ( 1988 + 1 = 1989 ) if function called OK.
jmp returnLoader

exitLoader:
xor ecx,ecx
returnLoader:
push ecx               ; Push ECX = return code used for library call validity.
call ExitProcess
main ENDP

END main

