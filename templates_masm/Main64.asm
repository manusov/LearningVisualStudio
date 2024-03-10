;
; https://programminghaven.home.blog/2020/02/16/setup-an-assembly-project-on-visual-studio-2019/
; https://blogs.kpi.kharkov.ua/v2/asm/knigi/.
; Kn1-Rysovanyj-AN-Sistemnoe-programmirovanie-2015.pdf
; Kn2-Rysovanyj-AN-Sistemnoe-programmirovanie-2015.pdf
;
; Generate sound by WinAPI, select frequency and duration.
; Variant for 64-bit windows console application.
;
; See instruction for setup project at "_SCREENS_IDE_SETUP_" folder.
; 


OPTION casemap:none

ExitProcess PROTO:DWORD
Beep PROTO:DWORD, :DWORD

.DATA
frequency  dd  350     ; Sound frequency, Hz.
duration   dd  2500    ; Sound duration, milliseconds.

.CODE
main PROC
sub rsp,28h
; https://learn.microsoft.com/ru-ru/windows/win32/api/utilapiset/nf-utilapiset-beep
mov ecx,frequency      ; RCX = First parameter of WinAPI function.
mov edx,duration       ; RDX = Second parameter of WinAPI function.
call Beep
; https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess
xor ecx,ecx            ; RCX = First parameter of WinAPI function.
call ExitProcess
main ENDP

END
