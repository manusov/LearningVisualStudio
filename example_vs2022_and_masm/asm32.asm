.686P
.XMM
.MODEL FLAT, C

.CODE
ALIGN 8
OPTION LANGUAGE: syscall
_readAvx512@24 PROC

push ebx
push esi
push edi
push ebp

mov ebx,[esp + 40]
rdtsc
mov [ebx + 0],eax
mov [ebx + 4],edx

mov esi,[esp + 32]   ; ESI = Repeats, Low32.
mov edi,[esp + 36]   ; EDI = Repeats, High32.

measurement:
mov eax,[esp + 20]   ; EAX = Pointer.
mov edx,[esp + 24]   ; EDX = Iterations count.

readBlock:
vmovapd zmm0,[eax + 64*0]
vmovapd zmm1,[eax + 64*1]
vmovapd zmm2,[eax + 64*2]
vmovapd zmm3,[eax + 64*3]
vmovapd zmm4,[eax + 64*4]
vmovapd zmm5,[eax + 64*5]
vmovapd zmm6,[eax + 64*6]
vmovapd zmm7,[eax + 64*7]
vmovapd zmm0,[eax + 64*8]
vmovapd zmm1,[eax + 64*9]
vmovapd zmm2,[eax + 64*10]
vmovapd zmm3,[eax + 64*11]
vmovapd zmm4,[eax + 64*12]
vmovapd zmm5,[eax + 64*13]
vmovapd zmm6,[eax + 64*14]
vmovapd zmm7,[eax + 64*15]
dec edx
jnz readBlock

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
ret 24

_readAvx512@24 ENDP
OPTION LANGUAGE: C
END
