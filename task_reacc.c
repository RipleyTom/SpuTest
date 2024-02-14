#include <stdint.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>

CELL_SPU_LS_PARAM(16 * 1024, 16 * 1024);

#define NUM_REACC 800000000ull

int cellSpursTaskMain(qword argTask, uint64_t argTaskset)
{
	(void)argTaskset;

	vec_float4 values = (vec_float4)argTask;
	vec_float4 the_ones = {1.0f, 1.0f, 1.0f, 1.0f};

	for (uint64_t counter = 0; counter < NUM_REACC; counter++)
	{
		vec_float4 frest_result = __builtin_spu_frest((vec_float4)values);
		qword fi_result = __builtin_si_fi((qword)values, (qword)frest_result);
		qword fnms_result = __builtin_si_fnms((qword)values, (qword)fi_result, (qword)the_ones);
		qword fma_result = __builtin_si_fma((qword)fnms_result, (qword)fi_result, (qword)fi_result);
		values = (vec_float4)fma_result;
	}

	return si_to_int((qword)values);
}
