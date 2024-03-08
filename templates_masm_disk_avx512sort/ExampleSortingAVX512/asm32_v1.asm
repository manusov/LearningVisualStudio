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
; This module connected for 32-bit build. See also asm64_v1.asm for 64-bit.
; 

.686P
.XMM
.MODEL FLAT, C

.CODE
ALIGN 8
OPTION LANGUAGE: syscall

_CMP_LE_OQ  EQU  12h

;-----------------------------------------------------------------------------------------------;
; Parm#1 = dword [esp + 04] = pointer to array.                                                 ;
; Parm#2 = dword [esp + 08] = number of elements in the array.                                  ;
;-----------------------------------------------------------------------------------------------;

_asmSortingPartitioned_v1@8 PROC
mov eax,0AAh
push ebx
kmovw k5,eax
push esi
mov eax,0CCh
push edi
kmovw k6,eax
push ebp
mov eax,0F0h
mov ebx,[esp + 20]
kmovw k7,eax
mov edx,[esp + 24]
xor esi,esi
lea edi,[edx - 1]
call internalSortRecursive_v1
pop ebp
pop edi
pop esi
pop ebx
ret 8
_asmSortingPartitioned_v1@8 ENDP

;----------------------------------;
; EBX = Pointer to array.          ;
; ESI = Left index.                ;
; EDI = Right index.               ;
;----------------------------------;

internalSortRecursive_v1 PROC

mov eax,edi
sub eax,esi
jle done
cmp eax,7
jle sortNetwork8

push esi
push edi
push ebp
lea eax,[ebx + edi * 8]
push eax

changePivot:
lea ecx,[ebx + esi * 8]        ; ECX = Pointer to first element.
lea edx,[edi + 1]
sub edx,esi                    ; EDX = Number of elements.
mov eax,[esp]
cmp eax,ecx
jb doneRegs

push edx
vbroadcastsd zmm0,qword ptr [eax]   ; XMM0[63-0] = Pivot value, and broadcast to ZMM.
push ecx

;--- call _asmPartitioning_v1@16 ---
;-----------------------------------------------------------------------------------------------;
; Parm#1 = dword [esp + 04] = pointer to array.                  ECX                            ;
; Parm#2 = dword [esp + 08] = number of elements in the array.   EDX                            ;
; CHANGED: Parm#3 = qword [esp + 12] = pivot value, double.      ZMM0 broadcast                 ;
;                                                                                               ;
; Return = EAX = Bound position after partitioning, last index of element with VALUE <= PIVOT.  ;
;-----------------------------------------------------------------------------------------------;
;--- _asmPartitioning_v1@16 PROC ---
cmp edx,16
ja above16
push ebx
push esi
push edi
mov ebx,ecx
mov ecx,16
sub ecx,edx
mov eax,0FFh
shr eax,cl
kmovw k3,eax
xor eax,eax
vmovupd zmm1,[ebx]
vmovupd zmm2{k3}{z},[ebx + 64]
vcmppd k1, zmm1, zmm0, _CMP_LE_OQ        ; K1 = Left source mask, it for VALUES <= PIVOT.
kmovd esi,k1                             ; ESI = Left source mask, it for VALUES <= PIVOT.
popcnt edi,esi                           ; EDI = Number of bits in the left source mask.
knotw k2,k1                              ; K2 = Right source mask, it for VALUES > PIVOT.
mov esi,8                                ; ESI = 8, used for calculate number of bits in the right source mask.
sub esi,edi                              ; ESI = Number of bits in the right source mask.
vcompresspd [ebx + eax * 8] {k1}, zmm1   ; Store VALUES <= PIVOT to left position, by left mask.
sub edx,esi                              ; EDX = (EDX - number of bits in the right source mask), shift right position.
add eax,edi                              ; EAX = (EAX - number of bits in the left source mask), shift left position.
vcompresspd [ebx + edx * 8] {k2}, zmm1   ; Store VALUES > PIVOT to right position, by right mask.
vcmppd k1, zmm2, zmm0, _CMP_LE_OQ        ; K1 = Left source mask, it for VALUES <= PIVOT.
kandw k1,k1,k3
kmovd esi,k1                             ; ESI = Left source mask, it for VALUES <= PIVOT.
popcnt edi,esi                           ; EDI = Number of bits in the left source mask.
kandnw k2,k1,k3                          ; K2 = Right source mask, it for VALUES > PIVOT.
mov esi,8                                ; ESI = 8, used for calculate number of bits in the right source mask.
sub esi,ecx
sub esi,edi                              ; ESI = Number of bits in the right source mask.
vcompresspd [ebx + eax * 8] {k1}, zmm2   ; Store VALUES <= PIVOT to left position, by left mask.
sub edx,esi                              ; EDX = (EDX - number of bits in the right source mask), shift right position.
add eax,edi                              ; EAX = (EAX - number of bits in the left source mask), shift left position.
vcompresspd [ebx + edx * 8] {k2}, zmm2   ; Store VALUES > PIVOT to right position, by right mask.
pop edi
pop esi
pop ebx
jmp skip1

;--- Common case for N>16 elements. ---
above16:                                 ; ECX = Pointer to array, ZMM0 = Broadcast pivot.
push ebx
push esi
push edi
push ebp
xor esi,esi                              ; ESI = Left pre-loading index (leftSrc).
lea edi,[edx - 8]                        ; EDI = Right pre-loading index (rightSrc). EDX = Right storing index, right group used for storing VALUES > PIVOT (rightDst).
xor eax,eax                              ; EAX = Left storing index, left group used for storing VALUES <= PIVOT (leftDst).
vmovupd zmm1,[ecx + esi * 8]             ; ZMM1 = Get first 8 values from array LEFT.
vmovupd zmm2,[ecx + edi * 8]             ; ZMM2 = Get last 8 values from array LEFT, cache tail.
;--- This cycle for 8-elements groups. ---
arrayCycle:
vcmppd k1, zmm1, zmm0, _CMP_LE_OQ        ; This fragment for main cycle. K1 = Left source mask, it for VALUES <= PIVOT.
vmovupd zmm3,[ecx + esi * 8 + 64]
kmovd ebx,k1                             ; EBX = Left source mask, it for VALUES <= PIVOT.
vmovupd zmm4,[ecx + edi * 8 - 64]
popcnt ebp,ebx                           ; EBP = Number of bits in the left source mask.
knotw k2,k1                              ; K2 = Right source mask, it for VALUES > PIVOT.
mov ebx,8                                ; EBX = 8, used for calculate number of bits in the right source mask.
sub ebx,ebp                              ; EBX = Number of bits in the right source mask.
vcompresspd [ecx + eax * 8] {k1}, zmm1   ; Store VALUES <= PIVOT to left position, by left mask.
sub edx,ebx                              ; EDX = (EDX - number of bits in the right source mask), shift right position.
add eax,ebp                              ; EAX = (EAX - number of bits in the left source mask), shift left position.
vcompresspd [ecx + edx * 8] {k2}, zmm1   ; Store VALUES > PIVOT to right position, by right mask.
cmp eax,esi
jle loadRight
loadLeft:
add esi,8
vmovapd zmm1,zmm3
jmp loadDone
loadRight:
sub edi,8
vmovapd zmm1,zmm4
loadDone:
mov ebx,edi
sub ebx,esi
cmp ebx,8           ; EBX = Number of elements in the tail.
jae arrayCycle
;--- This fragment for N<8 elements tail after 8-elements group cycle. ---
test ebx,ebx
jz absentTail8
mov ebp,ecx
mov ecx,8
sub ecx,ebx
mov esi,0FFh
shr esi,cl
kmovw k3,esi
mov ecx,ebp
vmovapd zmm1{k3},zmm3
vcmppd k1, zmm1, zmm0, _CMP_LE_OQ        ; K1 = Left source mask, it for VALUES <= PIVOT.
kandw k1,k1,k3
kmovw edi,k1                             ; EDI = Left source mask, it for VALUES <= PIVOT.
popcnt ebp,edi                           ; EBP = Number of bits in the left source mask.
kandnw k2,k1,k3                          ; K2 = Right source mask, it for VALUES > PIVOT.
sub ebx,ebp                              ; EBX = Number of bits in the right source mask.
vcompresspd [ecx + eax * 8] {k1}, zmm1   ; Store VALUES <= PIVOT to left position, by left mask.
sub edx,ebx                              ; EDX = (EDX - number of bits in the right source mask), shift right position.
add eax,ebp                              ; EAX = (EAX - number of bits in the left source mask), shift left position.
vcompresspd [ecx + edx * 8] {k2}, zmm1   ; Store VALUES > PIVOT to right position, by right mask.
absentTail8:
;--- This fragment for cached 8-elements tail. ---
vcmppd k1, zmm2, zmm0, _CMP_LE_OQ        ; This fragment for cached tail, K1 = Left source mask, it for VALUES <= PIVOT.
kmovd ebx,k1                             ; EBX = Left source mask, it for VALUES <= PIVOT.
popcnt ebp,ebx                           ; EBP = Number of bits in the left source mask.
knotw k2,k1                              ; K2 = Right source mask, it for VALUES > PIVOT.
mov ebx,8                                ; EBX = 8, used for calculate number of bits in the right source mask.
sub ebx,ebp                              ; EBX = Number of bits in the right source mask.
vcompresspd [ecx + eax * 8] {k1}, zmm2   ; Store VALUES <= PIVOT to left position, by left mask.
sub edx,ebx                              ; EDX = (EDX - number of bits in the right source mask), shift right position.
add eax,ebp                              ; EAX = (EAX - number of bits in the left source mask), shift left position.
vcompresspd [ecx + edx * 8] {k2}, zmm2   ; Store VALUES > PIVOT to right position, by right mask.
pop ebp
pop edi
pop esi
pop ebx
skip1:
add esp,8
;--- _asmPartitioning_v1@16 ENDP ---

sub dword ptr [esp],8
lea edx,[eax + esi]
cmp edx,edi
jae changePivot

mov [esp],eax
mov ebp,edi
lea edi,[esi + eax - 1]
call internalSortRecursive_v1   ; Recursive call for LEFT sub-array.

mov edi,ebp
add esi,[esp]
call internalSortRecursive_v1   ; Recursive call for RIGHT sub-array.

doneRegs:
pop eax                        ; This POP for free stack only.
pop ebp
pop edi
pop esi
done:
ret

sortNetwork8:
mov ecx,7
sub ecx,eax
mov eax,0FFh
shr eax,cl
kmovw k4,eax
lea eax,[permuteConstants_v1]              ; TODO. Alignment 64 is better for this (VMOVAPD instead VMOVUPD).
vbroadcastsd zmm4,qword ptr [eax + 256]
vmovupd zmm0,[eax]
vmovupd zmm1,[eax + 64]
vmovupd zmm4{k4},[ebx + esi * 8]
vmovupd zmm2,[eax + 128]
vmovupd zmm3,[eax + 192]

vpermpd zmm5,zmm0,zmm4     ; OPTIMIZATION REQUIRED, BECAUSE X8 SORTING NETWORK USED FOR N<8 ELEMENTS ALSO.
vminpd zmm6,zmm5,zmm4
vmaxpd zmm7,zmm5,zmm4
vmovapd zmm6{k5},zmm7
vpermpd zmm7,zmm1,zmm6
vminpd zmm4,zmm7,zmm6
vmaxpd zmm5,zmm7,zmm6
vmovapd zmm4{k6},zmm5
vpermpd zmm5,zmm0,zmm4
vminpd zmm6,zmm5,zmm4
vmaxpd zmm7,zmm5,zmm4
vmovapd zmm6{k5},zmm7
vpermpd zmm7,zmm2,zmm6
vminpd zmm4,zmm7,zmm6
vmaxpd zmm5,zmm7,zmm6
vmovapd zmm4{k7},zmm5
vpermpd zmm5,zmm3,zmm4
vminpd zmm6,zmm5,zmm4
vmaxpd zmm7,zmm5,zmm4
vmovapd zmm6{k6},zmm7
vpermpd zmm7,zmm0,zmm6
vminpd zmm4,zmm7,zmm6
vmaxpd zmm5,zmm7,zmm6
vmovapd zmm4{k5},zmm5

vmovupd [ebx + esi * 8]{k4},zmm4
ret
internalSortRecursive_v1 ENDP


.DATA
ALIGN 16                                    ; TODO. Alignment 64 is better for this.
permuteConstants_v1  dq  1,0,3,2,5,4,7,6
                     dq  3,2,1,0,7,6,5,4
                     dq  7,6,5,4,3,2,1,0
                     dq  2,3,0,1,6,7,4,5
qnanConstant_v1      dq  07FF8000000000001h

OPTION LANGUAGE: C
END

