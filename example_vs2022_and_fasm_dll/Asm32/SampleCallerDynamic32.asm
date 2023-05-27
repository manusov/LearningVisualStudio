include 'win32a.inc'
format PE GUI 4.0
entry start

section '.text' code readable executable
start:

push libraryName
call [LoadLibrary]
test eax,eax
jz .skip
mov [libraryHandle],eax
push functionName
push eax
call [GetProcAddress]
test eax,eax
jz .skip
mov [functionAddress],eax
push messageString
call eax
.skip:
push 0
call [ExitProcess]

section '.data' data readable writeable
libraryName      DB  'SAMPLELIBRARY32.DLL', 0
functionName     DB  'BoxOutput', 0
messageString    DB  'Message string from caller: dynamic linking (ia32).',0
libraryHandle    DD  ?
functionAddress  DD  ?

section '.idata' import data readable writeable
library kernel32, 'KERNEL32.DLL'
include 'api\kernel32.inc'


