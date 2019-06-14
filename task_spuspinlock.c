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
    const unsigned int expected = (uint64_t)(si_to_uint(argTask) & 0xF);
    const unsigned int to_change = (uint64_t)((si_to_uint(argTask) & 0xF0) >> 4);

    uint64_t ret_value;

    for (unsigned long counter = 0; counter < NUM_SPINS; counter++)
    {
        do
        {
            ret_value = cellAtomicCompareAndSwap64(&ls_version, argTaskset, expected, to_change);
        } while (ret_value != expected);
    }

    return 0;
}
