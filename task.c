#include <stdint.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>

CELL_SPU_LS_PARAM(16 * 1024, 16 * 1024);

unsigned long thevar = 0x12345678;

int cellSpursTaskMain(qword argTask, uint64_t argTaskset)
{
	(void)argTaskset;

	unsigned long x = si_to_int(argTask);

	x += thevar;

	return x;
}
