.686P
.XMM
.MODEL FLAT, C

.CODE
ALIGN 8
OPTION LANGUAGE: syscall
_calcSine@32 PROC

push ebx
push esi
push edi
push ebp

finit
fld qword ptr [esp + 44]
fld st(0)

mov ebx,[esp + 40]
rdtsc
mov [ebx + 0],eax
mov [ebx + 4],edx

mov esi,[esp + 32]   ; ESI = Repeats, Low32.
mov edi,[esp + 36]   ; EDI = Repeats, High32.

measurement:
mov eax,[esp + 20]   ; EAX = Pointer.
mov edx,[esp + 24]   ; EDX = Iterations count.

calcBlock:
fld st(0)
fsin
fstp qword ptr [eax]
fadd st(0), st(1)
add eax,8
dec edx
jnz calcBlock

sub esi,1
sbb edi,1
mov ebp,esi
or ebp,esi
jnz measurement

rdtsc
sub eax,[ebx + 0]
sbb edx,[ebx + 4]
mov [ebx + 0],eax
mov [ebx + 4],edx

pop ebp
pop edi
pop esi
pop ebx
ret 32

_calcSine@32 ENDP
OPTION LANGUAGE: C
END

