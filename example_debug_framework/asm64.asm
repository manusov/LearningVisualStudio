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


;---------- RCX = Source pointer, RDX = Destination pointer ----------;
transposeBigToLittle PROC

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

;---------- Load source data ----------;
vmovapd zmm0,[rcx]
add rcx,r8
vmovapd zmm1,[rcx]
add rcx,r8
vmovapd zmm2,[rcx]
add rcx,r8
vmovapd zmm3,[rcx]
add rcx,r8
vmovapd zmm4,[rcx]
add rcx,r8
vmovapd zmm5,[rcx]
add rcx,r8
vmovapd zmm6,[rcx]
add rcx,r8
vmovapd zmm7,[rcx]
;---------- Big: x4 transfers ----------;
mov eax,0F0h
vinsertf64x4 zmm8,zmm0,ymm4,1
vinsertf64x4 zmm9,zmm1,ymm5,1
vinsertf64x4 zmm10,zmm2,ymm6,1
vinsertf64x4 zmm11,zmm3,ymm7,1
kmovw k1,eax
vextractf64x4 ymm12,zmm0,1
vextractf64x4 ymm13,zmm1,1
vextractf64x4 ymm14,zmm2,1
vextractf64x4 ymm15,zmm3,1
vmovapd zmm12{k1},zmm4
vmovapd zmm13{k1},zmm5
vmovapd zmm14{k1},zmm6
vmovapd zmm15{k1},zmm7
;---------- Middle: x2 transfers ----------;
mov eax,11001100b
mov ecx,00110011b
valignq zmm16,zmm8,zmm8,2
valignq zmm17,zmm9,zmm9,2
valignq zmm18,zmm10,zmm10,6
valignq zmm19,zmm11,zmm11,6
kmovw k1,eax
kmovw k2,ecx
valignq zmm20,zmm12,zmm12,2
valignq zmm21,zmm13,zmm13,2
valignq zmm22,zmm14,zmm14,6
valignq zmm23,zmm15,zmm15,6
vmovapd zmm8{k1},zmm18
vmovapd zmm9{k1},zmm19
vmovapd zmm10{k2},zmm16
vmovapd zmm11{k2},zmm17
vmovapd zmm12{k1},zmm22
vmovapd zmm13{k1},zmm23
vmovapd zmm14{k2},zmm20
vmovapd zmm15{k2},zmm21
;---------- Little: x1 transfers ----------;
vunpcklpd zmm24,zmm8,zmm9
vunpckhpd zmm25,zmm8,zmm9
vunpcklpd zmm26,zmm10,zmm11
vunpckhpd zmm27,zmm10,zmm11
vunpcklpd zmm28,zmm12,zmm13
vunpckhpd zmm29,zmm12,zmm13
vunpcklpd zmm30,zmm14,zmm15
vunpckhpd zmm31,zmm14,zmm15
;---------- Store destination data ----------;
vmovapd [rdx],zmm24
add rdx,r8
vmovapd [rdx],zmm25
add rdx,r8
vmovapd [rdx],zmm26
add rdx,r8
vmovapd [rdx],zmm27
add rdx,r8
vmovapd [rdx],zmm28
add rdx,r8
vmovapd [rdx],zmm29
add rdx,r8
vmovapd [rdx],zmm30
add rdx,r8
vmovapd [rdx],zmm31

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

ret
transposeBigToLittle ENDP


;---------- RCX = Source pointer, RDX = Destination pointer ----------;
transposeLittleToBig PROC

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

;---------- Load source data ----------;
vmovapd zmm0,[rcx]
add rcx,r8
vmovapd zmm1,[rcx]
add rcx,r8
vmovapd zmm2,[rcx]
add rcx,r8
vmovapd zmm3,[rcx]
add rcx,r8
vmovapd zmm4,[rcx]
add rcx,r8
vmovapd zmm5,[rcx]
add rcx,r8
vmovapd zmm6,[rcx]
add rcx,r8
vmovapd zmm7,[rcx]
;---------- Little: x1 transfers ----------;
vunpcklpd zmm8,zmm0,zmm1
vunpckhpd zmm9,zmm0,zmm1
vunpcklpd zmm10,zmm2,zmm3
vunpckhpd zmm11,zmm2,zmm3
vunpcklpd zmm12,zmm4,zmm5
vunpckhpd zmm13,zmm4,zmm5
vunpcklpd zmm14,zmm6,zmm7
vunpckhpd zmm15,zmm6,zmm7
;---------- Middle: x2 transfers ----------;
mov eax,11001100b
mov ecx,00110011b
valignq zmm16,zmm8,zmm8,2
valignq zmm17,zmm9,zmm9,2
valignq zmm18,zmm10,zmm10,6
valignq zmm19,zmm11,zmm11,6
kmovw k1,eax
kmovw k2,ecx
valignq zmm20,zmm12,zmm12,2
valignq zmm21,zmm13,zmm13,2
valignq zmm22,zmm14,zmm14,6
valignq zmm23,zmm15,zmm15,6
vmovapd zmm8{k1},zmm18
vmovapd zmm9{k1},zmm19
vmovapd zmm10{k2},zmm16
vmovapd zmm11{k2},zmm17
vmovapd zmm12{k1},zmm22
vmovapd zmm13{k1},zmm23
vmovapd zmm14{k2},zmm20
vmovapd zmm15{k2},zmm21
;---------- Big: x4 transfers ----------;
mov eax,0F0h
vinsertf64x4 zmm24,zmm8,ymm12,1
vinsertf64x4 zmm25,zmm9,ymm13,1
vinsertf64x4 zmm26,zmm10,ymm14,1
vinsertf64x4 zmm27,zmm11,ymm15,1
kmovw k1,eax
vextractf64x4 ymm28,zmm8,1
vextractf64x4 ymm29,zmm9,1
vextractf64x4 ymm30,zmm10,1
vextractf64x4 ymm31,zmm11,1
vmovapd zmm28{k1},zmm12
vmovapd zmm29{k1},zmm13
vmovapd zmm30{k1},zmm14
vmovapd zmm31{k1},zmm15
;---------- Store destination data ----------;
vmovapd [rdx],zmm24
add rdx,r8
vmovapd [rdx],zmm25
add rdx,r8
vmovapd [rdx],zmm26
add rdx,r8
vmovapd [rdx],zmm27
add rdx,r8
vmovapd [rdx],zmm28
add rdx,r8
vmovapd [rdx],zmm29
add rdx,r8
vmovapd [rdx],zmm30
add rdx,r8
vmovapd [rdx],zmm31

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

ret
transposeLittleToBig ENDP






;---------- RCX = Source pointer, RDX = Destination pointer ----------;
transposeBigToLittleNT PROC

; imul r10,r8,32                    ; R10 = addend for prefetch address, one core advances to next row
; lea r10,[r8 + 64*2]               ; one core advances to next 8 columns
; mov r10,4096
;

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

;---------- Load source data ----------;
;-
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm0,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm1,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm2,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm3,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm4,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm5,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm6,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm7,zmmword ptr [rcx]
;-


;-
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm0,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm1,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm2,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm3,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm4,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm5,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm6,zmmword ptr [rcx]
;add rcx,r8
;; prefetchnta [rcx + r10]            ; prefetch
;vmovntdqa zmm7,zmmword ptr [rcx]
;-


vmovapd zmm0,[rcx]
add rcx,r8
vmovapd zmm1,[rcx]
add rcx,r8
vmovapd zmm2,[rcx]
add rcx,r8
vmovapd zmm3,[rcx]
add rcx,r8
vmovapd zmm4,[rcx]
add rcx,r8
vmovapd zmm5,[rcx]
add rcx,r8
vmovapd zmm6,[rcx]
add rcx,r8
vmovapd zmm7,[rcx]



;---------- Big: x4 transfers ----------;
mov eax,0F0h
vinsertf64x4 zmm8,zmm0,ymm4,1
vinsertf64x4 zmm9,zmm1,ymm5,1
vinsertf64x4 zmm10,zmm2,ymm6,1
vinsertf64x4 zmm11,zmm3,ymm7,1
kmovw k1,eax
vextractf64x4 ymm12,zmm0,1
vextractf64x4 ymm13,zmm1,1
vextractf64x4 ymm14,zmm2,1
vextractf64x4 ymm15,zmm3,1
vmovapd zmm12{k1},zmm4
vmovapd zmm13{k1},zmm5
vmovapd zmm14{k1},zmm6
vmovapd zmm15{k1},zmm7
;---------- Middle: x2 transfers ----------;
mov eax,11001100b
mov ecx,00110011b
valignq zmm16,zmm8,zmm8,2
valignq zmm17,zmm9,zmm9,2
valignq zmm18,zmm10,zmm10,6
valignq zmm19,zmm11,zmm11,6
kmovw k1,eax
kmovw k2,ecx
valignq zmm20,zmm12,zmm12,2
valignq zmm21,zmm13,zmm13,2
valignq zmm22,zmm14,zmm14,6
valignq zmm23,zmm15,zmm15,6
vmovapd zmm8{k1},zmm18
vmovapd zmm9{k1},zmm19
vmovapd zmm10{k2},zmm16
vmovapd zmm11{k2},zmm17
vmovapd zmm12{k1},zmm22
vmovapd zmm13{k1},zmm23
vmovapd zmm14{k2},zmm20
vmovapd zmm15{k2},zmm21
;---------- Little: x1 transfers ----------;
vunpcklpd zmm24,zmm8,zmm9
vunpckhpd zmm25,zmm8,zmm9
vunpcklpd zmm26,zmm10,zmm11
vunpckhpd zmm27,zmm10,zmm11
vunpcklpd zmm28,zmm12,zmm13
vunpckhpd zmm29,zmm12,zmm13
vunpcklpd zmm30,zmm14,zmm15
vunpckhpd zmm31,zmm14,zmm15
;---------- Store destination data ----------;
vmovntpd zmmword ptr [rdx],zmm24
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm25
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm26
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm27
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm28
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm29
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm30
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm31

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

ret
transposeBigToLittleNT ENDP


;---------- RCX = Source pointer, RDX = Destination pointer ----------;
transposeLittleToBigNT PROC

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

;---------- Load source data ----------;
vmovntdqa zmm0,zmmword ptr [rcx]
add rcx,r8
vmovntdqa zmm1,zmmword ptr [rcx]
add rcx,r8
vmovntdqa zmm2,zmmword ptr [rcx]
add rcx,r8
vmovntdqa zmm3,zmmword ptr [rcx]
add rcx,r8
vmovntdqa zmm4,zmmword ptr [rcx]
add rcx,r8
vmovntdqa zmm5,zmmword ptr [rcx]
add rcx,r8
vmovntdqa zmm6,zmmword ptr [rcx]
add rcx,r8
vmovntdqa zmm7,zmmword ptr [rcx]
;---------- Little: x1 transfers ----------;
vunpcklpd zmm8,zmm0,zmm1
vunpckhpd zmm9,zmm0,zmm1
vunpcklpd zmm10,zmm2,zmm3
vunpckhpd zmm11,zmm2,zmm3
vunpcklpd zmm12,zmm4,zmm5
vunpckhpd zmm13,zmm4,zmm5
vunpcklpd zmm14,zmm6,zmm7
vunpckhpd zmm15,zmm6,zmm7
;---------- Middle: x2 transfers ----------;
mov eax,11001100b
mov ecx,00110011b
valignq zmm16,zmm8,zmm8,2
valignq zmm17,zmm9,zmm9,2
valignq zmm18,zmm10,zmm10,6
valignq zmm19,zmm11,zmm11,6
kmovw k1,eax
kmovw k2,ecx
valignq zmm20,zmm12,zmm12,2
valignq zmm21,zmm13,zmm13,2
valignq zmm22,zmm14,zmm14,6
valignq zmm23,zmm15,zmm15,6
vmovapd zmm8{k1},zmm18
vmovapd zmm9{k1},zmm19
vmovapd zmm10{k2},zmm16
vmovapd zmm11{k2},zmm17
vmovapd zmm12{k1},zmm22
vmovapd zmm13{k1},zmm23
vmovapd zmm14{k2},zmm20
vmovapd zmm15{k2},zmm21
;---------- Big: x4 transfers ----------;
mov eax,0F0h
vinsertf64x4 zmm24,zmm8,ymm12,1
vinsertf64x4 zmm25,zmm9,ymm13,1
vinsertf64x4 zmm26,zmm10,ymm14,1
vinsertf64x4 zmm27,zmm11,ymm15,1
kmovw k1,eax
vextractf64x4 ymm28,zmm8,1
vextractf64x4 ymm29,zmm9,1
vextractf64x4 ymm30,zmm10,1
vextractf64x4 ymm31,zmm11,1
vmovapd zmm28{k1},zmm12
vmovapd zmm29{k1},zmm13
vmovapd zmm30{k1},zmm14
vmovapd zmm31{k1},zmm15
;---------- Store destination data ----------;
vmovntpd zmmword ptr [rdx],zmm24
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm25
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm26
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm27
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm28
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm29
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm30
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm31

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

ret
transposeLittleToBigNT ENDP





;---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------;






;---------- RCX = Source pointer, RDX = Destination pointer ----------;
transposeTemporal PROC

;-
; mov r11,rsp
; and rsp,0FFFFFFFFFFFFFFF0h
; ;---------- Save non-volatile (by x64 calling convention) registers ----------;
; sub rsp,10*16                    ; 10 128-bit SSE registers XMM6-XMM15 must be
; vmovapd [rsp + 16*0],xmm6        ; non-volatile by convention, note ALIGN16
; vmovapd [rsp + 16*1],xmm7        ; TODO. Check code size and performance if use volatile ZMM16-ZMM31 without save XMM6-XMM15.
; vmovapd [rsp + 16*2],xmm8
; vmovapd [rsp + 16*3],xmm9
; vmovapd [rsp + 16*4],xmm10
; vmovapd [rsp + 16*5],xmm11
; vmovapd [rsp + 16*6],xmm12
; vmovapd [rsp + 16*7],xmm13
; vmovapd [rsp + 16*8],xmm14
; vmovapd [rsp + 16*9],xmm15
;-

;---------- (1) ---------- Load source data ----------;
vmovapd zmm16,[rcx]
add rcx,r8
vmovapd zmm17,[rcx]
add rcx,r8
vmovapd zmm18,[rcx]
add rcx,r8
vmovapd zmm19,[rcx]
add rcx,r8
vmovapd zmm20,[rcx]
add rcx,r8
vmovapd zmm21,[rcx]
add rcx,r8
vmovapd zmm22,[rcx]
add rcx,r8
vmovapd zmm23,[rcx]
;---------- (2) ---------- Big: x4 transfers ----------;
mov eax,0F0h
vinsertf64x4 zmm24,zmm16,ymm20,1
vinsertf64x4 zmm25,zmm17,ymm21,1
vinsertf64x4 zmm26,zmm18,ymm22,1
vinsertf64x4 zmm27,zmm19,ymm23,1
kmovw k1,eax
vextractf64x4 ymm28,zmm16,1
vextractf64x4 ymm29,zmm17,1
vextractf64x4 ymm30,zmm18,1
vextractf64x4 ymm31,zmm19,1
vmovapd zmm28{k1},zmm20
vmovapd zmm29{k1},zmm21
vmovapd zmm30{k1},zmm22
vmovapd zmm31{k1},zmm23
;---------- (3) ---------- Middle: x2 transfers ----------;
mov eax,11001100b
mov ecx,00110011b
valignq zmm16,zmm24,zmm24,2
valignq zmm17,zmm25,zmm25,2
valignq zmm18,zmm26,zmm26,6
valignq zmm19,zmm27,zmm27,6
kmovw k1,eax
kmovw k2,ecx
valignq zmm20,zmm28,zmm28,2
valignq zmm21,zmm29,zmm29,2
valignq zmm22,zmm30,zmm30,6
valignq zmm23,zmm31,zmm31,6
;---------- (4) ---------- Masked store odd and even pairs ----------;
vmovapd zmm24{k1},zmm18
vmovapd zmm25{k1},zmm19
vmovapd zmm26{k2},zmm16
vmovapd zmm27{k2},zmm17
vmovapd zmm28{k1},zmm22
vmovapd zmm29{k1},zmm23
vmovapd zmm30{k2},zmm20
vmovapd zmm31{k2},zmm21
;---------- (5) ---------- Little: x1 transfers ----------;
vunpcklpd zmm16,zmm24,zmm25
vunpckhpd zmm17,zmm24,zmm25
vunpcklpd zmm18,zmm26,zmm27
vunpckhpd zmm19,zmm26,zmm27
vunpcklpd zmm20,zmm28,zmm29
vunpckhpd zmm21,zmm28,zmm29
vunpcklpd zmm22,zmm30,zmm31
vunpckhpd zmm23,zmm30,zmm31
;---------- (6) ---------- Store destination data ----------;
vmovapd [rdx],zmm16
add rdx,r8
vmovapd [rdx],zmm17
add rdx,r8
vmovapd [rdx],zmm18
add rdx,r8
vmovapd [rdx],zmm19
add rdx,r8
vmovapd [rdx],zmm20
add rdx,r8
vmovapd [rdx],zmm21
add rdx,r8
vmovapd [rdx],zmm22
add rdx,r8
vmovapd [rdx],zmm23

;-
; ;---------- Restore non-volatile (by x64 calling convention) registers ----------;
; vmovapd xmm6,[rsp + 16*0]
; vmovapd xmm7,[rsp + 16*1]
; vmovapd xmm8,[rsp + 16*2]
; vmovapd xmm9,[rsp + 16*3]
; vmovapd xmm10,[rsp + 16*4]
; vmovapd xmm11,[rsp + 16*5]
; vmovapd xmm12,[rsp + 16*6]
; vmovapd xmm13,[rsp + 16*7]
; vmovapd xmm14,[rsp + 16*8]     ; 10 128-bit SSE registers XMM6-XMM15 must be
; vmovapd xmm15,[rsp + 16*9]     ; non-volatile by convention, note ALIGN16
; mov rsp,r11
;-

ret
transposeTemporal ENDP




;---------- RCX = Source pointer, RDX = Destination pointer ----------;
transposeNonTemporal PROC

;-
; mov r11,rsp
; and rsp,0FFFFFFFFFFFFFFF0h
; ;---------- Save non-volatile (by x64 calling convention) registers ----------;
; sub rsp,10*16                    ; 10 128-bit SSE registers XMM6-XMM15 must be
; vmovapd [rsp + 16*0],xmm6        ; non-volatile by convention, note ALIGN16
; vmovapd [rsp + 16*1],xmm7        ; TODO. Check code size and performance if use volatile ZMM16-ZMM31 without save XMM6-XMM15.
; vmovapd [rsp + 16*2],xmm8
; vmovapd [rsp + 16*3],xmm9
; vmovapd [rsp + 16*4],xmm10
; vmovapd [rsp + 16*5],xmm11
; vmovapd [rsp + 16*6],xmm12
; vmovapd [rsp + 16*7],xmm13
; vmovapd [rsp + 16*8],xmm14
; vmovapd [rsp + 16*9],xmm15
;-

;---------- (1) ---------- Load source data ----------;
vmovapd zmm16,[rcx]
add rcx,r8
vmovapd zmm17,[rcx]
add rcx,r8
vmovapd zmm18,[rcx]
add rcx,r8
vmovapd zmm19,[rcx]
add rcx,r8
vmovapd zmm20,[rcx]
add rcx,r8
vmovapd zmm21,[rcx]
add rcx,r8
vmovapd zmm22,[rcx]
add rcx,r8
vmovapd zmm23,[rcx]
;---------- (2) ---------- Big: x4 transfers ----------;
mov eax,0F0h
vinsertf64x4 zmm24,zmm16,ymm20,1
vinsertf64x4 zmm25,zmm17,ymm21,1
vinsertf64x4 zmm26,zmm18,ymm22,1
vinsertf64x4 zmm27,zmm19,ymm23,1
kmovw k1,eax
vextractf64x4 ymm28,zmm16,1
vextractf64x4 ymm29,zmm17,1
vextractf64x4 ymm30,zmm18,1
vextractf64x4 ymm31,zmm19,1
vmovapd zmm28{k1},zmm20
vmovapd zmm29{k1},zmm21
vmovapd zmm30{k1},zmm22
vmovapd zmm31{k1},zmm23
;---------- (3) ---------- Middle: x2 transfers ----------;
mov eax,11001100b
mov ecx,00110011b
valignq zmm16,zmm24,zmm24,2
valignq zmm17,zmm25,zmm25,2
valignq zmm18,zmm26,zmm26,6
valignq zmm19,zmm27,zmm27,6
kmovw k1,eax
kmovw k2,ecx
valignq zmm20,zmm28,zmm28,2
valignq zmm21,zmm29,zmm29,2
valignq zmm22,zmm30,zmm30,6
valignq zmm23,zmm31,zmm31,6
;---------- (4) ---------- Masked store odd and even pairs ----------;
vmovapd zmm24{k1},zmm18
vmovapd zmm25{k1},zmm19
vmovapd zmm26{k2},zmm16
vmovapd zmm27{k2},zmm17
vmovapd zmm28{k1},zmm22
vmovapd zmm29{k1},zmm23
vmovapd zmm30{k2},zmm20
vmovapd zmm31{k2},zmm21
;---------- (5) ---------- Little: x1 transfers ----------;
vunpcklpd zmm16,zmm24,zmm25
vunpckhpd zmm17,zmm24,zmm25
vunpcklpd zmm18,zmm26,zmm27
vunpckhpd zmm19,zmm26,zmm27
vunpcklpd zmm20,zmm28,zmm29
vunpckhpd zmm21,zmm28,zmm29
vunpcklpd zmm22,zmm30,zmm31
vunpckhpd zmm23,zmm30,zmm31
;---------- (6) ---------- Store destination data ----------;
vmovapd [rdx],zmm16
add rdx,r8
vmovapd [rdx],zmm17
add rdx,r8
vmovapd [rdx],zmm18
add rdx,r8
vmovapd [rdx],zmm19
add rdx,r8
vmovapd [rdx],zmm20
add rdx,r8
vmovapd [rdx],zmm21
add rdx,r8
vmovapd [rdx],zmm22
add rdx,r8
vmovapd [rdx],zmm23

;-
; ;---------- Restore non-volatile (by x64 calling convention) registers ----------;
; vmovapd xmm6,[rsp + 16*0]
; vmovapd xmm7,[rsp + 16*1]
; vmovapd xmm8,[rsp + 16*2]
; vmovapd xmm9,[rsp + 16*3]
; vmovapd xmm10,[rsp + 16*4]
; vmovapd xmm11,[rsp + 16*5]
; vmovapd xmm12,[rsp + 16*6]
; vmovapd xmm13,[rsp + 16*7]
; vmovapd xmm14,[rsp + 16*8]     ; 10 128-bit SSE registers XMM6-XMM15 must be
; vmovapd xmm15,[rsp + 16*9]     ; non-volatile by convention, note ALIGN16
; mov rsp,r11
;-

ret
transposeNonTemporal ENDP





;---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------;






 ;-----------------------------------;
 ; RCX        = source pointer       ;
 ; RDX        = destination pointer  ;
 ; R8         = bytesPerRow          ;
 ; R9         = srcAdd               ;
 ; [rsp + 40] = dstAdd               ;
 ; [rsp + 48] = count                ;
 ;-----------------------------------;


transposeCycledTemporal PROC
;---------- (1) ---------- Initializing context not changed in the cycle ----------;
mov eax,11110000b
mov r10,rcx
kmovw k1,eax
mov eax,11001100b
mov r11,rdx
kmovw k2,eax
mov eax,00110011b
kmovw k3,eax
;---------- (2) ---------- Start cycle ----------;
mov rax,[rsp + 48]
matrixCycle:
mov rcx,r10
mov rdx,r11
;---------- (3) ---------- Load source data ----------;
vmovapd zmm16,[rcx]
add rcx,r8
vmovapd zmm17,[rcx]
add rcx,r8
vmovapd zmm18,[rcx]
add rcx,r8
vmovapd zmm19,[rcx]
add rcx,r8
vmovapd zmm20,[rcx]
add rcx,r8
vmovapd zmm21,[rcx]
add rcx,r8
vmovapd zmm22,[rcx]
add rcx,r8
vmovapd zmm23,[rcx]
;---------- (4) ---------- Big: x4 transfers ----------;
vinsertf64x4 zmm24,zmm16,ymm20,1
vinsertf64x4 zmm25,zmm17,ymm21,1
vinsertf64x4 zmm26,zmm18,ymm22,1
vinsertf64x4 zmm27,zmm19,ymm23,1
vextractf64x4 ymm28,zmm16,1
vextractf64x4 ymm29,zmm17,1
vextractf64x4 ymm30,zmm18,1
vextractf64x4 ymm31,zmm19,1
vmovapd zmm28{k1},zmm20
vmovapd zmm29{k1},zmm21
vmovapd zmm30{k1},zmm22
vmovapd zmm31{k1},zmm23
;---------- (5) ---------- Middle: x2 transfers ----------;
valignq zmm16,zmm24,zmm24,2
valignq zmm17,zmm25,zmm25,2
valignq zmm18,zmm26,zmm26,6
valignq zmm19,zmm27,zmm27,6
valignq zmm20,zmm28,zmm28,2
valignq zmm21,zmm29,zmm29,2
valignq zmm22,zmm30,zmm30,6
valignq zmm23,zmm31,zmm31,6
vmovapd zmm24{k2},zmm18
vmovapd zmm25{k2},zmm19
vmovapd zmm26{k3},zmm16
vmovapd zmm27{k3},zmm17
vmovapd zmm28{k2},zmm22
vmovapd zmm29{k2},zmm23
vmovapd zmm30{k3},zmm20
vmovapd zmm31{k3},zmm21
;---------- (6) ---------- Little: x1 transfers ----------;
vunpcklpd zmm16,zmm24,zmm25
vunpckhpd zmm17,zmm24,zmm25
vunpcklpd zmm18,zmm26,zmm27
vunpckhpd zmm19,zmm26,zmm27
vunpcklpd zmm20,zmm28,zmm29
vunpckhpd zmm21,zmm28,zmm29
vunpcklpd zmm22,zmm30,zmm31
vunpckhpd zmm23,zmm30,zmm31
;---------- (7) ---------- Store destination data ----------;
vmovapd [rdx],zmm16
add rdx,r8
vmovapd [rdx],zmm17
add rdx,r8
vmovapd [rdx],zmm18
add rdx,r8
vmovapd [rdx],zmm19
add rdx,r8
vmovapd [rdx],zmm20
add rdx,r8
vmovapd [rdx],zmm21
add rdx,r8
vmovapd [rdx],zmm22
add rdx,r8
vmovapd [rdx],zmm23
;---------- (8) ---------- Make cycle ----------;
add r10,r9
add r11,[rsp + 40]
dec rax
jnz matrixCycle
;---------- (9) ---------- Done ----------;
ret
transposeCycledTemporal ENDP



transposeCycledNonTemporal PROC
;---------- (1) ---------- Initializing context not changed in the cycle ----------;
mov eax,11110000b
mov r10,rcx
kmovw k1,eax
mov eax,11001100b
mov r11,rdx
kmovw k2,eax
mov eax,00110011b
kmovw k3,eax
;---------- (2) ---------- Start cycle ----------;
mov rax,[rsp + 48]
matrixCycle:
mov rcx,r10
mov rdx,r11
;---------- (3) ---------- Load source data ----------;
vmovapd zmm16,[rcx]
add rcx,r8
vmovapd zmm17,[rcx]
add rcx,r8
vmovapd zmm18,[rcx]
add rcx,r8
vmovapd zmm19,[rcx]
add rcx,r8
vmovapd zmm20,[rcx]
add rcx,r8
vmovapd zmm21,[rcx]
add rcx,r8
vmovapd zmm22,[rcx]
add rcx,r8
vmovapd zmm23,[rcx]
;---------- (4) ---------- Big: x4 transfers ----------;
vinsertf64x4 zmm24,zmm16,ymm20,1
vinsertf64x4 zmm25,zmm17,ymm21,1
vinsertf64x4 zmm26,zmm18,ymm22,1
vinsertf64x4 zmm27,zmm19,ymm23,1
vextractf64x4 ymm28,zmm16,1
vextractf64x4 ymm29,zmm17,1
vextractf64x4 ymm30,zmm18,1
vextractf64x4 ymm31,zmm19,1
vmovapd zmm28{k1},zmm20
vmovapd zmm29{k1},zmm21
vmovapd zmm30{k1},zmm22
vmovapd zmm31{k1},zmm23
;---------- (5) ---------- Middle: x2 transfers ----------;
valignq zmm16,zmm24,zmm24,2
valignq zmm17,zmm25,zmm25,2
valignq zmm18,zmm26,zmm26,6
valignq zmm19,zmm27,zmm27,6
valignq zmm20,zmm28,zmm28,2
valignq zmm21,zmm29,zmm29,2
valignq zmm22,zmm30,zmm30,6
valignq zmm23,zmm31,zmm31,6
vmovapd zmm24{k2},zmm18
vmovapd zmm25{k2},zmm19
vmovapd zmm26{k3},zmm16
vmovapd zmm27{k3},zmm17
vmovapd zmm28{k2},zmm22
vmovapd zmm29{k2},zmm23
vmovapd zmm30{k3},zmm20
vmovapd zmm31{k3},zmm21
;---------- (6) ---------- Little: x1 transfers ----------;
vunpcklpd zmm16,zmm24,zmm25
vunpckhpd zmm17,zmm24,zmm25
vunpcklpd zmm18,zmm26,zmm27
vunpckhpd zmm19,zmm26,zmm27
vunpcklpd zmm20,zmm28,zmm29
vunpckhpd zmm21,zmm28,zmm29
vunpcklpd zmm22,zmm30,zmm31
vunpckhpd zmm23,zmm30,zmm31
;---------- (7) ---------- Store destination data ----------;
vmovntpd zmmword ptr [rdx],zmm16
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm17
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm18
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm19
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm20
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm21
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm22
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm23
;---------- (8) ---------- Make cycle ----------;
add r10,r9
add r11,[rsp + 40]
dec rax
jnz matrixCycle
;---------- (9) ---------- Done ----------;
ret
transposeCycledNonTemporal ENDP



 ;-----------------------------------------;
 ; RCX        = source pointer             ;
 ; RDX        = destination pointer        ;
 ; R8         = pointer to offsets map     ;
 ; R9         = count in 128-numbers units ;
 ; [rsp + 40] = srcAdd                     ;
 ; [rsp + 48] = dstAdd                     ;
 ;-----------------------------------------;

transposeGather PROC

push rbp
mov eax,0FFFFh
mov rbp,rsp
kmovw k0,eax                     ; K0 = storage for predicate mask
mov r10,[rsp + 40 + 8]
mov r11,[rsp + 48 + 8]
and rsp,0FFFFFFFFFFFFFFF0h

;---------- Save non-volatile (by x64 calling convention) registers ----------;
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

;---------- (1) ---------- Offsets map initialization ----------;
vmovapd zmm16,[r8 + 64*0]
vmovapd zmm17,[r8 + 64*1]
vmovapd zmm18,[r8 + 64*2]
vmovapd zmm19,[r8 + 64*3]
vmovapd zmm20,[r8 + 64*4]
vmovapd zmm21,[r8 + 64*5]
vmovapd zmm22,[r8 + 64*6]
vmovapd zmm23,[r8 + 64*7]
vmovapd zmm24,[r8 + 64*8]
vmovapd zmm25,[r8 + 64*9]
vmovapd zmm26,[r8 + 64*10]
vmovapd zmm27,[r8 + 64*11]
vmovapd zmm28,[r8 + 64*12]
vmovapd zmm29,[r8 + 64*13]
vmovapd zmm30,[r8 + 64*14]
vmovapd zmm31,[r8 + 64*15]

;---------- (2) ---------- Start cycle ----------;
lineCycle:

;---------- (3) ---------- Load 128 numbers from source matrix ----------;
kmovw k1,k0    ; Mask must be reloaded because gather instruction clear it
kmovw k2,k0
kmovw k3,k0
kmovw k4,k0
kmovw k5,k0
kmovw k6,k0
kmovw k7,k0
vgatherqpd zmm0{k1},[rcx + zmm16]  
vgatherqpd zmm1{k2},[rcx + zmm17]
vgatherqpd zmm2{k3},[rcx + zmm18]  
vgatherqpd zmm3{k4},[rcx + zmm19]
vgatherqpd zmm4{k5},[rcx + zmm20]  
vgatherqpd zmm5{k6},[rcx + zmm21]
vgatherqpd zmm6{k7},[rcx + zmm22]  
kmovw k1,k0
kmovw k2,k0
kmovw k3,k0
kmovw k4,k0
kmovw k5,k0
kmovw k6,k0
kmovw k7,k0
vgatherqpd zmm7{k1},[rcx + zmm23]
vgatherqpd zmm8{k2},[rcx + zmm24]
vgatherqpd zmm9{k3},[rcx + zmm25]
vgatherqpd zmm10{k4},[rcx + zmm26]
vgatherqpd zmm11{k5},[rcx + zmm27]
vgatherqpd zmm12{k6},[rcx + zmm28]
vgatherqpd zmm13{k7},[rcx + zmm29]
kmovw k1,k0
kmovw k2,k0
vgatherqpd zmm14{k1},[rcx + zmm30]
vgatherqpd zmm15{k2},[rcx + zmm31]

;---------- (4) ---------- Store 128 numbers to destination matrix ----------;
vmovntpd zmmword ptr [rdx + 64*0],zmm0
vmovntpd zmmword ptr [rdx + 64*1],zmm1
vmovntpd zmmword ptr [rdx + 64*2],zmm2
vmovntpd zmmword ptr [rdx + 64*3],zmm3
vmovntpd zmmword ptr [rdx + 64*4],zmm4
vmovntpd zmmword ptr [rdx + 64*5],zmm5
vmovntpd zmmword ptr [rdx + 64*6],zmm6
vmovntpd zmmword ptr [rdx + 64*7],zmm7
vmovntpd zmmword ptr [rdx + 64*8],zmm8
vmovntpd zmmword ptr [rdx + 64*9],zmm9
vmovntpd zmmword ptr [rdx + 64*10],zmm10
vmovntpd zmmword ptr [rdx + 64*11],zmm11
vmovntpd zmmword ptr [rdx + 64*12],zmm12
vmovntpd zmmword ptr [rdx + 64*13],zmm13
vmovntpd zmmword ptr [rdx + 64*14],zmm14
vmovntpd zmmword ptr [rdx + 64*15],zmm15

;---------- (5) ---------- Make cycle ----------;
add rcx,r10
add rdx,r11
dec r9
jnz lineCycle

;---------- Restore non-volatile (by x64 calling convention) registers ----------;
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
mov rsp,rbp
pop rbp
ret

transposeGather ENDP



 ;-----------------------------------------;
 ; RCX        = source pointer             ;
 ; RDX        = destination pointer        ;
 ; R8         = pointer to offsets map     ;
 ; R9         = count in 128-numbers units ;
 ; [rsp + 40] = srcAdd                     ;
 ; [rsp + 48] = dstAdd                     ;
 ;-----------------------------------------;

transposeScatter PROC

push rbp
mov eax,0FFFFh
mov rbp,rsp
kmovw k0,eax                     ; K0 = storage for predicate mask
mov r10,[rsp + 40 + 8]
mov r11,[rsp + 48 + 8]
and rsp,0FFFFFFFFFFFFFFF0h

;---------- Save non-volatile (by x64 calling convention) registers ----------;
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

;---------- (1) ---------- Offsets map initialization ----------;
vmovapd zmm16,[r8 + 64*0]
vmovapd zmm17,[r8 + 64*1]
vmovapd zmm18,[r8 + 64*2]
vmovapd zmm19,[r8 + 64*3]
vmovapd zmm20,[r8 + 64*4]
vmovapd zmm21,[r8 + 64*5]
vmovapd zmm22,[r8 + 64*6]
vmovapd zmm23,[r8 + 64*7]
vmovapd zmm24,[r8 + 64*8]
vmovapd zmm25,[r8 + 64*9]
vmovapd zmm26,[r8 + 64*10]
vmovapd zmm27,[r8 + 64*11]
vmovapd zmm28,[r8 + 64*12]
vmovapd zmm29,[r8 + 64*13]
vmovapd zmm30,[r8 + 64*14]
vmovapd zmm31,[r8 + 64*15]

;---------- (2) ---------- Start cycle ----------;
lineCycle:

;---------- (3) ---------- Load 128 numbers from source matrix ----------;
vmovapd zmm0,[rcx + 64*0]
vmovapd zmm1,[rcx + 64*1]
vmovapd zmm2,[rcx + 64*2]
vmovapd zmm3,[rcx + 64*3]
vmovapd zmm4,[rcx + 64*4]
vmovapd zmm5,[rcx + 64*5]
vmovapd zmm6,[rcx + 64*6]
vmovapd zmm7,[rcx + 64*7]
vmovapd zmm8,[rcx + 64*8]
vmovapd zmm9,[rcx + 64*9]
vmovapd zmm10,[rcx + 64*10]
vmovapd zmm11,[rcx + 64*11]
vmovapd zmm12,[rcx + 64*12]
vmovapd zmm13,[rcx + 64*13]
vmovapd zmm14,[rcx + 64*14]
vmovapd zmm15,[rcx + 64*15]

;---------- (4) ---------- Store 128 numbers to destination matrix ----------;
kmovw k1,k0    ; Mask must be reloaded because gather instruction clear it
kmovw k2,k0
kmovw k3,k0
kmovw k4,k0
kmovw k5,k0
kmovw k6,k0
kmovw k7,k0
vscatterqpd [rdx + zmm16]{k1},zmm0
vscatterqpd [rdx + zmm17]{k2},zmm1
vscatterqpd [rdx + zmm18]{k3},zmm2
vscatterqpd [rdx + zmm19]{k4},zmm3
vscatterqpd [rdx + zmm20]{k5},zmm4
vscatterqpd [rdx + zmm21]{k6},zmm5
vscatterqpd [rdx + zmm22]{k7},zmm6
kmovw k1,k0
kmovw k2,k0
kmovw k3,k0
kmovw k4,k0
kmovw k5,k0
kmovw k6,k0
kmovw k7,k0
vscatterqpd [rdx + zmm23]{k1},zmm7
vscatterqpd [rdx + zmm24]{k2},zmm8
vscatterqpd [rdx + zmm25]{k3},zmm9
vscatterqpd [rdx + zmm26]{k4},zmm10
vscatterqpd [rdx + zmm27]{k5},zmm11
vscatterqpd [rdx + zmm28]{k6},zmm12
vscatterqpd [rdx + zmm29]{k7},zmm13
kmovw k1,k0
kmovw k2,k0
vscatterqpd [rdx + zmm30]{k1},zmm14
vscatterqpd [rdx + zmm31]{k2},zmm15

;---------- (5) ---------- Make cycle ----------;
add rcx,r11
add rdx,r10
dec r9
jnz lineCycle

;---------- Restore non-volatile (by x64 calling convention) registers ----------;
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
mov rsp,rbp
pop rbp
ret

transposeScatter ENDP




;---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------;




;-----------------------------------------;
; RCX        = source pointer             ;
; RDX        = destination pointer        ;
; R8         = columns count              ;
; R9         = rows count                 ;
;-----------------------------------------;

transposeAVX512 PROC
;---------- (1) ---------- Save non-volatile (by x64 calling convention) registers ----------;
;---------- (2) ---------- Initializing context not changed in the cycle ----------;
push rbp
mov eax,11110000b
mov rbp,rsp
kmovw k1,eax
and rsp,0FFFFFFFFFFFFFFF0h
mov eax,11001100b
sub rsp,10*16                    ; 10 128-bit SSE registers XMM6-XMM15 must be non-volatile by convention, note ALIGN16
kmovw k2,eax
mov r10,rcx
mov r11,rdx
mov eax,00110011b
shr r9,3                         ; Because 8 scalars per vector register. Yet only quadrat matrices supported.
kmovw k3,eax
vmovapd [rsp + 16*0],xmm6
vmovapd [rsp + 16*1],xmm7
vmovapd [rsp + 16*2],xmm8
vmovapd [rsp + 16*3],xmm9
vmovapd [rsp + 16*4],xmm10
vmovapd [rsp + 16*5],xmm11
vmovapd [rsp + 16*6],xmm12
vmovapd [rsp + 16*7],xmm13
vmovapd [rsp + 16*8],xmm14
vmovapd [rsp + 16*9],xmm15
mov rax,r8
shl r8,3
shl rax,6
;---------- (3) ---------- Start cycle ----------;
lineCycle:
mov rcx,r10
mov rdx,r11
;---------- (4) ---------- Load source data ----------;
vmovapd zmm0,[rcx]
add rcx,r8
vmovapd zmm1,[rcx]
add rcx,r8
vmovapd zmm2,[rcx]
add rcx,r8
vmovapd zmm3,[rcx]
add rcx,r8
vmovapd zmm4,[rcx]
add rcx,r8
vmovapd zmm5,[rcx]
add rcx,r8
vmovapd zmm6,[rcx]
add rcx,r8
vmovapd zmm7,[rcx]
add rcx,r8
vmovapd zmm16,[rcx]
add rcx,r8
vmovapd zmm17,[rcx]
add rcx,r8
vmovapd zmm18,[rcx]
add rcx,r8
vmovapd zmm19,[rcx]
add rcx,r8
vmovapd zmm20,[rcx]
add rcx,r8
vmovapd zmm21,[rcx]
add rcx,r8
vmovapd zmm22,[rcx]
add rcx,r8
vmovapd zmm23,[rcx]
add rcx,r8
;---------- (5) ---------- Big: x4 transfers ----------;
vinsertf64x4 zmm8,zmm0,ymm4,1
vinsertf64x4 zmm24,zmm16,ymm20,1
vinsertf64x4 zmm9,zmm1,ymm5,1
vinsertf64x4 zmm25,zmm17,ymm21,1
vinsertf64x4 zmm10,zmm2,ymm6,1
vinsertf64x4 zmm26,zmm18,ymm22,1
vinsertf64x4 zmm11,zmm3,ymm7,1
vinsertf64x4 zmm27,zmm19,ymm23,1
vextractf64x4 ymm12,zmm0,1
vextractf64x4 ymm28,zmm16,1
vextractf64x4 ymm13,zmm1,1
vextractf64x4 ymm29,zmm17,1
vextractf64x4 ymm14,zmm2,1
vextractf64x4 ymm30,zmm18,1
vextractf64x4 ymm15,zmm3,1
vextractf64x4 ymm31,zmm19,1
vmovapd zmm12{k1},zmm4
vmovapd zmm28{k1},zmm20
vmovapd zmm13{k1},zmm5
vmovapd zmm29{k1},zmm21
vmovapd zmm14{k1},zmm6
vmovapd zmm30{k1},zmm22
vmovapd zmm15{k1},zmm7
vmovapd zmm31{k1},zmm23
;---------- (6) ---------- Middle: x2 transfers ----------;
valignq zmm0,zmm8,zmm8,2
valignq zmm16,zmm24,zmm24,2
valignq zmm1,zmm9,zmm9,2
valignq zmm17,zmm25,zmm25,2
valignq zmm2,zmm10,zmm10,6
valignq zmm18,zmm26,zmm26,6
valignq zmm3,zmm11,zmm11,6
valignq zmm19,zmm27,zmm27,6
valignq zmm4,zmm12,zmm12,2
valignq zmm20,zmm28,zmm28,2
valignq zmm5,zmm13,zmm13,2
valignq zmm21,zmm29,zmm29,2
valignq zmm6,zmm14,zmm14,6
valignq zmm22,zmm30,zmm30,6
valignq zmm7,zmm15,zmm15,6
valignq zmm23,zmm31,zmm31,6
vmovapd zmm8{k2},zmm2
vmovapd zmm24{k2},zmm18
vmovapd zmm9{k2},zmm3
vmovapd zmm25{k2},zmm19
vmovapd zmm10{k3},zmm0
vmovapd zmm26{k3},zmm16
vmovapd zmm11{k3},zmm1
vmovapd zmm27{k3},zmm17
vmovapd zmm12{k2},zmm6
vmovapd zmm28{k2},zmm22
vmovapd zmm13{k2},zmm7
vmovapd zmm29{k2},zmm23
vmovapd zmm14{k3},zmm4
vmovapd zmm30{k3},zmm20
vmovapd zmm15{k3},zmm5
vmovapd zmm31{k3},zmm21
;---------- (7) ---------- Little: x1 transfers ----------;
vunpcklpd zmm0,zmm8,zmm9
vunpcklpd zmm16,zmm24,zmm25
vunpckhpd zmm1,zmm8,zmm9
vunpckhpd zmm17,zmm24,zmm25
vunpcklpd zmm2,zmm10,zmm11
vunpcklpd zmm18,zmm26,zmm27
vunpckhpd zmm3,zmm10,zmm11
vunpckhpd zmm19,zmm26,zmm27
vunpcklpd zmm4,zmm12,zmm13
vunpcklpd zmm20,zmm28,zmm29
vunpckhpd zmm5,zmm12,zmm13
vunpckhpd zmm21,zmm28,zmm29
vunpcklpd zmm6,zmm14,zmm15
vunpcklpd zmm22,zmm30,zmm31
vunpckhpd zmm7,zmm14,zmm15
vunpckhpd zmm23,zmm30,zmm31
;---------- (8) ---------- Store destination data ----------;
vmovapd [rdx],zmm0
vmovapd [rdx + 64],zmm16
add rdx,r8
vmovapd [rdx],zmm1
vmovapd [rdx + 64],zmm17
add rdx,r8
vmovapd [rdx],zmm2
vmovapd [rdx + 64],zmm18
add rdx,r8
vmovapd [rdx],zmm3
vmovapd [rdx + 64],zmm19
add rdx,r8
vmovapd [rdx],zmm4
vmovapd [rdx + 64],zmm20
add rdx,r8
vmovapd [rdx],zmm5
vmovapd [rdx + 64],zmm21
add rdx,r8
vmovapd [rdx],zmm6
vmovapd [rdx + 64],zmm22
add rdx,r8
vmovapd [rdx],zmm7
vmovapd [rdx + 64],zmm23
add rdx,r8
;---------- (9) ---------- Make cycle ----------;
add r10,8*8
add r11,rax
dec r9
jnz lineCycle
;---------- (10) ---------- Restore non-volatile (by x64 calling convention) registers ----------;
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
mov rsp,rbp
pop rbp
ret

transposeAVX512 ENDP




transposeAVX512NT PROC
;---------- (1) ---------- Save non-volatile (by x64 calling convention) registers ----------;
;---------- (2) ---------- Initializing context not changed in the cycle ----------;
push rbp
mov eax,11110000b
mov rbp,rsp
kmovw k1,eax
and rsp,0FFFFFFFFFFFFFFF0h
mov eax,11001100b
sub rsp,10*16                    ; 10 128-bit SSE registers XMM6-XMM15 must be non-volatile by convention, note ALIGN16
kmovw k2,eax
mov r10,rcx
mov r11,rdx
mov eax,00110011b
shr r9,3                         ; Because 8 scalars per vector register. Yet only quadrat matrices supported.
kmovw k3,eax
vmovapd [rsp + 16*0],xmm6
vmovapd [rsp + 16*1],xmm7
vmovapd [rsp + 16*2],xmm8
vmovapd [rsp + 16*3],xmm9
vmovapd [rsp + 16*4],xmm10
vmovapd [rsp + 16*5],xmm11
vmovapd [rsp + 16*6],xmm12
vmovapd [rsp + 16*7],xmm13
vmovapd [rsp + 16*8],xmm14
vmovapd [rsp + 16*9],xmm15
mov rax,r8
shl r8,3
shl rax,6
;---------- (3) ---------- Start cycle ----------;
lineCycle:
mov rcx,r10
mov rdx,r11
;---------- (4) ---------- Load source data ----------;
vmovapd zmm0,[rcx]
add rcx,r8
vmovapd zmm1,[rcx]
add rcx,r8
vmovapd zmm2,[rcx]
add rcx,r8
vmovapd zmm3,[rcx]
add rcx,r8
vmovapd zmm4,[rcx]
add rcx,r8
vmovapd zmm5,[rcx]
add rcx,r8
vmovapd zmm6,[rcx]
add rcx,r8
vmovapd zmm7,[rcx]
add rcx,r8
vmovapd zmm16,[rcx]
add rcx,r8
vmovapd zmm17,[rcx]
add rcx,r8
vmovapd zmm18,[rcx]
add rcx,r8
vmovapd zmm19,[rcx]
add rcx,r8
vmovapd zmm20,[rcx]
add rcx,r8
vmovapd zmm21,[rcx]
add rcx,r8
vmovapd zmm22,[rcx]
add rcx,r8
vmovapd zmm23,[rcx]
add rcx,r8
;---------- (5) ---------- Big: x4 transfers ----------;
vinsertf64x4 zmm8,zmm0,ymm4,1
vinsertf64x4 zmm24,zmm16,ymm20,1
vinsertf64x4 zmm9,zmm1,ymm5,1
vinsertf64x4 zmm25,zmm17,ymm21,1
vinsertf64x4 zmm10,zmm2,ymm6,1
vinsertf64x4 zmm26,zmm18,ymm22,1
vinsertf64x4 zmm11,zmm3,ymm7,1
vinsertf64x4 zmm27,zmm19,ymm23,1
vextractf64x4 ymm12,zmm0,1
vextractf64x4 ymm28,zmm16,1
vextractf64x4 ymm13,zmm1,1
vextractf64x4 ymm29,zmm17,1
vextractf64x4 ymm14,zmm2,1
vextractf64x4 ymm30,zmm18,1
vextractf64x4 ymm15,zmm3,1
vextractf64x4 ymm31,zmm19,1
vmovapd zmm12{k1},zmm4
vmovapd zmm28{k1},zmm20
vmovapd zmm13{k1},zmm5
vmovapd zmm29{k1},zmm21
vmovapd zmm14{k1},zmm6
vmovapd zmm30{k1},zmm22
vmovapd zmm15{k1},zmm7
vmovapd zmm31{k1},zmm23
;---------- (6) ---------- Middle: x2 transfers ----------;
valignq zmm0,zmm8,zmm8,2
valignq zmm16,zmm24,zmm24,2
valignq zmm1,zmm9,zmm9,2
valignq zmm17,zmm25,zmm25,2
valignq zmm2,zmm10,zmm10,6
valignq zmm18,zmm26,zmm26,6
valignq zmm3,zmm11,zmm11,6
valignq zmm19,zmm27,zmm27,6
valignq zmm4,zmm12,zmm12,2
valignq zmm20,zmm28,zmm28,2
valignq zmm5,zmm13,zmm13,2
valignq zmm21,zmm29,zmm29,2
valignq zmm6,zmm14,zmm14,6
valignq zmm22,zmm30,zmm30,6
valignq zmm7,zmm15,zmm15,6
valignq zmm23,zmm31,zmm31,6
vmovapd zmm8{k2},zmm2
vmovapd zmm24{k2},zmm18
vmovapd zmm9{k2},zmm3
vmovapd zmm25{k2},zmm19
vmovapd zmm10{k3},zmm0
vmovapd zmm26{k3},zmm16
vmovapd zmm11{k3},zmm1
vmovapd zmm27{k3},zmm17
vmovapd zmm12{k2},zmm6
vmovapd zmm28{k2},zmm22
vmovapd zmm13{k2},zmm7
vmovapd zmm29{k2},zmm23
vmovapd zmm14{k3},zmm4
vmovapd zmm30{k3},zmm20
vmovapd zmm15{k3},zmm5
vmovapd zmm31{k3},zmm21
;---------- (7) ---------- Little: x1 transfers ----------;
vunpcklpd zmm0,zmm8,zmm9
vunpcklpd zmm16,zmm24,zmm25
vunpckhpd zmm1,zmm8,zmm9
vunpckhpd zmm17,zmm24,zmm25
vunpcklpd zmm2,zmm10,zmm11
vunpcklpd zmm18,zmm26,zmm27
vunpckhpd zmm3,zmm10,zmm11
vunpckhpd zmm19,zmm26,zmm27
vunpcklpd zmm4,zmm12,zmm13
vunpcklpd zmm20,zmm28,zmm29
vunpckhpd zmm5,zmm12,zmm13
vunpckhpd zmm21,zmm28,zmm29
vunpcklpd zmm6,zmm14,zmm15
vunpcklpd zmm22,zmm30,zmm31
vunpckhpd zmm7,zmm14,zmm15
vunpckhpd zmm23,zmm30,zmm31
;---------- (8) ---------- Store destination data ----------;
vmovntpd zmmword ptr [rdx],zmm0
vmovntpd zmmword ptr [rdx + 64],zmm16
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm1
vmovntpd zmmword ptr [rdx + 64],zmm17
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm2
vmovntpd zmmword ptr [rdx + 64],zmm18
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm3
vmovntpd zmmword ptr [rdx + 64],zmm19
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm4
vmovntpd zmmword ptr [rdx + 64],zmm20
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm5
vmovntpd zmmword ptr [rdx + 64],zmm21
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm6
vmovntpd zmmword ptr [rdx + 64],zmm22
add rdx,r8
vmovntpd zmmword ptr [rdx],zmm7
vmovntpd zmmword ptr [rdx + 64],zmm23
add rdx,r8
;---------- (9) ---------- Make cycle ----------;
add r10,8*8
add r11,rax
dec r9
jnz lineCycle
;---------- (10) ---------- Restore non-volatile (by x64 calling convention) registers ----------;
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
mov rsp,rbp
pop rbp
ret

transposeAVX512NT ENDP






transposeAVX256 PROC
;---------- (1) ---------- Save non-volatile (by x64 calling convention) registers ----------;
;---------- (2) ---------- Initializing context not changed in the cycle ----------;
push rbp
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
mov rax,r8
shl r8,3
shl rax,5
shr r9,2                         ; Because 4 scalars per vector register. Yet only quadrat matrices supported.
sub rsp,10*16                    ; 10 128-bit SSE registers XMM6-XMM15 must be non-volatile by convention, note ALIGN16
mov r10,rcx
mov r11,rdx
vmovapd [rsp + 16*0],xmm6
vmovapd [rsp + 16*1],xmm7
vmovapd [rsp + 16*2],xmm8
vmovapd [rsp + 16*3],xmm9
vmovapd [rsp + 16*4],xmm10
vmovapd [rsp + 16*5],xmm11
vmovapd [rsp + 16*6],xmm12
vmovapd [rsp + 16*7],xmm13
vmovapd [rsp + 16*8],xmm14
vmovapd [rsp + 16*9],xmm15
;---------- (3) ---------- Start cycle ----------;
lineCycle:
mov rcx,r10
mov rdx,r11
;---------- (4) ---------- Load source data ----------;
vmovapd ymm0,[rcx]                       ;  1   2   3   4
add rcx,r8
vmovapd ymm1,[rcx]                       ;  5   6   7   8
add rcx,r8
vmovapd ymm2,[rcx]                       ;  9  10  11  12
add rcx,r8
vmovapd ymm3,[rcx]                       ; 13  14  15  16
add rcx,r8
vmovapd ymm4,[rcx]                       ;  1   2   3   4
add rcx,r8
vmovapd ymm5,[rcx]                       ;  5   6   7   8
add rcx,r8
vmovapd ymm6,[rcx]                       ;  9  10  11  12
add rcx,r8
vmovapd ymm7,[rcx]                       ; 13  14  15  16
add rcx,r8
;---------- (5) ---------- Unpack ----------;
vunpcklpd ymm8,ymm0,ymm1                 ;  1   5   3   7
vunpckhpd ymm9,ymm0,ymm1                 ;  2   6   4   8
vunpcklpd ymm10,ymm2,ymm3                ;  9  13  11  15
vunpckhpd ymm11,ymm2,ymm3                ; 10  14  12  16
vunpcklpd ymm12,ymm4,ymm5                ;  1   5   3   7
vunpckhpd ymm13,ymm4,ymm5                ;  2   6   4   8
vunpcklpd ymm14,ymm6,ymm7                ;  9  13  11  15
vunpckhpd ymm15,ymm6,ymm7                ; 10  14  12  16
;---------- (6) ---------- Permutate ----------;
vperm2f128 ymm0,ymm8,ymm10,00100000b     ; 1   5   9  13
vperm2f128 ymm1,ymm9,ymm11,00100000b     ; 2   6  10  14
vperm2f128 ymm2,ymm8,ymm10,00110001b     ; 3   7  11  15 
vperm2f128 ymm3,ymm9,ymm11,00110001b     ; 4   8  12  16
vperm2f128 ymm4,ymm12,ymm14,00100000b    ; 1   5   9  13
vperm2f128 ymm5,ymm13,ymm15,00100000b    ; 2   6  10  14
vperm2f128 ymm6,ymm12,ymm14,00110001b    ; 3   7  11  15 
vperm2f128 ymm7,ymm13,ymm15,00110001b    ; 4   8  12  16
;---------- (7) ---------- Write to memory ----------;
vmovapd [rdx],ymm0
vmovapd [rdx + 32],ymm4
add rdx,r8
vmovapd [rdx],ymm1
vmovapd [rdx + 32],ymm5
add rdx,r8
vmovapd [rdx],ymm2
vmovapd [rdx + 32],ymm6
add rdx,r8
vmovapd [rdx],ymm3
vmovapd [rdx + 32],ymm7
add rdx,r8
;---------- (9) ---------- Make cycle ----------;
add r10,4*8
add r11,rax
dec r9
jnz lineCycle
;---------- (10) ---------- Restore non-volatile (by x64 calling convention) registers ----------;
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
mov rsp,rbp
pop rbp
ret
transposeAVX256 ENDP


transposeAVX256NT PROC
;---------- (1) ---------- Save non-volatile (by x64 calling convention) registers ----------;
;---------- (2) ---------- Initializing context not changed in the cycle ----------;
push rbp
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
mov rax,r8
shl r8,3
shl rax,5
shr r9,2                         ; Because 4 scalars per vector register. Yet only quadrat matrices supported.
sub rsp,10*16                    ; 10 128-bit SSE registers XMM6-XMM15 must be non-volatile by convention, note ALIGN16
mov r10,rcx
mov r11,rdx
vmovapd [rsp + 16*0],xmm6
vmovapd [rsp + 16*1],xmm7
vmovapd [rsp + 16*2],xmm8
vmovapd [rsp + 16*3],xmm9
vmovapd [rsp + 16*4],xmm10
vmovapd [rsp + 16*5],xmm11
vmovapd [rsp + 16*6],xmm12
vmovapd [rsp + 16*7],xmm13
vmovapd [rsp + 16*8],xmm14
vmovapd [rsp + 16*9],xmm15
;---------- (3) ---------- Start cycle ----------;
lineCycle:
mov rcx,r10
mov rdx,r11
;---------- (4) ---------- Load source data ----------;
vmovapd ymm0,[rcx]                       ;  1   2   3   4
add rcx,r8
vmovapd ymm1,[rcx]                       ;  5   6   7   8
add rcx,r8
vmovapd ymm2,[rcx]                       ;  9  10  11  12
add rcx,r8
vmovapd ymm3,[rcx]                       ; 13  14  15  16
add rcx,r8
vmovapd ymm4,[rcx]                       ;  1   2   3   4
add rcx,r8
vmovapd ymm5,[rcx]                       ;  5   6   7   8
add rcx,r8
vmovapd ymm6,[rcx]                       ;  9  10  11  12
add rcx,r8
vmovapd ymm7,[rcx]                       ; 13  14  15  16
add rcx,r8
;---------- (5) ---------- Unpack ----------;
vunpcklpd ymm8,ymm0,ymm1                 ;  1   5   3   7
vunpckhpd ymm9,ymm0,ymm1                 ;  2   6   4   8
vunpcklpd ymm10,ymm2,ymm3                ;  9  13  11  15
vunpckhpd ymm11,ymm2,ymm3                ; 10  14  12  16
vunpcklpd ymm12,ymm4,ymm5                ;  1   5   3   7
vunpckhpd ymm13,ymm4,ymm5                ;  2   6   4   8
vunpcklpd ymm14,ymm6,ymm7                ;  9  13  11  15
vunpckhpd ymm15,ymm6,ymm7                ; 10  14  12  16
;---------- (6) ---------- Permutate ----------;
vperm2f128 ymm0,ymm8,ymm10,00100000b     ; 1   5   9  13
vperm2f128 ymm1,ymm9,ymm11,00100000b     ; 2   6  10  14
vperm2f128 ymm2,ymm8,ymm10,00110001b     ; 3   7  11  15 
vperm2f128 ymm3,ymm9,ymm11,00110001b     ; 4   8  12  16
vperm2f128 ymm4,ymm12,ymm14,00100000b    ; 1   5   9  13
vperm2f128 ymm5,ymm13,ymm15,00100000b    ; 2   6  10  14
vperm2f128 ymm6,ymm12,ymm14,00110001b    ; 3   7  11  15 
vperm2f128 ymm7,ymm13,ymm15,00110001b    ; 4   8  12  16
;---------- (7) ---------- Write to memory ----------;
vmovntpd ymmword ptr [rdx],ymm0
vmovntpd ymmword ptr [rdx + 32],ymm4
add rdx,r8
vmovntpd ymmword ptr [rdx],ymm1
vmovntpd ymmword ptr [rdx + 32],ymm5
add rdx,r8
vmovntpd ymmword ptr [rdx],ymm2
vmovntpd ymmword ptr [rdx + 32],ymm6
add rdx,r8
vmovntpd ymmword ptr [rdx],ymm3
vmovntpd ymmword ptr [rdx + 32],ymm7
add rdx,r8
;---------- (9) ---------- Make cycle ----------;
add r10,4*8
add r11,rax
dec r9
jnz lineCycle
;---------- (10) ---------- Restore non-volatile (by x64 calling convention) registers ----------;
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
mov rsp,rbp
pop rbp
ret
transposeAVX256NT ENDP

END