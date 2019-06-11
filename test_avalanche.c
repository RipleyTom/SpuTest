#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cell/spurs.h>
#include "spurs_helpers.h"
#include "globals.h"

#define NUM_AVALANCHE          1024
#define NUM_PROGRAMS           1024
#define NUM_AVALANCHE_TASKSETS 16
#define NUM_TASK_PER_TASKSET   64

extern const CellSpursTaskBinInfo _binary_task_task_elf_taskbininfo;

int test_avalanche(CellSpurs2* spurs2)
{
	int ret;
	CellSpursTaskArgument arg[NUM_PROGRAMS];
	unsigned char *spu_data[NUM_PROGRAMS];

	CellSpursTaskset2 *tasksets[NUM_AVALANCHE_TASKSETS];

	CellSpursTaskId tid[NUM_AVALANCHE_TASKSETS][NUM_TASK_PER_TASKSET];

	// Init tasksets
	for (int index = 0; index < NUM_AVALANCHE_TASKSETS; index++)
	{
		ret = initialize_taskset(spurs2, &tasksets[index], 0);
		if (ret != 0)
		{
			printf("Error initializing taskset: 0x%x\n", ret);
			return ret;
		}
	}

	unsigned char *spu_src = (unsigned char *)((uint32_t)_binary_task_task_elf_taskbininfo.eaElf);
	unsigned long spu_size = (uint32_t)&_binary_task_task_elf_taskbininfo.eaElf - (uint32_t)spu_src;

	// Dynamically finds value to patch
	bool found = false;
	int offset_patch = 0;
	for(; offset_patch < spu_size; offset_patch+=4)
	{
		if(*((uint32_t *)&spu_src[offset_patch]) == 0x12345678)
		{
			found = true;
			break;
		}
	}

	if(found==false)
	{
		printf("Failed to find value to patch in task.c object(expecting to find 0x12345678)\n");
		return -1;
	}

	// Creates NUM_PROGRAMS programs
	for (int index = 0; index < NUM_PROGRAMS; index++)
	{
		spu_data[index] = memalign(128, spu_size);
		memcpy(spu_data[index], spu_src, spu_size);
		*((int *)(spu_data[index] + offset_patch)) = index;
		arg[index].u32[0] = (uint32_t)rand();
	}

	int avalanche_result = 0;
	unsigned int ind_program = 0;
	int exitCode;

	for (int index = 0; index < NUM_AVALANCHE; index++)
	{
		// One avalanche consist of NUM_TASK_PER_TASKSET * NUM_AVALANCHE_TASKSETS tasks
		for(int setind = 0; setind < NUM_AVALANCHE_TASKSETS; setind++)
		{
			// Join previous tasks if it's not the first avalanche
			if(index!=0)
			{
				for(int taskind = 0; taskind < NUM_TASK_PER_TASKSET; taskind++)
				{
					ret = cellSpursJoinTask2(tasksets[setind], tid[setind][taskind], &exitCode);
					if (ret != 0)
					{
						printf("Error cellSpursJoinTask2: 0x%x\n", ret);
						return ret;
					}
					avalanche_result += exitCode;
				}
			}

			for(int taskind = 0; taskind < NUM_TASK_PER_TASKSET; taskind++)
			{
				ret = cellSpursCreateTask2(tasksets[setind], &tid[setind][taskind], spu_data[ind_program], &arg[ind_program], NULL);
				if (ret != 0)
				{
					printf("Error cellSpursCreateTask2: 0x%x\n", ret);
					return ret;
				}
				ind_program = (ind_program + 1) % NUM_PROGRAMS;
			}
		}
	}

	for(int setind = 0; setind < NUM_AVALANCHE_TASKSETS; setind++)
	{
		for(int taskind = 0; taskind < NUM_TASK_PER_TASKSET; taskind++)
		{
			ret = cellSpursJoinTask2(tasksets[setind], tid[setind][taskind], &exitCode);
			if (ret != 0)
			{
				printf("Error cellSpursJoinTask2: 0x%x\n", ret);
				return ret;
			}
			avalanche_result += exitCode;
		}
	}

	for (int index = 0; index < NUM_AVALANCHE_TASKSETS; index++)
	{
		ret = free_taskset(tasksets[index]);
		if (ret != 0)
		{
			printf("Error freeing taskset: 0x%x\n", ret);
			return ret;
		}
	}

	for(int index = 0; index < NUM_PROGRAMS; index++)
	{
		free(spu_data[index]);
	}

	if(verbose)
	{
		printf("Avalanche result: %u\n", avalanche_result);
	}

	return 0;
}
