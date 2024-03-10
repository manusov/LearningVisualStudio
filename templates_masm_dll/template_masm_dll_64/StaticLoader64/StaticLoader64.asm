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
; Application use 64-bit DLL, variant for static import.
; Use "Release" and "x64" settings for Library64.asm, DynamicLoader64.asm, StaticLoader64,asm.
; Add reference: Linker \ Input \ Library64.lib.
; File Library64.lib must be in the project directory with file StaticLoader64.asm.
;


OPTION casemap:none
ExitProcess PROTO:DWORD
GetKeyValue PROTO
.CODE
main PROC
sub rsp,28h
mov ecx,10           ; Input ECX = 10.
call GetKeyValue     ; Return EAX = ECX + 1986.
xchg ecx,eax         ; EAX = return code used for library call validity.
call ExitProcess
main ENDP
END

