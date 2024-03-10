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
; Main module. Variant for 64-bit application.
;


OPTION casemap:none

STD_OUTPUT_HANDLE = -11
GetStdHandle PROTO:PTR
WriteConsoleA PROTO:PTR, :PTR, :DWORD, :PTR DWORD, :PTR
wsprintfA PROTO C :DWORD, :VARARG
ExitProcess PROTO:DWORD

EXTERN subroutineAdd:PROC, subroutineSub:PROC
PUBLIC x1, x2, y1, y2

.CONST
x1        dd  34
x2        dd  15
szText    db  13, 10, "[ x64 demo. ][ x1=%d, x2=%d, y1=%d, y2=%d. ]", 13, 10, 0

.DATA
y1        dd  ?
y2        dd  ?
iCounter  dd  ?
sBuffer   db  1024 dup (?)

.CODE
main PROC
sub rsp,28h
; Calculations at subroutines.
call subroutineAdd
call subroutineSub
; https://learn.microsoft.com/ru-ru/windows/win32/api/winuser/nf-winuser-wsprintfa
mov eax,y2
push rax
mov eax,y1
push rax
mov r9d,x2
mov r8d,x1
lea rdx,szText
lea rbx,sBuffer
mov rcx,rbx
sub rsp,32
call wsprintfA
add rsp,32 + 16
xchg esi,eax
; https://learn.microsoft.com/ru-ru/windows/console/getstdhandle
mov ecx,STD_OUTPUT_HANDLE
call GetStdHandle
xchg rcx,rax
; https://learn.microsoft.com/ru-ru/windows/console/writeconsole
push 0
push 0
lea r9,iCounter
mov r8d,esi
mov rdx,rbx
sub rsp,32
call WriteConsoleA
add rsp,32 + 16
; https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess
xor ecx,ecx            ; RCX = First parameter of WinAPI function.
call ExitProcess
main ENDP

END
