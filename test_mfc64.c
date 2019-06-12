#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cell/spurs.h>
#include <cell/atomic.h>
#include "spurs_helpers.h"
#include "globals.h"
#include "shared_defines.h"

extern const CellSpursTaskBinInfo _binary_task_task_mfc64_elf_taskbininfo;

static uint64_t atomic_counter __attribute__((aligned(128)));

void ppu_mfc64_entry(uint64_t arg)
{
	for (unsigned long counter = 0; counter < NUM_INCREMENTS; counter++)
	{
		cellAtomicIncr64(&atomic_counter);
	}

	sys_ppu_thread_exit(0);
}

int test_mfc64(CellSpurs2 *spurs2, unsigned int num_spu, unsigned int num_ppu)
{
	int ret;

	CellSpursTaskset2 *taskset;
	ret = initialize_taskset(spurs2, &taskset, (uint64_t)&atomic_counter);
	if (ret != 0)
	{
		printf("Error initializing taskset: 0x%x\n", ret);
		return ret;
	}

	atomic_counter = 0;

	CellSpursTaskId tids[num_spu];
	CellSpursTaskArgument args[num_spu];
	void *ctx[num_spu];

	for (int index = 0; index < num_spu; index++)
	{
		ctx[index] = memalign(128, _binary_task_task_mfc64_elf_taskbininfo.sizeContext);

		ret = cellSpursCreateTask2WithBinInfo(taskset, &tids[index], &_binary_task_task_mfc64_elf_taskbininfo, &args[index], ctx[index], NULL, NULL);
		if (ret != 0)
		{
			printf("Error cellSpursCreateTask2WithBinInfo: 0x%x\n", ret);
			return ret;
		}
	}

	sys_ppu_thread_t ppu_ids[num_ppu];
	for (int index = 0; index < num_ppu; index++)
	{
		ret = sys_ppu_thread_create(&ppu_ids[index], ppu_mfc64_entry, 0, 1000, 0x1000, SYS_PPU_THREAD_CREATE_JOINABLE, "PPU MFC 64 Bit");
		if (ret != 0)
		{
			printf("Error sys_ppu_thread_create: 0x%x\n", ret);
			return ret;
		}
	}

	uint64_t ppu_exit_code;
	for (int index = 0; index < num_ppu; index++)
	{
		ret = sys_ppu_thread_join(ppu_ids[index], &ppu_exit_code);
		if (ret != 0)
		{
			printf("Error sys_ppu_thread_join: 0x%x\n", ret);
			return ret;
		}
	}

	int exit_code;
	for (int index = 0; index < num_spu; index++)
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
		printf("Atomic counter is %u\n", atomic_counter);
	}

	free_taskset(taskset);

	return 0;
}
