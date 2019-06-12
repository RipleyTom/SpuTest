#include <stdint.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>
#include <cell/atomic.h>
#include "shared_defines.h"

CELL_SPU_LS_PARAM(16 * 1024, 16 * 1024);

static uint64_t ls_version __attribute__((aligned(128)));

int cellSpursTaskMain(qword argTask, uint64_t argTaskset)
{
	const uint64_t address = argTaskset;

	for (unsigned long counter = 0; counter < NUM_INCREMENTS; counter++)
	{
		cellAtomicIncr64(&ls_version, address);
	}

	return 0;
}
