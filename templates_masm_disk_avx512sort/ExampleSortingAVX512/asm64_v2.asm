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
; This module connected for 64-bit build. See also asm32_v2.asm for 32-bit.
; 

.CODE
ALIGN 16

_CMP_LE_OQ  EQU  12h

;-----------------------------------------------------------------------------------------------;
; Parm#1 = RCX  = pointer to array.                                                             ;
; Parm#2 = RDX  = bits [31-00] = EDX, number of elements in the array.                          ;
;-----------------------------------------------------------------------------------------------;

asmSortingPartitioned_v2 PROC
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
lea rax,[permuteConstants_v2]             ; TODO. Alignment 64 is better for this (VMOVAPD instead VMOVUPD).
lea edi,[edx - 1]
vmovupd zmm16,[rax]
vmovupd zmm17,[rax + 64]
vmovupd zmm18,[rax + 128]
vmovupd zmm19,[rax + 192]
vbroadcastsd zmm22,qword ptr [rax + 256]
call internalSortRecursive_v2
mov rbx,[rsp + 8]
mov rsi,[rsp + 16]
mov rdi,[rsp + 24]
mov rbp,[rsp + 32]
ret
asmSortingPartitioned_v2 ENDP

;----------------------------------;
; RBX = Pointer to array.          ;
; ESI = Left index.                ;
; EDI = Right index.               ;
;----------------------------------;

internalSortRecursive_v2 PROC

mov eax,edi
sub eax,esi
jle done
cmp eax,7
jle sortNetwork8

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

;--- call asmPartitioning_v2 ---
;-----------------------------------------------------------------------------------------------;
; Parm#1 = RCX  = pointer to array.                                                             ;
; Parm#2 = RDX  = bits [31-00] = EDX, number of elements in the array.                          ;
; CHANGED: Parm#3 = XMM2 = bits [63-00], pivot value, double                                    ;
;                                                                                               ;
; Return = EAX = Bound position after partitioning, last index of element with VALUE <= PIVOT.  ;
;                                                                                               ;
; Unlike calling convention, this procedure not requires parameters shadow and aligned stack.   ;
; Can be simple called from pure-assembler routines and from C++ code.                          ;
; Plus, must not destroy ZMM16-ZMM19 and ZMM22, ZMM23.                                          ;
;-----------------------------------------------------------------------------------------------;
;--- asmPartitioning_v2 PROC ---
cmp edx,16
ja above16
mov r10,rcx
mov ecx,16
sub ecx,edx
mov r8d,0FFh
shr r8d,cl
xor eax,eax
kmovw k3,r8d
vmovupd zmm1,[r10]
vmovupd zmm2{k3}{z},[r10 + 64]
vcmppd k1, zmm1, zmm0, _CMP_LE_OQ        ; K1 = Left source mask, it for VALUES <= PIVOT.
kmovd r8d,k1                             ; R8D = Left source mask, it for VALUES <= PIVOT.
popcnt r9d,r8d                           ; R9D = Number of bits in the left source mask.
knotw k2,k1                              ; K2 = Right source mask, it for VALUES > PIVOT.
mov r8d,8                                ; R8D = 8, used for calculate number of bits in the right source mask.
sub r8d,r9d                              ; R8D = Number of bits in the right source mask.
vcompresspd [r10 + rax * 8] {k1}, zmm1   ; Store VALUES <= PIVOT to left position, by left mask.
sub edx,r8d                              ; EDX = (EDX - number of bits in the right source mask), shift right position.
add eax,r9d                              ; EAX = (EAX - number of bits in the left source mask), shift left position.
vcompresspd [r10 + rdx * 8] {k2}, zmm1   ; Store VALUES > PIVOT to right position, by right mask.
vcmppd k1, zmm2, zmm0, _CMP_LE_OQ        ; K1 = Left source mask, it for VALUES <= PIVOT.
kandw k1,k1,k3
kmovd r8d,k1                             ; R8D = Left source mask, it for VALUES <= PIVOT.
popcnt r9d,r8d                           ; R9D = Number of bits in the left source mask.
kandnw k2,k1,k3                          ; K2 = Right source mask, it for VALUES > PIVOT.
mov r8d,8                                ; R8D = 8, used for calculate number of bits in the right source mask.
sub r8d,ecx
sub r8d,r9d                              ; R8D = Number of bits in the right source mask.
vcompresspd [r10 + rax * 8] {k1}, zmm2   ; Store VALUES <= PIVOT to left position, by left mask.
sub edx,r8d                              ; EDX = (EDX - number of bits in the right source mask), shift right position.
add eax,r9d                              ; EAX = (EAX - number of bits in the left source mask), shift left position.
vcompresspd [r10 + rdx * 8] {k2}, zmm2   ; Store VALUES > PIVOT to right position, by right mask.
jmp skip1

;--- Common case for N>16 elements. ---
above16:                                 ; ZMM0 = Broadcast pivot.
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
;--- asmPartitioning_v2 ENDP ---

skipPivot:
sub r12,8
lea edx,[eax + esi]
cmp edx,edi
jae changePivot

mov r12d,eax
mov ebp,edi
lea edi,[esi + eax - 1]
call internalSortRecursive_v2     ; Recursive call for LEFT sub-array.

mov edi,ebp
add esi,r12d
call internalSortRecursive_v2     ; Recursive call for RIGHT sub-array.

doneRegs:
pop r12
pop rbp
pop rdi
pop rsi
done:
ret

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

internalSortRecursive_v2 ENDP


.DATA
ALIGN 16                                    ; TODO. Alignment 64 is better for this.
permuteConstants_v2  dq  1,0,3,2,5,4,7,6
                     dq  3,2,1,0,7,6,5,4
                     dq  7,6,5,4,3,2,1,0
                     dq  2,3,0,1,6,7,4,5
qnanConstant_v2      dq  07FF8000000000001h

END

