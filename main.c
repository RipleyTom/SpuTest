#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sys_time.h>
#include <cell/spurs.h>
#include "spurs_helpers.h"
#include "test_sha512.h"
#include "test_avalanche.h"
#include "test_pingpong.h"
#include "test_mfc64.h"

#define SHA512_BUF_SIZE        128 * 1024 * 1024

bool verbose = false;

unsigned long long get_time()
{
	sys_time_sec_t secs;
	sys_time_nsec_t nsecs;
	sys_time_get_current_time(&secs, &nsecs);
	return (secs * 1000ull) + (nsecs / 1000000);
}

int main(int argc, char *argv[])
{
	printf("SPU Test v0.5.2 by GalCiv\n");

	unsigned int seed = 12345678;

	for(int index = 1; index < argc; index++)
	{
		if(strcmp(argv[index], "-h") == 0)
		{
			printf("Usage:\n");
			printf("%s <-s seed> <-v>\n", argv[0]);
			printf("-s seed : sets the seed used for random generation\n");
			printf("-v : sets verbose mode on to return the results of tests(hash, final value, etc)\n");
			return 0;
		}

		if(strcmp(argv[index], "-v") == 0)
		{
			printf("Verbose mode on\n");
			verbose = true;
			continue;
		}

		if(strcmp(argv[index], "-s") == 0)
		{
			if((index + 1) >= argc)
			{
				printf("-s need a number for the seed\n");
				return -1;
			}

			index++;

			seed = atoi(argv[index]);
			printf("Using seed %u\n", seed);
			continue;
		}
	}

	srand(seed);


	CellSpurs2 *spurs2;
	int ret = initialize_spurs(&spurs2);
	if (ret != CELL_OK)
	{
		printf("Error initializing SPURS: 0x%x\n", ret);
		return ret;
	}

	unsigned long long time1, time2;

	//////////////////////////////////////////////////////////////////////////////////////

	if(verbose) printf("Test #1: SHA512 on SPU\n");
	time1 = get_time();
	unsigned char *data = memalign(128, SHA512_BUF_SIZE);
	if (!data)
	{
		printf("Failed to allocate memory for data\n");
		return -1;
	}
	for (unsigned int index = 0; index < SHA512_BUF_SIZE; index += sizeof(int))
	{
		*((int *)&data[index]) = rand();
	}
	time2 = get_time();
	printf("[PPU] Generated %uMB of data in %ums(PS3: 778ms)\n", SHA512_BUF_SIZE / (1024 * 1024), (time2 - time1));

	time1 = get_time();
	ret = test_sha512(spurs2, data, SHA512_BUF_SIZE);
	if (ret != 0)
	{
		printf("Error test_sha512: 0x%x\n", ret);
		return ret;
	}
	time2 = get_time();

	printf("[SPU] Finished hashing data in %ums(PS3: 3463ms)\n", (time2 - time1));
	free(data);

	//////////////////////////////////////////////////////////////////////////////////////

	if(verbose) printf("Test #2: SPU Task Avalanche\n");
	time1 = get_time();
	ret = test_avalanche(spurs2);
	if (ret != 0)
	{
		printf("Error test_avalanche: 0x%x\n", ret);
		return ret;
	}
	time2 = get_time();
	printf("[SPU] Avalanche completed in %ums(PS3: 2740ms)\n", (time2 - time1));

	//////////////////////////////////////////////////////////////////////////////////////

	if(verbose) printf("Test #3: SPU/PPU Ping-Pong\n");
	time1 = get_time();
	ret = test_pingpong(spurs2);
	if (ret != 0)
	{
		printf("Error test_pingpong: 0x%x\n", ret);
		return ret;
	}
	time2 = get_time();
	printf("[SPU] SPU/PPU Ping-Pong completed in %ums(PS3: 3045ms)\n", (time2 - time1));

	//////////////////////////////////////////////////////////////////////////////////////

	if(verbose) printf("Test #4: SPU MFC 64 bits war\n");
	time1 = get_time();
	ret = test_mfc64(spurs2, 6, 0);
	if (ret != 0)
	{
		printf("Error test_mfc64(1): 0x%x\n", ret);
		return ret;
	}
	time2 = get_time();
	printf("[SPU] SPU MFC 64 bits war completed in %ums(PS3: 3370ms)\n", (time2 - time1));

	//////////////////////////////////////////////////////////////////////////////////////

	if(verbose) printf("Test #5: PPU/SPU MFC 64 Bits War\n");
	time1 = get_time();
	ret = test_mfc64(spurs2, 6, 2);
	if (ret != 0)
	{
		printf("Error test_mfc64(2): 0x%x\n", ret);
		return ret;
	}
	time2 = get_time();
	printf("[SPU] PPU/SPU MFC 64 Bits War completed in %ums(PS3: 4443ms)\n", (time2 - time1));

	//////////////////////////////////////////////////////////////////////////////////////

	printf("--All tests completed--\n");

	free_spurs(spurs2);

	return 0;
}
