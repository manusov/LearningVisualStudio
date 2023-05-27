include 'win64a.inc'
format PE64 GUI 5.0
entry start

section '.text' code readable executable
start:
sub rsp,8 * 5
lea rcx,[msg]
call [BoxOutput]
xor ecx,ecx
call [ExitProcess]

section '.data' data readable writeable
msg  DB  'Message string from caller: static linking (x64).',0

section '.idata' import data readable writeable
library kernel32, 'KERNEL32.DLL', SampleLibrary, 'SAMPLELIBRARY64.DLL'
include 'api\kernel32.inc'
import SampleLibrary, \
BoxOutput , 'BoxOutput'

