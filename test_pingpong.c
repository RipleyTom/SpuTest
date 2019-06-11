#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cell/spurs.h>
#include "spurs_helpers.h"
#include "shared_defines.h"

#define NUM_SPU_THREADS 6

extern const CellSpursTaskBinInfo _binary_task_task_pingpong_elf_taskbininfo;

static CellSpursEventFlag event_flag __attribute__ ((aligned(CELL_SPURS_EVENT_FLAG_ALIGN)));

int test_pingpong(CellSpurs2* spurs2)
{
	int ret;

	CellSpursTaskset2 *taskset;
	ret = initialize_taskset(spurs2, &taskset, (uint64_t)&event_flag);
	if (ret != 0)
	{
		printf("Error initializing taskset: 0x%x\n", ret);
		return ret;
	}

	ret = cellSpursEventFlagInitialize((CellSpursTaskset *)taskset, &event_flag, CELL_SPURS_EVENT_FLAG_CLEAR_MANUAL, CELL_SPURS_EVENT_FLAG_ANY2ANY);
	if (ret != 0)
	{
		printf("Error cellSpursEventFlagInitialize: 0x%x\n", ret);
		return ret;
	}

	CellSpursTaskId tids[NUM_SPU_THREADS];
	CellSpursTaskArgument args[NUM_SPU_THREADS];
	void *ctx[NUM_SPU_THREADS];

	for (int index = 0; index < NUM_SPU_THREADS; index++)
	{
		args[index].u32[0] = (1 << index);
		ctx[index] = memalign(128, _binary_task_task_pingpong_elf_taskbininfo.sizeContext);

		ret = cellSpursCreateTask2WithBinInfo(taskset, &tids[index], &_binary_task_task_pingpong_elf_taskbininfo, &args[index], ctx[index], NULL, NULL);
		if (ret != 0)
		{
			printf("Error cellSpursCreateTask2WithBinInfo: 0x%x\n", ret);
			return ret;
		}
	}

	ret = cellSpursEventFlagAttachLv2EventQueue(&event_flag);
	if (ret != CELL_OK)
	{
		printf("Error cellSpursEventFlagAttachLv2EventQueue: 0x%x\n", ret);
		return ret;
	}

	// Ping? Pong!
	const uint16_t ppu_mask = (1 << NUM_SPU_THREADS);
	uint16_t mask;

	for (int index = 0; index < NUM_PONGS; index++)
	{
		mask = ppu_mask;
		ret = cellSpursEventFlagSet(&event_flag, 1);
		if (ret != CELL_OK)
		{
			printf("Error cellSpursEventFlagSet: 0x%x\n", ret);
			return ret;
		}
		ret = cellSpursEventFlagWait(&event_flag, &mask, CELL_SPURS_EVENT_FLAG_AND);
		if (ret != CELL_OK)
		{
			printf("Error cellSpursEventFlagWait: 0x%x\n", ret);
			return ret;
		}

		//printf("PING %u, mask %d!\n", index, mask);

		ret = cellSpursEventFlagClear(&event_flag, 0xFFFF);
		if (ret != CELL_OK)
		{
			printf("Error cellSpursEventFlagClear: 0x%x\n", ret);
			return ret;
		}
	}

	ret = cellSpursEventFlagDetachLv2EventQueue(&event_flag);
	if (ret != CELL_OK)
	{
		printf("Error cellSpursEventFlagDetachLv2EventQueue: 0x%x\n", ret);
		return ret;
	}

	int exitCode;

	for (int index = 0; index < NUM_SPU_THREADS; index++)
	{
		ret = cellSpursJoinTask2(taskset, tids[index], &exitCode);
		if (ret != 0)
		{
			printf("Error cellSpursJoinTask2: 0x%x\n", ret);
			return ret;
		}
		free(ctx[index]);
	}

	free_taskset(taskset);
}
