.CODE
ALIGN 16
calcSine PROC

push rbx
mov rbx,rdx

finit
fld qword ptr [rsp + 48]
fld st(0)

rdtsc
shl rdx,32
lea r10,[rax + rdx]

measurement:
mov rax,rcx  ; RAX = Pointer
mov rdx,rbx  ; RDX = Iterations count

calcBlock:
fld st(0)
fsin
fstp qword ptr [rax]
fadd st(0), st(1)
add rax,8
dec rdx
jnz calcBlock

dec r8
jnz measurement

rdtsc
shl rdx,32
or rax,rdx
sub rax,r10
mov [r9],rax

pop rbx
ret

calcSine ENDP
END


