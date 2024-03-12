;
; Multi-thread console application template. Threads management tests and benchmarking.
;
; This template is PART, extracted from:
; Mass storage performance test. Classes source file edition.
; https://github.com/manusov/LearningVisualStudio/tree/master/framework_disk_test_classes
; More projects at:
; https://github.com/manusov?tab=repositories
; Special thanks for C++ lessons:
; https://ravesli.com/uroki-cpp/
;
; Assembly subroutines for performance-critical fragments. Variant for x64.
;


.CODE
ALIGN 16


readAvx512 PROC

push rbx
mov r11,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,10*16                    ; 10 128-bit SSE registers XMM6-XMM15 must be
vmovapd [rsp + 16*0],xmm6        ; non-volatile by convention, note ALIGN16
vmovapd [rsp + 16*1],xmm7        ; TODO. Check code size and performance if use volatile ZMM16-ZMM31 without save XMM6-XMM15.
vmovapd [rsp + 16*2],xmm8
vmovapd [rsp + 16*3],xmm9
vmovapd [rsp + 16*4],xmm10
vmovapd [rsp + 16*5],xmm11
vmovapd [rsp + 16*6],xmm12
vmovapd [rsp + 16*7],xmm13
vmovapd [rsp + 16*8],xmm14
vmovapd [rsp + 16*9],xmm15

mov rbx,rdx

rdtsc
shl rdx,32
lea r10,[rax + rdx]

measurement:
mov rax,rcx  ; RAX = Pointer
mov rdx,rbx  ; RDX = Iterations count

readBlock:
vmovapd zmm0,[rax + 64*0]
vmovapd zmm1,[rax + 64*1]
vmovapd zmm2,[rax + 64*2]
vmovapd zmm3,[rax + 64*3]
vmovapd zmm4,[rax + 64*4]
vmovapd zmm5,[rax + 64*5]
vmovapd zmm6,[rax + 64*6]
vmovapd zmm7,[rax + 64*7]
vmovapd zmm8,[rax + 64*8]
vmovapd zmm9,[rax + 64*9]
vmovapd zmm10,[rax + 64*10]
vmovapd zmm11,[rax + 64*11]
vmovapd zmm12,[rax + 64*12]
vmovapd zmm13,[rax + 64*13]
vmovapd zmm14,[rax + 64*14]
vmovapd zmm15,[rax + 64*15]
dec rdx
jnz readBlock

dec r8
jnz measurement

rdtsc
shl rdx,32
or rax,rdx
sub rax,r10
mov [r9],rax

vmovapd xmm6,[rsp + 16*0]
vmovapd xmm7,[rsp + 16*1]
vmovapd xmm8,[rsp + 16*2]
vmovapd xmm9,[rsp + 16*3]
vmovapd xmm10,[rsp + 16*4]
vmovapd xmm11,[rsp + 16*5]
vmovapd xmm12,[rsp + 16*6]
vmovapd xmm13,[rsp + 16*7]
vmovapd xmm14,[rsp + 16*8]     ; 10 128-bit SSE registers XMM6-XMM15 must be
vmovapd xmm15,[rsp + 16*9]     ; non-volatile by convention, note ALIGN16
mov rsp,r11
pop rbx
ret

readAvx512 ENDP


END
