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
; This module connected for 64-bit build. See also asm32_net.asm for 32-bit.
; Sorting networks debug.
; 

.CODE
ALIGN 16

;-----------------------------------------------------------------------------------------------;
; Parm#1 = RCX  = pointer to array.                                                             ;
; Parm#2 = RDX  = bits [31-00] = EDX, number of elements in the array.                          ;
;                                Must be from 0 to 4.                                           ;
;-----------------------------------------------------------------------------------------------;

asmSortingNetworkX4 PROC

lea rax,[permuteConstants]                  ; TODO. Alignment 64 is better for this (VMOVAPD instead VMOVUPD).
vmovupd zmm16,[rax]
vmovupd zmm17,[rax + 64]
vmovupd zmm18,[rax + 128]
vmovupd zmm19,[rax + 192]
vmovupd zmm24,[rax + 256]
vmovupd zmm25,[rax + 320]
vbroadcastsd zmm22,qword ptr [rax + 384]
mov eax,0AAh
kmovw k1,eax
mov eax,0CCh
kmovw k2,eax
mov eax,0F0h
mov r8,rcx
kmovw k3,eax

mov ecx,4
sub ecx,edx
mov eax,0Fh
shr eax,cl
vmovapd ymm0,ymm22
kmovw k4,eax
vmovupd ymm0{k4},[r8]

vpermpd ymm2,ymm16,ymm0
vminpd ymm4,ymm2,ymm0
vmaxpd ymm5,ymm2,ymm0
vmovapd ymm4{k1},ymm5

vpermpd ymm2,ymm17,ymm4
vminpd ymm0,ymm2,ymm4
vmaxpd ymm1,ymm2,ymm4
vmovapd ymm0{k2},ymm1

vpermpd ymm2,ymm16,ymm0
vminpd ymm4,ymm2,ymm0
vmaxpd ymm5,ymm2,ymm0
vmovapd ymm4{k1},ymm5

vmovupd [r8]{k4},ymm4
ret
asmSortingNetworkX4 ENDP


;-----------------------------------------------------------------------------------------------;
; Parm#1 = RCX  = pointer to array.                                                             ;
; Parm#2 = RDX  = bits [31-00] = EDX, number of elements in the array.                          ;
;                                Must be from 9 to 16.                                          ;
;-----------------------------------------------------------------------------------------------;

asmSortingNetworkX16 PROC

lea rax,[permuteConstants]                  ; TODO. Alignment 64 is better for this (VMOVAPD instead VMOVUPD).
vmovupd zmm16,[rax]
vmovupd zmm17,[rax + 64]
vmovupd zmm18,[rax + 128]
vmovupd zmm19,[rax + 192]
vmovupd zmm24,[rax + 256]
vmovupd zmm25,[rax + 320]
vbroadcastsd zmm22,qword ptr [rax + 384]
mov eax,0AAh
vmovupd zmm0,[rcx]
kmovw k1,eax
mov eax,0CCh
;- vmovupd zmm1,[rcx + 64]
kmovw k2,eax
mov eax,0F0h
mov r8,rcx
kmovw k3,eax

mov ecx,16
sub ecx,edx
mov eax,0FFh
shr eax,cl
vmovapd zmm1,zmm22
kmovw k4,eax
vmovupd zmm1{k4},[r8 + 64]

vpermpd zmm2,zmm16,zmm0
vpermpd zmm3,zmm16,zmm1
vminpd zmm4,zmm2,zmm0
vminpd zmm5,zmm3,zmm1
vmaxpd zmm22,zmm2,zmm0
vmaxpd zmm23,zmm3,zmm1
vmovapd zmm4{k1},zmm22
vmovapd zmm5{k1},zmm23

vpermpd zmm2,zmm17,zmm4
vpermpd zmm3,zmm17,zmm5
vminpd zmm0,zmm2,zmm4
vminpd zmm1,zmm3,zmm5
vmaxpd zmm22,zmm2,zmm4
vmaxpd zmm23,zmm3,zmm5
vmovapd zmm0{k2},zmm22
vmovapd zmm1{k2},zmm23

vpermpd zmm2,zmm16,zmm0
vpermpd zmm3,zmm16,zmm1
vminpd zmm4,zmm2,zmm0
vminpd zmm5,zmm3,zmm1
vmaxpd zmm22,zmm2,zmm0
vmaxpd zmm23,zmm3,zmm1
vmovapd zmm4{k1},zmm22
vmovapd zmm5{k1},zmm23

vpermpd zmm2,zmm18,zmm4
vpermpd zmm3,zmm18,zmm5
vminpd zmm0,zmm2,zmm4
vminpd zmm1,zmm3,zmm5
vmaxpd zmm22,zmm2,zmm4
vmaxpd zmm23,zmm3,zmm5
vmovapd zmm0{k3},zmm22
vmovapd zmm1{k3},zmm23

vpermpd zmm2,zmm19,zmm0
vpermpd zmm3,zmm19,zmm1
vminpd zmm4,zmm2,zmm0
vminpd zmm5,zmm3,zmm1
vmaxpd zmm22,zmm2,zmm0
vmaxpd zmm23,zmm3,zmm1
vmovapd zmm4{k2},zmm22
vmovapd zmm5{k2},zmm23

vpermpd zmm2,zmm16,zmm4
vpermpd zmm3,zmm16,zmm5
vminpd zmm0,zmm2,zmm4
vminpd zmm1,zmm3,zmm5
vmaxpd zmm22,zmm2,zmm4
vmaxpd zmm23,zmm3,zmm5
vmovapd zmm0{k1},zmm22
vmovapd zmm1{k1},zmm23

vpermpd zmm0,zmm25,zmm0
vminpd zmm2,zmm0,zmm1
vmaxpd zmm1,zmm0,zmm1
vpermpd zmm0,zmm25,zmm2

vpermpd zmm2,zmm24,zmm0
vpermpd zmm3,zmm24,zmm1
vminpd zmm4,zmm2,zmm0
vminpd zmm5,zmm3,zmm1
vmaxpd zmm22,zmm2,zmm0
vmaxpd zmm23,zmm3,zmm1
vmovapd zmm4{k3},zmm22
vmovapd zmm5{k3},zmm23

vpermpd zmm2,zmm19,zmm4
vpermpd zmm3,zmm19,zmm5
vminpd zmm0,zmm2,zmm4
vminpd zmm1,zmm3,zmm5
vmaxpd zmm22,zmm2,zmm4
vmaxpd zmm23,zmm3,zmm5
vmovapd zmm0{k2},zmm22
vmovapd zmm1{k2},zmm23

vpermpd zmm2,zmm16,zmm0
vpermpd zmm3,zmm16,zmm1
vminpd zmm4,zmm2,zmm0
vminpd zmm5,zmm3,zmm1
vmaxpd zmm22,zmm2,zmm0
vmaxpd zmm23,zmm3,zmm1
vmovapd zmm4{k1},zmm22
vmovapd zmm5{k1},zmm23

vmovupd [r8],zmm4
vmovupd [r8 + 64]{k4},zmm5
ret

asmSortingNetworkX16 ENDP

.DATA
ALIGN 16                                    ; TODO. Alignment 64 is better for this.
permuteConstants  dq  1,0,3,2,5,4,7,6
                  dq  3,2,1,0,7,6,5,4
                  dq  7,6,5,4,3,2,1,0
                  dq  2,3,0,1,6,7,4,5
                  dq  4,5,6,7,0,1,2,3
                  dq  7,6,5,4,3,2,1,0      ; THIS PATTERN DUPLICATED, REMOVE AT OPTIMIZATION.

                ; dq  07FF8000000000001h   ; QNAN
                  dq 7FF0000000000000h     ; +INF

END