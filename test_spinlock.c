#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cell/spurs.h>
#include <cell/atomic.h>
#include "spurs_helpers.h"
#include "globals.h"
#include "shared_defines.h"

#define NUM_SPU 6

extern const CellSpursTaskBinInfo _binary_task_task_spuspinlock_elf_taskbininfo;

static uint64_t atomic_spinlock __attribute__((aligned(128)));

int test_spuspinlock(CellSpurs2 *spurs2)
{
    int ret;

    CellSpursTaskset2 *taskset;
    ret = initialize_taskset(spurs2, &taskset, (uint64_t)&atomic_spinlock);
    if (ret != 0)
    {
        printf("Error initializing taskset: 0x%x\n", ret);
        return ret;
    }

    atomic_spinlock = 0;

    CellSpursTaskId tids[NUM_SPU];
    CellSpursTaskArgument args[NUM_SPU];
    void *ctx[NUM_SPU];

    for (int index = 0; index < NUM_SPU; index++)
    {
        args[index].u32[0] = (index) | ((index + 1) << 4);

        ctx[index] = memalign(128, _binary_task_task_spuspinlock_elf_taskbininfo.sizeContext);

        ret = cellSpursCreateTask2WithBinInfo(taskset, &tids[index], &_binary_task_task_spuspinlock_elf_taskbininfo, &args[index], ctx[index], NULL, NULL);
        if (ret != 0)
        {
            printf("Error cellSpursCreateTask2WithBinInfo: 0x%x\n", ret);
            return ret;
        }
    }

    uint64_t ret_value;
    for (unsigned long index = 0; index < NUM_SPINS; index++)
    {
        do
        {
            ret_value = cellAtomicCompareAndSwap64(&atomic_spinlock, 6, 0);
        } while (ret_value != 6);
    }

    int exit_code;
    for (int index = 0; index < NUM_SPU; index++)
    {
        ret = cellSpursJoinTask2(taskset, tids[index], &exit_code);
        if (ret != 0)
        {
            printf("Error cellSpursJoinTask2: 0x%x\n", ret);
            return ret;
        }
        free(ctx[index]);
    }

    free_taskset(taskset);

    return 0;
}
