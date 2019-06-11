#include <stdint.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>

CELL_SPU_LS_PARAM(16 * 1024, 16 * 1024);

#define NUM_FLOAT_LOOPS 1000000
#define NUM_VECTORS 64

int cellSpursTaskMain(qword argTask, uint64_t argTaskset)
{
	(void)argTaskset;

	vec_float4 thevecs[NUM_VECTORS];

	for (int index = 0; index < NUM_VECTORS; index++)
	{
		thevecs[index] = (vec_float4)argTask;
	}

	for (unsigned long counter = 0; counter < NUM_FLOAT_LOOPS; counter++)
	{
		for (int index = 0; index < NUM_VECTORS; index++)
		{
			thevecs[index] = spu_add(thevecs[index], thevecs[(index + 1) % NUM_VECTORS]);
			thevecs[index] = spu_madd(thevecs[index], thevecs[(index + 1) % NUM_VECTORS], thevecs[(index + 2) % NUM_VECTORS]);
			thevecs[index] = spu_msub(thevecs[index], thevecs[(index + 1) % NUM_VECTORS], thevecs[(index + 2) % NUM_VECTORS]);
			thevecs[index] = spu_mul(thevecs[index], thevecs[(index + 1) % NUM_VECTORS]);
			thevecs[index] = (vec_float4)spu_nmadd((vec_double2)thevecs[index], (vec_double2)thevecs[(index + 1) % NUM_VECTORS], (vec_double2)thevecs[(index + 2) % NUM_VECTORS]);
			thevecs[index] = spu_nmsub(thevecs[index], thevecs[(index + 1) % NUM_VECTORS], thevecs[(index + 2) % NUM_VECTORS]);
			thevecs[index] = spu_sub(thevecs[index], thevecs[(index + 1) % NUM_VECTORS]);
		}
	}

	return si_to_int((qword)thevecs[0]);
}
