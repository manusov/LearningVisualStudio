include 'win64a.inc'
format PE64 GUI 4.0 DLL
entry DllMain

section '.text' code readable executable

DllMain:
mov eax,1
ret

BoxOutput:
push rbp
mov rbp,rsp
sub rsp,32
and rsp,0FFFFFFFFFFFFFFF0h
mov rdx,rcx
xor rcx,rcx
lea r8,[caption]
xor r9,r9
call [MessageBoxA]
mov rsp,rbp
pop rbp
ret

DebugCode:
mov rax,1111111111111111h
mov rdx,2222222222222222h
vpbroadcastq zmm0,rax
vpbroadcastq zmm1,rdx
ret

section '.data' data readable writeable
caption  DB 'Mesage box output by DLL (x64)',0

section '.edata' export data readable
export 'SAMPLELIBRARY64.DLL' ,\
BoxOutput , 'BoxOutput' ,\
DebugCode , 'DebugCode'

section '.idata' import data readable writeable
library user32, 'USER32.DLL'    
include 'api\user32.inc'

data fixups
end data