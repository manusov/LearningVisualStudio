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
; 32-bit DLL.
; Use "Release" and "x86" settings for Library32.asm, DynamicLoader32.asm, StaticLoader32,asm.
; Common \ Configuration type = DLL.
;

.686
.MODEL flat, stdcall
OPTION casemap:none
GetKeyValue PROTO

.CODE

PUBLIC GetKeyValue

DllMain PROC
mov eax,1
ret 12
DllMain ENDP

GetKeyValue PROC EXPORT
lea eax,[ecx + 1988]
ret
GetKeyValue ENDP

END DllMain


