include 'win64a.inc'
format PE64 GUI 5.0
entry start

section '.text' code readable executable
start:
sub rsp,8 * 5
lea rcx,[libraryName]
call [LoadLibrary]
test rax,rax
jz .skip
mov [libraryHandle],rax
xchg rcx,rax
lea rdx,[functionName]
call [GetProcAddress]
test rax,rax
jz .skip
mov [functionAddress],rax
lea rcx,[messageString]
call rax
.skip:
xor ecx,ecx
call [ExitProcess]

section '.data' data readable writeable
libraryName      DB  'SAMPLELIBRARY64.DLL', 0
functionName     DB  'BoxOutput', 0
messageString    DB  'Message string from caller: dynamic linking (x64).',0
libraryHandle    DQ  ?
functionAddress  DQ  ?

section '.idata' import data readable writeable
library kernel32, 'KERNEL32.DLL'
include 'api\kernel32.inc'


