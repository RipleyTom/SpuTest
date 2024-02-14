#include <stdint.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>
#include <cell/dma.h>

CELL_SPU_LS_PARAM(16 * 1024, 16 * 1024);

#define NUM_LARGE_PUTC 1000000

uint8_t ls_buf[16384] __attribute__((aligned(128)));

int cellSpursTaskMain(qword argTask, uint64_t argTaskset)
{
    const uint64_t task_id = ((vec_uint4)argTask)[0];
    const uint64_t task_mask = 1 << task_id;
    const uint64_t addr = argTaskset + (task_id * 16384);

    for (int index = 0; index < 16384; index++)
    {
        ls_buf[index] = (index + task_id) % 0xFF;
    }

    for (unsigned long counter = 0; counter < NUM_LARGE_PUTC; counter++)
    {
        cellDmaPut(ls_buf, addr, 16384, task_id, 0, 0);
        ls_buf[counter % 16384]++;
        cellDmaWaitTagStatusAny(task_mask);
    }

    return 0;
}
