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
; Application use 32-bit DLL, variant for static import.
; Use "Release" and "x86" settings for Library32.asm, DynamicLoader32.asm, StaticLoader32,asm.
; Add reference: Linker \ Input \ Library32.lib.
; File Library32.lib must be in the project directory with file StaticLoader32.asm.
;

.686
.MODEL flat, stdcall
OPTION casemap:none
ExitProcess PROTO:DWORD
GetKeyValue PROTO
.CODE
main PROC
mov ecx,5            ; Input ECX = 5.
call GetKeyValue     ; Return EAX = ECX + 1988.
push eax             ; EAX = return code used for library call validity.
call ExitProcess
main ENDP
END main


