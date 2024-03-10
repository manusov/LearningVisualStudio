;
; https://programminghaven.home.blog/2020/02/16/setup-an-assembly-project-on-visual-studio-2019/
; https://blogs.kpi.kharkov.ua/v2/asm/knigi/.
; Kn1-Rysovanyj-AN-Sistemnoe-programmirovanie-2015.pdf
; Kn2-Rysovanyj-AN-Sistemnoe-programmirovanie-2015.pdf
;
; Generate sound by WinAPI, select frequency and duration.
; Variant for 32-bit windows console application.
;
; See instruction for setup project at "_SCREENS_IDE_SETUP_" folder.
;


.686
.MODEL flat, stdcall
OPTION casemap:none

ExitProcess PROTO:DWORD
Beep PROTO:DWORD, :DWORD

.DATA
frequency  dd  700   ; Sound frequency, Hz.
duration   dd  2000  ; Sound duration, milliseconds.

.CODE
main PROC
; https://learn.microsoft.com/ru-ru/windows/win32/api/utilapiset/nf-utilapiset-beep
push duration       ; Second parameter of WinAPI function.
push frequency      ; First parameter of WinAPI function (reverse order of parameters).
call Beep
; https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess
push 0              ; First parameter of WinAPI function.
call ExitProcess
main ENDP

END main

