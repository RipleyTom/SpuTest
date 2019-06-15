#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cell/spurs.h>
#include <cell/atomic.h>
#include "spurs_helpers.h"
#include "globals.h"

#define NUM_SPU 6

unsigned char bigbuf[128] __attribute__((aligned(128)));

int test_block(CellSpurs2 *spurs2, const CellSpursTaskBinInfo *program)
{
    int ret;

    CellSpursTaskset2 *taskset;
    ret = initialize_taskset(spurs2, &taskset, (uint64_t)bigbuf);
    if (ret != 0)
    {
        printf("Error initializing taskset: 0x%x\n", ret);
        return ret;
    }

    CellSpursTaskId tids[NUM_SPU];
    CellSpursTaskArgument args[NUM_SPU];
    void *ctx[NUM_SPU];

    for (int index = 0; index < NUM_SPU; index++)
    {
        ctx[index] = memalign(128, program->sizeContext);
        for (int subdex = 0; subdex < 4; subdex++)
        {
            args[index].u32[subdex] = (index * 10000000) + subdex;
        }

        ret = cellSpursCreateTask2WithBinInfo(taskset, &tids[index], program, &args[index], ctx[index], NULL, NULL);
        if (ret != 0)
        {
            printf("Error cellSpursCreateTask2WithBinInfo: 0x%x\n", ret);
            return ret;
        }
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

    if (verbose)
    {
        printf("BigBuf:");
        for (int index = 0; index < 128; index++)
        {
            printf("%02X", bigbuf[index]);
            if ((index % 16) == 0)
                printf("\n");
        }
    }

    free_taskset(taskset);

    return 0;
}
