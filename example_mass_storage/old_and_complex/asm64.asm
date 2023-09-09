.CODE
ALIGN 16

randomArray PROC

buildArray:
rdrand rax
jnc buildArray
mov [rcx],rax

add rcx,8
dec edx
jnz buildArray

ret

randomArray ENDP

END


