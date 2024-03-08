;
; Sorting algorithm debug.
; Quick sort and sorting networks with AVX512 instructions.
; Starts with variant for double precision numbers.
;
; This project based on Intel sources:
; https://github.com/intel/x86-simd-sort
; https://github.com/berenger-eu/avx-512-sort/tree/master
; Use example at document:
; 1704.08579.pdf. Appendix B, code 2.
; See also:
; LIPIcs.SEA.2021.3.pdf.
;
; Special thanks for C++ lessons:
; https://ravesli.com/uroki-cpp/
;
; Same template used by: Mass storage performance test. Classes source file edition.
; https://github.com/manusov/LearningVisualStudio/tree/master/framework_disk_test_classes
; More projects at:
; https://github.com/manusov?tab=repositories
;
; Performance-critical subroutines written on MASM.
; This module connected for 64-bit build. See also asm32_v3.asm for 32-bit.
; 

.CODE
ALIGN 16

_CMP_LE_OQ  EQU  12h

;-----------------------------------------------------------------------------------------------;
; Parm#1 = RCX  = pointer to array.                                                             ;
; Parm#2 = RDX  = bits [31-00] = EDX, number of elements in the array.                          ;
;-----------------------------------------------------------------------------------------------;

asmSortingPartitioned_v3 PROC
mov eax,0AAh
mov [rsp + 8],rbx
kmovw k5,eax
mov [rsp + 16],rsi
mov eax,0CCh
mov [rsp + 24],rdi
kmovw k6,eax
mov [rsp + 32],rbp
mov eax,0F0h
mov rbx,rcx
kmovw k7,eax
xor esi,esi
lea rax,[permuteConstants_v3]             ; TODO. Alignment 64 is better for this (VMOVAPD instead VMOVUPD).
lea edi,[edx - 1]
vmovupd zmm16,[rax]
vmovupd zmm17,[rax + 64]
vmovupd zmm18,[rax + 128]
vmovupd zmm19,[rax + 192]
vmovupd zmm24,[rax + 256]
vbroadcastsd zmm22,qword ptr [rax + 320]
call internalSortRecursive_v3
mov rbx,[rsp + 8]
mov rsi,[rsp + 16]
mov rdi,[rsp + 24]
mov rbp,[rsp + 32]
ret
asmSortingPartitioned_v3 ENDP

;----------------------------------;
; RBX = Pointer to array.          ;
; ESI = Left index.                ;
; EDI = Right index.               ;
;----------------------------------;

internalSortRecursive_v3 PROC

mov eax,edi
sub eax,esi
jle done
cmp eax,1
je sortNetwork2
cmp eax,3
jle sortNetwork4
cmp eax,7
jle sortNetwork8
cmp eax,15
jle sortNetwork16

push rsi
push rdi
push rbp
push r12
lea r12,[rbx + rdi * 8]

vmovsd xmm23,qword ptr [r12]   ; Note first iteration always match and skipped.

changePivot:
lea rcx,[rbx + rsi * 8]        ; RCX = Pointer to first element.
lea edx,[edi + 1]
sub edx,esi                    ; EDX = Number of elements.  TODO. USE PREVIOUS EAX.
cmp r12,rcx
jb doneRegs

vbroadcastsd zmm0,qword ptr [r12]    ; XMM0[63-0] = Pivot value.
vucomisd xmm0,xmm23
je skipPivot
vmovapd xmm23,xmm0

;-----------------------------------------------------------------------------------------------;
; Parm#1 = RCX  = pointer to array.                                                             ;
; Parm#2 = RDX  = bits [31-00] = EDX, number of elements in the array.                          ;
; CHANGED: Parm#3 = XMM2 = bits [63-00], pivot value, double                                    ;
;                                                                                               ;
; Return = EAX = Bound position after partitioning, last index of element with VALUE <= PIVOT.  ;
;                                                                                               ;
; Unlike calling convention, this procedure not requires parameters shadow and aligned stack.   ;
; Can be simple called from pure-assembler routines and from C++ code.                          ;
; Plus, must not destroy ZMM16-ZMM19 and ZMM22, ZMM24.                                          ;
;-----------------------------------------------------------------------------------------------;

                                         ; ZMM0 = Broadcast pivot.
xor r10d,r10d                            ; R10 = Left pre-loading index (leftSrc).
lea r11d,[edx - 8]                       ; R11 = Right pre-loading index (rightSrc).
xor eax,eax                              ; RAX = Left storing index, left group used for storing VALUES <= PIVOT (leftDst).
mov edx,edx                              ; This clears bits RDX.[63-32]. RDX = Right storing index, right group used for storing VALUES > PIVOT (rightDst).
vmovupd zmm1,[rcx + r10 * 8]             ; ZMM1 = Get first 8 values from array LEFT.
vmovupd zmm2,[rcx + r11 * 8]             ; ZMM2 = Get last 8 values from array LEFT, cache tail.

;--- This cycle for 8-elements groups. ---

arrayCycle:
vcmppd k1, zmm1, zmm0, _CMP_LE_OQ        ; This fragment for main cycle. K1 = Left source mask, it for VALUES <= PIVOT.
vmovupd zmm3,[rcx + r10 * 8 + 64]
kmovd r8d,k1                             ; R8D = Left source mask, it for VALUES <= PIVOT.
vmovupd zmm4,[rcx + r11 * 8 - 64]
popcnt r9d,r8d                           ; R9D = Number of bits in the left source mask.
knotw k2,k1                              ; K2 = Right source mask, it for VALUES > PIVOT.
mov r8d,8                                ; R8D = 8, used for calculate number of bits in the right source mask.
sub r8d,r9d                              ; R8D = Number of bits in the right source mask.
vcompresspd [rcx + rax * 8] {k1}, zmm1   ; Store VALUES <= PIVOT to left position, by left mask.
sub edx,r8d                              ; EDX = (EDX - number of bits in the right source mask), shift right position.
add eax,r9d                              ; EAX = (EAX - number of bits in the left source mask), shift left position.
vcompresspd [rcx + rdx * 8] {k2}, zmm1   ; Store VALUES > PIVOT to right position, by right mask.
cmp eax,r10d
jle loadRight
loadLeft:
add r10d,8
vmovapd zmm1,zmm3
jmp loadDone
loadRight:
sub r11d,8
vmovapd zmm1,zmm4
loadDone:
mov r8d,r11d
sub r8d,r10d
cmp r8d,8           ; R8D = Number of elements in the tail.
jae arrayCycle

;--- This fragment for N<8 elements tail after 8-elements group cycle. ---

test r8d,r8d
jz absentTail8
mov r9,rcx
mov ecx,8
sub ecx,r8d
mov r10d,0FFh
shr r10d,cl
kmovw k3,r10d
mov rcx,r9
vmovapd zmm1{k3},zmm3
vcmppd k1, zmm1, zmm0, _CMP_LE_OQ        ; K1 = Left source mask, it for VALUES <= PIVOT.
kandw k1,k1,k3
kmovw r11d,k1                            ; R11D = Left source mask, it for VALUES <= PIVOT.
popcnt r9d,r11d                          ; R9D = Number of bits in the left source mask.
kandnw k2,k1,k3                          ; K2 = Right source mask, it for VALUES > PIVOT.
sub r8d,r9d                              ; R8D = Number of bits in the right source mask.
vcompresspd [rcx + rax * 8] {k1}, zmm1   ; Store VALUES <= PIVOT to left position, by left mask.
sub edx,r8d                              ; EDX = (EDX - number of bits in the right source mask), shift right position.
add eax,r9d                              ; EAX = (EAX - number of bits in the left source mask), shift left position.
vcompresspd [rcx + rdx * 8] {k2}, zmm1   ; Store VALUES > PIVOT to right position, by right mask.
absentTail8:

;--- This fragment for cached 8-elements tail. ---

vcmppd k1, zmm2, zmm0, _CMP_LE_OQ        ; K1 = Left source mask, it for VALUES <= PIVOT.
kmovd r8d,k1                             ; R8D = Left source mask, it for VALUES <= PIVOT.
popcnt r9d,r8d                           ; R9D = Number of bits in the left source mask.
knotw k2,k1                              ; K2 = Right source mask, it for VALUES > PIVOT.
mov r8d,8                                ; R8D = 8, used for calculate number of bits in the right source mask.
sub r8d,r9d                              ; R8D = Number of bits in the right source mask.
vcompresspd [rcx + rax * 8] {k1}, zmm2   ; Store VALUES <= PIVOT to left position, by left mask.
sub edx,r8d                              ; EDX = (EDX - number of bits in the right source mask), shift right position.
add eax,r9d                              ; EAX = (EAX - number of bits in the left source mask), shift left position.
vcompresspd [rcx + rdx * 8] {k2}, zmm2   ; Store VALUES > PIVOT to right position, by right mask.
skip1:

skipPivot:
sub r12,8
lea edx,[eax + esi]
cmp edx,edi
jae changePivot

mov r12d,eax
mov ebp,edi
lea edi,[esi + eax - 1]
call internalSortRecursive_v3     ; Recursive call for LEFT sub-array.

mov edi,ebp
add esi,r12d
call internalSortRecursive_v3     ; Recursive call for RIGHT sub-array.

doneRegs:
pop r12
pop rbp
pop rdi
pop rsi
done:
ret

;---------- SPECIAL SUPPORT FOR N <= 2 SORTING NETWORK. ----------

sortNetwork2:

vmovupd xmm0,[rbx + rsi * 8]
vshufpd ymm2,ymm0,ymm0,0001b   ;   vpermpd ymm2,ymm16,ymm0
vminpd xmm4,xmm2,xmm0
vmaxpd xmm5,xmm2,xmm0
vmovapd xmm4{k5},xmm5
vmovupd [rbx + rsi * 8],xmm4
ret

;---------- SPECIAL SUPPORT FOR N <= 4 SORTING NETWORK. ----------

sortNetwork4:

mov ecx,3
sub ecx,eax
mov eax,0Fh
shr eax,cl
vmovapd ymm0,ymm22
kmovw k4,eax
vmovupd ymm0{k4},[rbx + rsi * 8]

vpermpd ymm2,ymm16,ymm0
vminpd ymm4,ymm2,ymm0
vmaxpd ymm5,ymm2,ymm0
vmovapd ymm4{k5},ymm5

vpermpd ymm2,ymm17,ymm4
vminpd ymm0,ymm2,ymm4
vmaxpd ymm1,ymm2,ymm4
vmovapd ymm0{k6},ymm1

vpermpd ymm2,ymm16,ymm0
vminpd ymm4,ymm2,ymm0
vmaxpd ymm5,ymm2,ymm0
vmovapd ymm4{k5},ymm5

vmovupd [rbx + rsi * 8]{k4},ymm4
ret

;---------- SPECIAL SUPPORT FOR N <= 8 SORTING NETWORK. ----------

sortNetwork8:

mov ecx,7
sub ecx,eax
mov eax,0FFh
shr eax,cl
vmovapd zmm4,zmm22
kmovw k4,eax
vmovupd zmm4{k4},[rbx + rsi * 8]

vpermpd zmm5,zmm16,zmm4     ; OPTIMIZATION REQUIRED, BECAUSE X8 SORTING NETWORK USED FOR N<8 ELEMENTS ALSO.
vminpd zmm20,zmm5,zmm4
vmaxpd zmm21,zmm5,zmm4
vmovapd zmm20{k5},zmm21

vpermpd zmm21,zmm17,zmm20
vminpd zmm4,zmm21,zmm20
vmaxpd zmm5,zmm21,zmm20
vmovapd zmm4{k6},zmm5

vpermpd zmm5,zmm16,zmm4
vminpd zmm20,zmm5,zmm4
vmaxpd zmm21,zmm5,zmm4
vmovapd zmm20{k5},zmm21

vpermpd zmm21,zmm18,zmm20
vminpd zmm4,zmm21,zmm20
vmaxpd zmm5,zmm21,zmm20
vmovapd zmm4{k7},zmm5

vpermpd zmm5,zmm19,zmm4
vminpd zmm20,zmm5,zmm4
vmaxpd zmm21,zmm5,zmm4
vmovapd zmm20{k6},zmm21

vpermpd zmm21,zmm16,zmm20
vminpd zmm4,zmm21,zmm20
vmaxpd zmm5,zmm21,zmm20
vmovapd zmm4{k5},zmm5

vmovupd [rbx + rsi * 8]{k4},zmm4
ret

;---------- SPECIAL SUPPORT FOR N <= 16 SORTING NETWORK. ----------

sortNetwork16:

mov ecx,15
sub ecx,eax
mov eax,0FFh
vmovapd zmm1,zmm22
shr eax,cl
kmovw k4,eax
vmovupd zmm0,[rbx + rsi * 8]
vmovupd zmm1{k4},[rbx + rsi * 8 + 64]

vpermpd zmm2,zmm16,zmm0
vpermpd zmm3,zmm16,zmm1
vminpd zmm4,zmm2,zmm0
vminpd zmm5,zmm3,zmm1
vmaxpd zmm26,zmm2,zmm0
vmaxpd zmm27,zmm3,zmm1
vmovapd zmm4{k5},zmm26
vmovapd zmm5{k5},zmm27

vpermpd zmm2,zmm17,zmm4
vpermpd zmm3,zmm17,zmm5
vminpd zmm0,zmm2,zmm4
vminpd zmm1,zmm3,zmm5
vmaxpd zmm26,zmm2,zmm4
vmaxpd zmm27,zmm3,zmm5
vmovapd zmm0{k6},zmm26
vmovapd zmm1{k6},zmm27

vpermpd zmm2,zmm16,zmm0
vpermpd zmm3,zmm16,zmm1
vminpd zmm4,zmm2,zmm0
vminpd zmm5,zmm3,zmm1
vmaxpd zmm26,zmm2,zmm0
vmaxpd zmm27,zmm3,zmm1
vmovapd zmm4{k5},zmm26
vmovapd zmm5{k5},zmm27

vpermpd zmm2,zmm18,zmm4
vpermpd zmm3,zmm18,zmm5
vminpd zmm0,zmm2,zmm4
vminpd zmm1,zmm3,zmm5
vmaxpd zmm26,zmm2,zmm4
vmaxpd zmm27,zmm3,zmm5
vmovapd zmm0{k7},zmm26
vmovapd zmm1{k7},zmm27

vpermpd zmm2,zmm19,zmm0
vpermpd zmm3,zmm19,zmm1
vminpd zmm4,zmm2,zmm0
vminpd zmm5,zmm3,zmm1
vmaxpd zmm26,zmm2,zmm0
vmaxpd zmm27,zmm3,zmm1
vmovapd zmm4{k6},zmm26
vmovapd zmm5{k6},zmm27

vpermpd zmm2,zmm16,zmm4
vpermpd zmm3,zmm16,zmm5
vminpd zmm0,zmm2,zmm4
vminpd zmm1,zmm3,zmm5
vmaxpd zmm26,zmm2,zmm4
vmaxpd zmm27,zmm3,zmm5
vmovapd zmm0{k5},zmm26
vmovapd zmm1{k5},zmm27

vpermpd zmm0,zmm18,zmm0
vminpd zmm2,zmm0,zmm1
vmaxpd zmm1,zmm0,zmm1
vpermpd zmm0,zmm18,zmm2

vpermpd zmm2,zmm24,zmm0
vpermpd zmm3,zmm24,zmm1
vminpd zmm4,zmm2,zmm0
vminpd zmm5,zmm3,zmm1
vmaxpd zmm26,zmm2,zmm0
vmaxpd zmm27,zmm3,zmm1
vmovapd zmm4{k7},zmm26
vmovapd zmm5{k7},zmm27

vpermpd zmm2,zmm19,zmm4
vpermpd zmm3,zmm19,zmm5
vminpd zmm0,zmm2,zmm4
vminpd zmm1,zmm3,zmm5
vmaxpd zmm26,zmm2,zmm4
vmaxpd zmm27,zmm3,zmm5
vmovapd zmm0{k6},zmm26
vmovapd zmm1{k6},zmm27

vpermpd zmm2,zmm16,zmm0
vpermpd zmm3,zmm16,zmm1
vminpd zmm4,zmm2,zmm0
vminpd zmm5,zmm3,zmm1
vmaxpd zmm26,zmm2,zmm0
vmaxpd zmm27,zmm3,zmm1
vmovapd zmm4{k5},zmm26
vmovapd zmm5{k5},zmm27

vmovupd [rbx + rsi * 8],zmm4
vmovupd [rbx + rsi * 8 + 64]{k4},zmm5
ret

;---------- END OF SPECIAL SUPPORT FOR N <= 16 SORTING NETWORK. ----------

internalSortRecursive_v3 ENDP


.DATA
ALIGN 16                                    ; TODO. Alignment 64 is better for this.
permuteConstants_v3  dq  1,0,3,2,5,4,7,6
                     dq  3,2,1,0,7,6,5,4
                     dq  7,6,5,4,3,2,1,0
                     dq  2,3,0,1,6,7,4,5
                     dq  4,5,6,7,0,1,2,3
                     dq  07FF0000000000000h    ; +INF

END

