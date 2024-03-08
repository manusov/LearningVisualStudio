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
; This module connected for 32-bit build. See also asm64_net.asm for 64-bit.
; Sorting networks debug.
; 

.686P
.XMM
.MODEL FLAT, C

.CODE
ALIGN 8
OPTION LANGUAGE: syscall

;-----------------------------------------------------------------------------------------------;
; Parm#1 = dword [esp + 04] = pointer to array.                                                 ;
; Parm#2 = dword [esp + 08] = number of elements in the array.                                  ;
;                             Must be from 0 to 4.                                              ;
;-----------------------------------------------------------------------------------------------;

_asmSortingNetworkX4@8 PROC
; UNDER CONSTRUCTION.
ret 8
_asmSortingNetworkX4@8 ENDP

;-----------------------------------------------------------------------------------------------;
; Parm#1 = dword [esp + 04] = pointer to array.                                                 ;
; Parm#2 = dword [esp + 08] = number of elements in the array.                                  ;
;                             Must be from 9 to 16.                                             ;
;-----------------------------------------------------------------------------------------------;

_asmSortingNetworkX16@8 PROC
lea eax,[permuteConstants]                ; TODO. Alignment 64 is better for this (VMOVAPD instead VMOVUPD).
mov ecx,[esp + 4]
vmovupd zmm0,[eax]
vmovupd zmm1,[eax + 64]
vmovupd zmm2,[eax + 128]
vmovupd zmm3,[eax + 192]
mov eax,0AAh
kmovd k1,eax
mov eax,0CCh
kmovd k2,eax
mov eax,0F0h
kmovd k3,eax

push ebx
push esi
push edi

; UNDER CONSTRUCTION. POSSIBLE THIS NETWORK NOT USED AT IA32 MODE.

vmovupd zmm4,[ecx]
call Sort_ZMM4
vmovupd [ecx],zmm4

vmovupd zmm4,[ecx + 64]
call Sort_ZMM4
vmovupd [ecx + 64],zmm4

pop edi
pop esi
pop ebx

ret 8
_asmSortingNetworkX16@8 ENDP

Sort_ZMM4 PROC
vpermpd zmm5,zmm0,zmm4
vminpd zmm6,zmm5,zmm4
vmaxpd zmm7,zmm5,zmm4
vmovapd zmm6{k1},zmm7
vpermpd zmm7,zmm1,zmm6
vminpd zmm4,zmm7,zmm6
vmaxpd zmm5,zmm7,zmm6
vmovapd zmm4{k2},zmm5
vpermpd zmm5,zmm0,zmm4
vminpd zmm6,zmm5,zmm4
vmaxpd zmm7,zmm5,zmm4
vmovapd zmm6{k1},zmm7
vpermpd zmm7,zmm2,zmm6
vminpd zmm4,zmm7,zmm6
vmaxpd zmm5,zmm7,zmm6
vmovapd zmm4{k3},zmm5
vpermpd zmm5,zmm3,zmm4
vminpd zmm6,zmm5,zmm4
vmaxpd zmm7,zmm5,zmm4
vmovapd zmm6{k2},zmm7
vpermpd zmm7,zmm0,zmm6
vminpd zmm4,zmm7,zmm6
vmaxpd zmm5,zmm7,zmm6
vmovapd zmm4{k1},zmm5
ret
Sort_ZMM4 ENDP

.DATA
ALIGN 16                                    ; TODO. Alignment 64 is better for this.
permuteConstants  dq  1,0,3,2,5,4,7,6
                  dq  3,2,1,0,7,6,5,4
                  dq  7,6,5,4,3,2,1,0
                  dq  2,3,0,1,6,7,4,5

OPTION LANGUAGE: C
END
