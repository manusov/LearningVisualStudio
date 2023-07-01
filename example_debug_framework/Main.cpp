/*
Platform components tests. Experiments with differrent styles.
Main module.
*/

#include "Shell.h"
#include "Task_01.h"   // Timers precision test.
#include "Task_02.h"   // AVX512 read benchmark, single/multi thread. Optimized for L1=48K, change size if required.
#include "Task_03.h"   // Sine calculation benchmark.
#include "Task_04.h"   // AVX512 addition by intrinsics benchmark.
#include "Task_05.h"   // AVX512 matrix transpose, step 1 = debug for one cell 8x8 and 16x16, for x64 builds only. TODO. Check not AVX512F only if required extensions.
#include "Task_06.h"   // AVX512 matrix transpose, step 2 = debug for C++ cycle with asm cells 8x8, for x64 builds only.  See TODO list at Task_06.cpp module.
#include "Task_07.h"   // AVX512 matrix transpose, step 3 = debug for asm cycle with asm cells 8x8, for x64 builds only.
#include "Task_08.h"   // AVX512 matrix transpose, step 4 = debug for asm cycle with gathered-scattered loads-stores, for x64 builds only.
#include "Task_09.h"   // AVX512 matrix transpose, 8x16 base block, re-create threads at measurement cycle.
#include "Task_10.h"   // AVX512 matrix transpose, 8x16 base block, create threads one time, measurement cycle in the threads routines.
#include "Task_11.h"   // AVX512 matrix transpose, 8x16 base block, measurement scenario for small (temporal) and big (nontemporal) blocks.
#include "Task_12.h"   // Interrogate threads running timings by TSC.
#include "Task_13.h"   // AVX256 matrix transpose, 4x8 base block, measurement scenario for small (temporal) and big (nontemporal) blocks.
#include "Task_14.h"   // Learn some intrinsics. VPTERNLOG.
#include "Task_15.h"   // Context, data types, initialization, visualization for vector data and intrinsics.
#include "Task_16.h"   // Learn some intrinsics. VPTERNLOG in the new context (context by Task_15).
#include "Task_17.h"   // Learn some intrinsics. AVX512 VNNI instructions.
#include "Task_18.h"   // Learn some intrinsics. Some AVX512F instructions.
#include "Task_19.h"   // Learn some intrinsics, use lambdas for create scenarios useable for many instructions.
#include "Task_20.h"   // Context, data types, initialization, visualization for vector data and intrinsics. v2.
#include "Task_21.h"   // Learn some intrinsics - miscellaneous.
#include "Task_22.h"   // Learn some intrinsics - load.
#include "Task_23.h"   // Learn some intrinsics - SSE.

int main(int argc, char** argv)
{
	Shell shell;
	//
	// Task_01 task;
	// Task_02 task;
	// Task_03 task;
	// Task_04 task;
	// Task_05 task;
	// Task_06 task;
	// Task_07 task;   // Relatively fast. TODO: use 16x8 blocks with all registers ZMM0-ZMM31? Required save-restore XMM6-XMM15, use all in asm? Select this evolution? (!)
	// Task_08 task;   // Relatively slow. TODO: blocking required?
	// Task_09 task;
	// Task_10 task;
	   Task_11 task;   // Complex AVX512 benchmark: temporal - nontemporal, f(matrix size).
	// Task_12 task;
	// Task_13 task;   // Complex AVX256 benchmark: temporal - nontemporal, f(matrix size).
	// Task_14 task;
	// Task_15 task;
	// Task_16 task;
	// Task_17 task;
	// Task_18 task;
	// Task_19 task;
	// Task_20 task;
	// Task_21 task;
	// Task_22 task;
	// Task_23 task;
	//
	return shell.execute(argc, argv, task);
}
