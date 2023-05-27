include 'win32a.inc'
format PE GUI 4.0 DLL
entry DllMain

section '.text' code readable executable

DllMain:
mov eax,1
ret 12

BoxOutput:
push 0
push caption
push dword [esp + 8 + 4]
push 0
call [MessageBoxA]
ret 4

DebugCode:
mov eax,11111111h
mov edx,22222222h
vpbroadcastd zmm0,eax
vpbroadcastd zmm1,edx
ret

section '.data' data readable writeable
caption  DB 'Mesage box output by DLL (ia32)',0

section '.edata' export data readable
export 'SAMPLELIBRARY32.DLL' ,\
BoxOutput , 'BoxOutput' ,\
DebugCode , 'DebugCode'

section '.idata' import data readable writeable
library user32, 'USER32.DLL'    
include 'api\user32.inc'

data fixups
end data