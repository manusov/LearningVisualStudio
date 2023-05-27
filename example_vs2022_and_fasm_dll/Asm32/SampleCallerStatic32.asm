include 'win32a.inc'
format PE GUI 4.0
entry start

section '.text' code readable executable
start:

push msg
call [BoxOutput]
push 0
call [ExitProcess]

section '.data' data readable writeable
msg  DB  'Message string from caller: static linking (ia32).',0

section '.idata' import data readable writeable
library kernel32, 'KERNEL32.DLL', SampleLibrary, 'SAMPLELIBRARY32.DLL'
include 'api\kernel32.inc'
import SampleLibrary, \
BoxOutput , 'BoxOutput'

