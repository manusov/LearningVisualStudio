.686P
.XMM
.MODEL FLAT, C

.CODE
ALIGN 8
OPTION LANGUAGE: syscall

_randomArray@8 PROC

mov ecx,[esp + 4]
mov edx,[esp + 8]

buildArray:
rdrand eax
jnc buildArray
mov [ecx],eax

waitArray:
rdrand eax
jnc waitArray
mov [ecx + 4],eax

add ecx,8
dec edx
jnz buildArray

_randomArray@8 ENDP

OPTION LANGUAGE: C
END
