#include <stdint.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>
#include "shared_defines.h"

CELL_SPU_LS_PARAM(16 * 1024, 16 * 1024);

int cellSpursTaskMain(qword argTask, uint64_t argTaskset)
{
	const unsigned short mask = (unsigned short)si_to_uint(argTask);

	for (unsigned long counter = 0; counter < NUM_PONGS; counter++)
	{
		unsigned short mask_tag = mask;
		cellSpursEventFlagWait(argTaskset, &mask_tag, CELL_SPURS_EVENT_FLAG_AND);
		cellSpursEventFlagClear(argTaskset, 0xFFFF);
		mask_tag = mask << 1;
		cellSpursEventFlagSet(argTaskset, mask_tag);
	}

	return 0;
}
