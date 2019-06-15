#include <stdint.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>
#include <cell/dma.h>

CELL_SPU_LS_PARAM(16 * 1024, 16 * 1024);

#define NUM_PUT 10000000

vec_uint4 ls_buf[128 / sizeof(vec_uint4)] __attribute__((aligned(128)));

int cellSpursTaskMain(qword argTask, uint64_t argTaskset)
{
    const uint64_t addr = argTaskset;

    for (int index = 0; index < (128 / sizeof(vec_uint4)); index++)
    {
        ls_buf[index] = (vec_uint4)argTask;
    }

    vec_uint4 toinc;
    toinc[0] = toinc[1] = toinc[2] = toinc[3] = 1;

    for (unsigned long counter = 0; counter < NUM_PUT; counter++)
    {
        cellDmaPutlluc(ls_buf, addr, 0, 0);
        cellDmaWaitAtomicStatus();
        ls_buf[counter % (128 / sizeof(vec_uint4))] = spu_add(ls_buf[counter % (128 / sizeof(vec_uint4))], toinc);
    }

    return 0;
}
