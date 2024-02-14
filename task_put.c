#include <stdint.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>
#include <cell/dma.h>

CELL_SPU_LS_PARAM(16 * 1024, 16 * 1024);

#define NUM_PUTC 30000000

vec_uint4 ls_buf[128 / sizeof(vec_uint4)] __attribute__((aligned(128)));

int cellSpursTaskMain(qword argTask, uint64_t argTaskset)
{
    const uint64_t task_id = ((vec_uint4)argTask)[0];
    const uint64_t task_mask = 1 << task_id;
    const uint64_t addr = argTaskset + (task_id * 128);

    for (int index = 0; index < (128 / sizeof(vec_uint4)); index++)
    {
        ls_buf[index] = (vec_uint4)argTask;
    }

    vec_uint4 toinc;
    toinc[0] = toinc[1] = toinc[2] = toinc[3] = 1;

    uint32_t result;

    for (unsigned long counter = 0; counter < NUM_PUTC; counter++)
    {
        cellDmaPut(&ls_buf[counter % ((128 - 32) / sizeof(vec_uint4))], addr, 32, task_id, 0, 0);
        ls_buf[counter % (128 / sizeof(vec_uint4))] = spu_add(ls_buf[counter % (128 / sizeof(vec_uint4))], toinc);
        cellDmaWaitTagStatusAny(task_mask);
    }

    return 0;
}
