#include <stdint.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>

CELL_SPU_LS_PARAM(16 * 1024, 16 * 1024);

#define NUM_INT_LOOPS 4000000
#define NUM_VECTORS 64

int cellSpursTaskMain(qword argTask, uint64_t argTaskset)
{
	(void)argTaskset;

	vec_int4 thevecs[NUM_VECTORS];

	for (int index = 0; index < NUM_VECTORS; index++)
	{
		thevecs[index] = (vec_int4)argTask;
	}

	for (unsigned long counter = 0; counter < NUM_INT_LOOPS; counter++)
	{
		for (int index = 0; index < NUM_VECTORS; index++)
		{
			thevecs[index] = spu_add(thevecs[index], thevecs[(index + 1) % NUM_VECTORS]);
			thevecs[index] = spu_madd((vec_short8)thevecs[index], (vec_short8)thevecs[(index + 1) % NUM_VECTORS], thevecs[(index + 2) % NUM_VECTORS]);
			thevecs[index] = spu_mhhadd((vec_short8)thevecs[index], (vec_short8)thevecs[(index + 1) % NUM_VECTORS], thevecs[(index + 2) % NUM_VECTORS]);
			thevecs[index] = spu_mulh((vec_short8)thevecs[index], (vec_short8)thevecs[(index + 1) % NUM_VECTORS]);
			thevecs[index] = spu_mule((vec_short8)thevecs[index], (vec_short8)thevecs[(index + 1) % NUM_VECTORS]);
			thevecs[index] = spu_mulo((vec_short8)thevecs[index], (vec_short8)thevecs[(index + 1) % NUM_VECTORS]);
			thevecs[index] = spu_sub(thevecs[index], thevecs[(index + 1) % NUM_VECTORS]);
		}
	}

	return si_to_int((qword)thevecs[0]);
}
