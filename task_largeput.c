#include <stdint.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>
#include <cell/dma.h>

CELL_SPU_LS_PARAM(16 * 1024, 16 * 1024);

#define NUM_LARGE_PUTC 10000000

uint8_t ls_buf[16384] __attribute__((aligned(128)));

int cellSpursTaskMain(qword argTask, uint64_t argTaskset)
{
    const uint64_t addr = argTaskset;

    for (unsigned long counter = 0; counter < NUM_LARGE_PUTC; counter++)
    {
        cellDmaPut(ls_buf, addr, 16384, 0, 0, 0);
    }

    return 0;
}
