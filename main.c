#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sys_time.h>
#include <cell/spurs.h>
#include "spurs_helpers.h"
#include "test_avalanche.h"
#include "test_pingpong.h"
#include "test_mfc64.h"

#define SHA512_BUF_SIZE        128 * 1024 * 1024

bool verbose = false;

#define DO_A_TEST(name, function, reference) \
	if(verbose) printf("Test #%d: %s\n", ++testnumber, name); \
	time1 = get_time(); \
	function; \
	if (ret != 0) \
	{ \
		printf("Error %s: 0x%x\n", name, ret); \
		return ret; \
	} \
	time2 = get_time(); \
	printf("%s completed in %u ms(PS3: %u ms)\n", name, (time2 - time1), reference)


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

	unsigned long long time1, time2, timestart, timeend, testnumber = 0;

	timestart = get_time();

	DO_A_TEST("SPU Task Avalanche", test_avalanche(spurs2), 2740);
	DO_A_TEST("PPU/SPU Ping-Pong", test_pingpong(spurs2), 3045);
	DO_A_TEST("SPU MFC 64 Bits War", test_mfc64(spurs2, 6, 0), 3370);
	DO_A_TEST("PPU/SPU MFC 64 Bits War", test_mfc64(spurs2, 6, 2), 4443);

	timeend = get_time();

	printf("--Completed %d tests in %u ms--\n", testnumber, timeend - timestart);

	free_spurs(spurs2);

	return 0;
}
