;
; GUI WINDOW WITH CLASSES. IA32 VERSION.
; https://programminghaven.home.blog/2020/02/16/setup-an-assembly-project-on-visual-studio-2019/
; https://blogs.kpi.kharkov.ua/v2/asm/knigi/.
; https://allasm.ru/urok_10.php
; https://wasm.in/blogs/win32-api-urok-3-prostoe-okno.17/
; https://www.cyberforum.ru/assembler-windows/thread324485.html
; Kn1-Rysovanyj-AN-Sistemnoe-programmirovanie-2015
; Kn2-Rysovanyj-AN-Sistemnoe-programmirovanie-2015
; Used structures and equations, ported from FASM.
; https://flatassembler.net/
;

.686
.XMM
.MODEL flat, stdcall
OPTION casemap:none

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

PTRTYPE              EQU   DD
PTRSIZE              =     4
HINSTANCE            EQU   PTRTYPE
HWND                 EQU   PTRTYPE
HDC                  EQU   PTRTYPE
ExitProcess          PROTO :DWORD
GetModuleHandleA     PROTO :PTR
GetStockObject       PROTO :DWORD
LoadIconA            PROTO :PTR, :PTR
LoadCursorA          PROTO :PTR, :PTR
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
  lpfnWndProc    DD ?
  cbClsExtra     DD ?
  cbWndExtra     DD ?
  hInstance      DD ?
  hIcon          DD ?
  hCursor        DD ?
  hbrBackground  DD ?
  lpszMenuName   DD ?
  lpszClassName  DD ?
  hIconSm        DD ?
WNDCLASSEX ENDS

MSG STRUC
  hwnd     DD ?
  message  DD ?
  wParam   DD ?
  lParam   DD ?
  time     DD ?
  pt       POINT <>
MSG ENDS

PAINTSTRUCT STRUC
  hdc          DD ?
  fErase       DD ?
  rcPaint      RECT <>
  fRestore     DD ?
  fIncUpdate   DD ?
  rgbReserved  DD 32 DUP (?)
PAINTSTRUCT ENDS

.DATA
className   DB  "First class at MASM under Visual Studio 2022 (ia32)."           , 0
appName     DB  "First GUI application at MASM under Visual Studio 2022 (ia32)." , 0
helloMsg    DB  "First message at MASM under Visual Studio 2022 (ia32)."         , 0

.DATA?
hInstance   HINSTANCE   ?
hWnd        HWND        ?
wc          WNDCLASSEX  <>
msg         MSG         <>

.CODE
;---------- Application entry point. ----------
main PROC
push 0
call GetModuleHandleA
test eax,eax
jz failed
mov hInstance,eax
mov ebx,offset hInstance
mov esi,offset wc
mov edi,offset msg
xor ebp,ebp
push dword ptr [ebx]
pop [esi + WNDCLASSEX.hInstance]
mov [esi + WNDCLASSEX.cbSize],sizeof(WNDCLASSEX)
mov [esi + WNDCLASSEX.style],CS_HREDRAW or CS_VREDRAW
mov [esi + WNDCLASSEX.lpfnWndProc],offset wndProc
mov [esi + WNDCLASSEX.hbrBackground],COLOR_WINDOW + 1
push WHITE_BRUSH
call GetStockObject
test eax,eax
jz @f
mov [esi + WNDCLASSEX.hbrBackground],eax
@@:
mov [esi + WNDCLASSEX.lpszMenuName],ebp
mov [esi + WNDCLASSEX.lpszClassName],offset className
push IDI_APPLICATION
push ebp
call LoadIconA
mov [esi + WNDCLASSEX.hIcon],eax
mov [esi + WNDCLASSEX.hIconSm],eax
push IDC_ARROW
push ebp
call LoadCursorA
mov [esi + WNDCLASSEX.hCursor],eax
mov [esi + WNDCLASSEX.cbClsExtra],ebp
mov [esi + WNDCLASSEX.cbWndExtra],ebp
push esi
call RegisterClassExA
test eax,eax
jz failed
push ebp
push [hInstance]
push ebp
push ebp
; Changed default base and size.
; mov eax,CW_USEDEFAULT
; push eax
; push eax
; push eax
; push eax
push WINDOW_HEIGHT
push WINDOW_WIDTX
push WINDOW_BASE_Y
push WINDOW_BASE_X
; End of changes.
push WS_OVERLAPPEDWINDOW
push offset appName
push offset className
push ebp
call CreateWindowExA
test eax,eax
jz failed
mov [ebx + PTRSIZE],eax
push SW_SHOWNORMAL
push dword ptr [ebx + PTRSIZE]
call ShowWindow
@@:
push ebp
push ebp
push ebp
push edi
call GetMessageA
test eax,eax
jz @f
push edi
call TranslateMessage
push edi
call DispatchMessageA
jmp @b
@@:
push [edi + MSG.wParam]
call ExitProcess
failed:
push -1
call ExitProcess
main ENDP
;---------- Window callback procedure. ----------
wndProc PROC
push ebx
mov ebx,[esp + 8]
mov eax,[esp + 12]
mov ecx,[esp + 16]
mov edx,[esp + 20]
cmp eax,WM_DESTROY
je wmDestroy
cmp eax,WM_PAINT
je wmPaint
push edx
push ecx
push eax
push ebx
call DefWindowProcA
wmDone:
pop ebx
ret 16
wmDestroy:
push 0
call PostQuitMessage
jmp wmDoneZero
wmPaint:
sub esp,PTRSIZE + sizeof(PAINTSTRUCT) + sizeof(RECT)
lea eax,[esp + sizeof(RECT)]
push eax
push ebx
call BeginPaint
test eax,eax
jz paintSkip
mov [esp + sizeof(PAINTSTRUCT) + sizeof(RECT)],eax
mov eax,esp
push eax
push ebx
call GetClientRect
test eax,eax
jz rectSkip
mov eax,esp
mov edx,[esp + sizeof(PAINTSTRUCT) + sizeof(RECT)]
push DT_SINGLELINE or DT_CENTER or DT_VCENTER
push eax
push -1
push offset helloMsg
push edx
call DrawTextA
rectSkip:
lea eax,[esp + sizeof(RECT)]
push eax
push ebx
call EndPaint
paintSkip:
add esp,PTRSIZE + sizeof(PAINTSTRUCT) + sizeof(RECT)
wmDoneZero:
xor eax,eax
jmp wmDone
wndProc  ENDP

END main

