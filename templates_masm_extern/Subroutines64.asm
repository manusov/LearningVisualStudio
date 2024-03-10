;
; https://programminghaven.home.blog/2020/02/16/setup-an-assembly-project-on-visual-studio-2019/
; https://blogs.kpi.kharkov.ua/v2/asm/knigi/.
; Kn1-Rysovanyj-AN-Sistemnoe-programmirovanie-2015.pdf
; Kn2-Rysovanyj-AN-Sistemnoe-programmirovanie-2015.pdf
;
; See instruction for setup project at "_SCREENS_IDE_SETUP_" folder:
; https://github.com/manusov/LearningVisualStudio/tree/master/templates_masm/_SCREENS_IDE_SETUP_
; Plus, Configuration properties \ Additional \ Char set \ Use multibyte (no unicode).
;
; Use EXTERN and PUBLIC directives. Use wsprintfA and WriteConsoleA functions.
; Subroutines module. Variant for 64-bit application.
;


OPTION casemap:none

EXTERN x1:DWORD, x2:DWORD, y1:DWORD, y2:DWORD

.CODE

subroutineAdd PROC
mov eax,x1
add eax,x2
mov y1,eax
ret
subroutineAdd ENDP

subroutineSub PROC
mov eax,x1
sub eax,x2
mov y2,eax
ret
subroutineSub ENDP

END


