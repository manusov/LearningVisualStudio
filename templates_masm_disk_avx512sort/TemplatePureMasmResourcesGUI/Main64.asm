;
; GUI WINDOW WITH CLASSES. MENU RESOURCE DEMO. X64 VERSION.
; See resource.h and sample_04.rc automatically created files.
;
; https://programminghaven.home.blog/2020/02/16/setup-an-assembly-project-on-visual-studio-2019/
; https://blogs.kpi.kharkov.ua/v2/asm/knigi/. (use VPN)
; https://allasm.ru/urok_10.php (use VPN)
; https://wasm.in/blogs/win32-api-urok-3-prostoe-okno.17/ (use VPN)
; https://www.cyberforum.ru/assembler-windows/thread324485.html (use VPN)
; Kn1-Rysovanyj-AN-Sistemnoe-programmirovanie-2015
; Kn2-Rysovanyj-AN-Sistemnoe-programmirovanie-2015
; Used structures and equations, ported from FASM.
; https://flatassembler.net/
;

OPTION casemap:none

IDI_ICON1            =  101
IDR_MENU1            =  102

WINDOW_BASE_X        =  500
WINDOW_BASE_Y        =  300
WINDOW_WIDTX         =  540
WINDOW_HEIGHT        =  390

CS_VREDRAW           =  00001h
CS_HREDRAW           =  00002h
COLOR_WINDOW         =  5
WHITE_BRUSH          =  0
IDI_APPLICATION      =  32512
IDC_ARROW            =  32512
CW_USEDEFAULT        =  80000000h
WS_OVERLAPPED        =  000000000h
WS_CAPTION           =  000C00000h
WS_SYSMENU           =  000080000h
WS_THICKFRAME        =  000040000h
WS_MINIMIZEBOX       =  000020000h
WS_MAXIMIZEBOX       =  000010000h
WS_OVERLAPPEDWINDOW  =  WS_OVERLAPPED or WS_CAPTION or WS_SYSMENU or WS_THICKFRAME or WS_MINIMIZEBOX or WS_MAXIMIZEBOX
SW_SHOWNORMAL        =  1
WM_CREATE            =  0001h
WM_DESTROY           =  0002h
WM_PAINT             =  000Fh
DT_SINGLELINE        =  00020h
DT_CENTER            =  00001h
DT_VCENTER           =  00004h

PTRTYPE              EQU   DQ
PTRSIZE              =     8
HINSTANCE            EQU   PTRTYPE
HWND                 EQU   PTRTYPE
HDC                  EQU   PTRTYPE
ExitProcess          PROTO :DWORD
GetModuleHandleA     PROTO :PTR
GetStockObject       PROTO :DWORD
LoadIconA            PROTO :PTR, :PTR
LoadCursorA          PROTO :PTR, :PTR
LoadMenuA            PROTO :PTR, :PTR
RegisterClassExA     PROTO :PTR
CreateWindowExA      PROTO :DWORD, :PTR, :PTR, :DWORD, :DWORD, :DWORD, :DWORD, :DWORD, :PTR, :PTR, :PTR, :PTR
ShowWindow           PROTO :PTR, :DWORD
GetMessageA          PROTO :PTR, :PTR, :DWORD, :DWORD
TranslateMessage     PROTO :PTR
DispatchMessageA     PROTO :PTR
DefWindowProcA       PROTO :PTR, :DWORD, :PTR, :PTR
PostQuitMessage      PROTO :DWORD
BeginPaint           PROTO :PTR, :PTR
EndPaint             PROTO :PTR, :PTR
GetClientRect        PROTO :PTR, :PTR
DrawTextA            PROTO :PTR, :PTR, :DWORD, :PTR, :DWORD

POINT STRUC
  x  DD  ?
  y  DD  ?
POINT ENDS

RECT STRUC
  left    DD ?
  top     DD ?
  right   DD ?
  bottom  DD ?
RECT ENDS

WNDCLASSEX STRUC
  cbSize         DD ?
  style          DD ?
  lpfnWndProc    DQ ?
  cbClsExtra     DD ?
  cbWndExtra     DD ?
  hInstance      DQ ?
  hIcon          DQ ?
  hCursor        DQ ?
  hbrBackground  DQ ?
  lpszMenuName   DQ ?
  lpszClassName  DQ ?
  hIconSm        DQ ?
WNDCLASSEX ENDS

MSG STRUC
  hwnd     DQ ?
  message  DD ?,?
  wParam   DQ ?
  lParam   DQ ?
  time     DD ?
  pt       POINT <>
           DD ?
MSG ENDS

PAINTSTRUCT STRUC
  hdc	       DQ ?
  fErase       DD ?
  rcPaint      RECT <>
  fRestore     DD ?
  fIncUpdate   DD ?
  rgbReserved  DB 36 DUP (?)
PAINTSTRUCT ENDS

.DATA
className   DB  "Window class at MASM under Visual Studio 2022 (x64)."        , 0
appName     DB  "Menu resource demo at MASM under Visual Studio 2022 (x64)."  , 0
helloMsg    DB  "Menu resource demo at MASM under Visual Studio 2022 (x64)."  , 0

.DATA?
hInstance   HINSTANCE   ?
hWnd        HWND        ?
wc          WNDCLASSEX  <>
msg         MSG         <>

.CODE
;---------- Application entry point. ----------
main PROC
sub rsp, 28h		; reserved shadow area
xor ecx,ecx
call GetModuleHandleA
test rax,rax
jz failed
mov hInstance,rax
lea rbx,hInstance
lea rsi,wc
lea rdi,msg
xor ebp,ebp
;
push qword ptr [rbx]
pop [rsi + WNDCLASSEX.hInstance]
; mov rax,[rbx]
; mov [rsi + WNDCLASSEX.hInstance],rax
;
mov [rsi + WNDCLASSEX.cbSize],sizeof(WNDCLASSEX)
mov [rsi + WNDCLASSEX.style],CS_HREDRAW or CS_VREDRAW
lea rax,wndProc
mov [rsi + WNDCLASSEX.lpfnWndProc],rax
mov [rsi + WNDCLASSEX.hbrBackground],COLOR_WINDOW + 1
mov ecx,WHITE_BRUSH
call GetStockObject
test rax,rax
jz @f
mov [rsi + WNDCLASSEX.hbrBackground],rax
@@:
mov [rsi + WNDCLASSEX.lpszMenuName],IDR_MENU1     ; mov [rsi + WNDCLASSEX.lpszMenuName],rbp
mov rax,offset className
mov [rsi + WNDCLASSEX.lpszClassName],rax
mov edx,IDI_ICON1                                 ;  mov edx,IDI_APPLICATION
mov rcx,hInstance                                 ;  xor ecx,ecx
call LoadIconA
mov [rsi + WNDCLASSEX.hIcon],rax
mov [rsi + WNDCLASSEX.hIconSm],rax
mov edx,IDC_ARROW
xor ecx,ecx
call LoadCursorA
mov [rsi + WNDCLASSEX.hCursor],rax
mov [rsi + WNDCLASSEX.cbClsExtra],ebp
mov [rsi + WNDCLASSEX.cbWndExtra],ebp
mov rcx,rsi
call RegisterClassExA
test rax,rax
jz failed
push rbp
push [hInstance]
push rbp
push rbp
; Changed default base and size.
; mov eax,CW_USEDEFAULT
; push rax
; push rax
; push rax
; push rax
push WINDOW_HEIGHT
push WINDOW_WIDTX
push WINDOW_BASE_Y
push WINDOW_BASE_X
; End of changes.
mov r9d,WS_OVERLAPPEDWINDOW
mov r8,offset appName
mov rdx,offset className
xor ecx,ecx
sub rsp,32
call CreateWindowExA
add rsp,32 + 64
test rax,rax
jz failed
mov [rbx + PTRSIZE],rax
mov edx,SW_SHOWNORMAL
mov rcx,[rbx + PTRSIZE]
call ShowWindow
@@:
xor r9d,r9d
xor r8d,r8d
xor edx,edx
mov rcx,rdi
call GetMessageA
test rax,rax
jz @f
mov rcx,rdi
call TranslateMessage
mov rcx,rdi
call DispatchMessageA
jmp @b
@@:
mov rcx,[rdi + MSG.wParam]
call ExitProcess
failed:
mov ecx,-1
call ExitProcess
main ENDP
;---------- Window callback procedure. ----------
wndProc PROC
push rbp
push rdi
mov rdi,rsp
sub rsp,PTRSIZE + sizeof(PAINTSTRUCT) + sizeof(RECT)
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32 + 8
push rbx
mov rbx,rcx
cmp edx,WM_DESTROY
je wmDestroy
cmp edx,WM_PAINT
je wmPaint
call DefWindowProcA
wmDone:
pop rbx
mov rsp,rdi
pop rdi
pop rbp
ret
wmDestroy:
xor ecx,ecx
call PostQuitMessage
jmp wmDoneZero
wmPaint:
lea rdx,[rbp + sizeof(RECT)]
mov rcx,rbx
call BeginPaint
test rax,rax
jz paintSkip
mov [rbp + sizeof(PAINTSTRUCT) + sizeof(RECT)],rax
mov rdx,rbp
mov rcx,rbx
call GetClientRect
test rax,rax
jz rectSkip
push 0
push DT_SINGLELINE or DT_CENTER or DT_VCENTER
mov r9,rbp
mov r8d,-1
mov rdx,offset helloMsg
mov rcx,[rbp + sizeof(PAINTSTRUCT) + sizeof(RECT)]
sub rsp,32
call DrawTextA
add rsp,32 + 16
rectSkip:
lea rdx,[rbp + sizeof(RECT)]
mov rcx,rbx
call EndPaint
paintSkip:
wmDoneZero:
xor eax,eax
jmp wmDone
wndProc  ENDP

END

