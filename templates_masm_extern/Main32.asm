;
; https://programminghaven.home.blog/2020/02/16/setup-an-assembly-project-on-visual-studio-2019/
; https://blogs.kpi.kharkov.ua/v2/asm/knigi/.
; Kn1-Rysovanyj-AN-Sistemnoe-programmirovanie-2015.pdf
; Kn2-Rysovanyj-AN-Sistemnoe-programmirovanie-2015.pdf
;
; See instruction for setup project at "_SCREENS_IDE_SETUP_" folder:
; https://github.com/manusov/LearningVisualStudio/tree/master/templates_masm/_SCREENS_IDE_SETUP_
; 
; Use EXTERN and PUBLIC directives. Use wsprintfA and WriteConsoleA functions.
; Main module. Variant for 32-bit application.
;


.686
.MODEL flat, stdcall
OPTION casemap:none

STD_OUTPUT_HANDLE = -11
GetStdHandle PROTO:PTR
WriteConsoleA PROTO:PTR, :PTR, :DWORD, :PTR DWORD, :PTR
wsprintfA PROTO C :DWORD, :VARARG
ExitProcess PROTO:DWORD

EXTERN subroutineAdd:PROC, subroutineSub:PROC
PUBLIC x1, x2, y1, y2

.CONST
x1        dd  15
x2        dd  11
szText    db  13, 10, "[ ia32 demo. ][ x1=%d, x2=%d, y1=%d, y2=%d. ]", 13, 10, 0

.DATA
y1        dd  ?
y2        dd  ?
iCounter  dd  ?
sBuffer   db  1024 dup (?)

.CODE
main PROC
; Calculations at subroutines.
call subroutineAdd
call subroutineSub
; https://learn.microsoft.com/ru-ru/windows/win32/api/winuser/nf-winuser-wsprintfa
push y2
push y1
push x2
push x1
push offset szText
mov ebx,offset sBuffer
push ebx
call wsprintfA
add esp,6 * 4       ; For wsprintf, caller must clear stack, even for ia32. Verify ESP is not corrupted?
mov esi,eax
; https://learn.microsoft.com/ru-ru/windows/console/getstdhandle
push STD_OUTPUT_HANDLE
call GetStdHandle
mov ecx,eax
; https://learn.microsoft.com/ru-ru/windows/console/writeconsole
push 0
push offset iCounter
push esi
push ebx
push ecx
call WriteConsoleA
; https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess
push 0              ; First parameter of WinAPI function.
call ExitProcess
main ENDP

END main

