#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cell/spurs.h>
#include <cell/hash/libsha512SPURS.h>
#include "spurs_helpers.h"
#include "globals.h"

#define BUF_SIZE		       16*1024
#define MAX_BUFS		       4
#define SHA_QDEPTH             8

static unsigned char *g_buf[MAX_BUFS];
unsigned char result[CELL_SHA512_DIGEST_SIZE] __attribute__((__aligned__(16)));
CellSha512SpuInstance inst;

int test_sha512(CellSpurs2* spurs2, unsigned char *data, unsigned long size)
{
	int ret;
	uint64_t  nread;
	CellSpursTaskset2 *taskset;

	initialize_taskset(spurs2, &taskset, 0);

	int read_buf = 0, i;
	int job_active[MAX_BUFS];

	for (i = 0; i < MAX_BUFS; i++)
	{
		job_active[i] = 0;
		g_buf[i] = memalign(16, BUF_SIZE);
	}


	ret = cellSha512SpuCreateTask2(&inst, taskset, NULL, NULL, SHA_QDEPTH, BUF_SIZE);
	if (ret < 0)
	{
		printf("Error cellSha512SpuCreateTask2: 0x%x\n", ret);
		return ret;
	}

	long sub_data = 0;

	do
	{
		if (job_active[read_buf])
		{
			ret = cellSha512SpuCheckStatus(&inst, 1 << read_buf, 1);
		}

		nread = (size - (sub_data * BUF_SIZE)) > BUF_SIZE ? BUF_SIZE : size - (sub_data * BUF_SIZE);
		memcpy(g_buf[read_buf], data + (sub_data * BUF_SIZE), nread);

		sub_data++;

		ret = cellSha512SpuProcessData(&inst, (uint64_t)((int)g_buf[read_buf]), nread, 1 << read_buf, 1);
		job_active[read_buf] = 1;

		read_buf = (read_buf + 1) % MAX_BUFS;
	} while (nread == BUF_SIZE);

	ret = cellSha512SpuEndTask2(&inst, (uint64_t)result);

	if (verbose)
	{
		printf("Hash result is: ");
		for (i = 0; i < CELL_SHA512_DIGEST_SIZE; i++)
		{
			printf("%02X", result[i]);
		}
		printf("\n");
	}

	for (i = 0; i < MAX_BUFS; i++)
	{
		if (g_buf[i] != NULL)
			free(g_buf[i]);
	}

	free_taskset(taskset);

	return 0;
}
