#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/spu_initialize.h>
#include <cell/spurs.h>


#define NUM_OF_SPU             6
#define SPU_SPURS_PRIORITY     100
#define PPU_SPURS_PRIORITY     500

int initialize_spurs(CellSpurs2 **spurs2)
{
	int ret = cellSysmoduleInitialize();
	if (ret != CELL_OK)
	{
		printf("Errir cellSysmoduleInitialize: 0x%x\n", ret);
		return ret;
	}

	ret = sys_spu_initialize(NUM_OF_SPU, 0);
	if (ret != CELL_OK)
	{
		printf("Error sys_spu_initialize: 0x%x\n", ret);
		return ret;
	}

	*spurs2 = (CellSpurs2*)memalign(128, sizeof(CellSpurs2));
	if (!(*spurs2))
	{
		printf("Failed to allocate memory for CellSpurs2\n");
		return -1;
	}

	CellSpursAttribute attributeSpurs;
	ret = cellSpursAttributeInitialize(&attributeSpurs, NUM_OF_SPU, SPU_SPURS_PRIORITY, PPU_SPURS_PRIORITY, false);
	if (ret != CELL_OK)
	{
		printf("Error cellSpursAttributeInitialize: 0x%x\n", ret);
		return ret;
	}

	ret = cellSpursInitializeWithAttribute2(*spurs2, &attributeSpurs);
	if (ret != CELL_OK)
	{
		printf("Error cellSpursInitializeWithAttribute: 0x%x\n", ret);
		return ret;
	}

	return 0;
}

int initialize_taskset(CellSpurs2* spurs2, CellSpursTaskset2** taskset, uint64_t arg_value) 
{
	*taskset = (CellSpursTaskset2*)memalign(128, sizeof(CellSpursTaskset2));

	if (!(*taskset))
	{
		printf("Failed to allocate memory for taskset\n");
		return -1;
	}

	CellSpursTasksetAttribute2 attributeTaskset;
	cellSpursTasksetAttribute2Initialize(&attributeTaskset);

	attributeTaskset.name = "TEST_TASKSET";
	attributeTaskset.argTaskset = arg_value;
	const uint8_t priorities[8] = {1, 1, 1, 1, 1, 1, 1, 1};
	memcpy(attributeTaskset.priority, priorities, sizeof(priorities));
	attributeTaskset.maxContention = NUM_OF_SPU;

	return cellSpursCreateTaskset2((CellSpurs *)spurs2, *taskset, &attributeTaskset);
}

int free_spurs(CellSpurs2 *spurs2)
{
	int ret = cellSpursFinalize((CellSpurs *)spurs2);
	if (ret != 0)
	{
		printf("Error cellSpursFinalize: 0x%x\n", ret);
	}
	free(spurs2);
}

int free_taskset(CellSpursTaskset2* taskset)
{
	int ret = cellSpursDestroyTaskset2(taskset);
	if (ret != 0)
	{
		printf("Error cellSpursDestroyTaskset2: 0x%x\n", ret);
	}
	free(taskset);
}
