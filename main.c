#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sys_time.h>
#include <cell/spurs.h>
#include "spurs_helpers.h"
#include "test_avalanche.h"
#include "test_pingpong.h"
#include "test_mfc64.h"
#include "test_spu_inst.h"
#include "test_spinlock.h"
#include "test_block.h"
#include "test_largeblock.h"

#define DO_A_TEST(name, function, reference)                                                \
	{                                                                                       \
		testnumber++;                                                                       \
		if (verbose)                                                                        \
			printf("Test #%d: %s\n", testnumber, name);                                     \
		time1 = get_time();                                                                 \
		function;                                                                           \
		if (ret != 0)                                                                       \
		{                                                                                   \
			printf("Error %s: 0x%x\n", name, ret);                                          \
			return ret;                                                                     \
		}                                                                                   \
		time2 = get_time();                                                                 \
		printf("%s completed in %llu ms (PS3: %u ms)\n", name, (time2 - time1), reference); \
	}

#define NUM_TESTS 11

enum tests
{
	TEST_AVALANCHE,
	TEST_PINGPONG,
	TEST_SPU64WAR,
	TEST_PPUSPU64WAR,
	TEST_SPUINTEGER,
	TEST_SPUFLOAT,
	TEST_SPUSPINLOCK,
	TEST_SPUPUTLLUC,
	TEST_SPUPUTLLC,
	TEST_SPUPUT,
	TEST_SPULARGEPUT,
};

#define AVALANCHE_NAME "SPU Task Avalanche"
#define PINGPONG_NAME "PPU/SPU Ping-Pong"
#define SPU64WAR_NAME "SPU MFC 64 Bits War"
#define PPUSPU64WAR_NAME "PPU/SPU MFC 64 Bits War"
#define SPUINTEGER_NAME "SPU Integer Perf"
#define SPUFLOAT_NAME "SPU Float Perf"
#define SPUSPINLOCK_NAME "SPU SpinLock"
#define SPUPUTLLUC_NAME "PUTLLUC Perf"
#define SPUPUTLLC_NAME "PUTLLC Perf"
#define SPUPUT_NAME "PUT Perf"
#define SPULARGEPUT_NAME "Large Put Perf"

typedef struct
{
	char character;
	int test;
	char *test_name;
} arg_test;

const arg_test arg_conv[NUM_TESTS] = {
	{'A', TEST_AVALANCHE, AVALANCHE_NAME},
	{'P', TEST_PINGPONG, PINGPONG_NAME},
	{'S', TEST_SPU64WAR, SPU64WAR_NAME},
	{'W', TEST_PPUSPU64WAR, PPUSPU64WAR_NAME},
	{'I', TEST_SPUINTEGER, SPUINTEGER_NAME},
	{'F', TEST_SPUFLOAT, SPUFLOAT_NAME},
	{'L', TEST_SPUSPINLOCK, SPUSPINLOCK_NAME},
	{'U', TEST_SPUPUTLLUC, SPUPUTLLUC_NAME},
	{'T', TEST_SPUPUTLLC, SPUPUTLLC_NAME},
	{'R', TEST_SPUPUT, SPUPUT_NAME},
	{'G', TEST_SPULARGEPUT, SPULARGEPUT_NAME}};

extern const CellSpursTaskBinInfo _binary_task_task_spuint_elf_taskbininfo;
extern const CellSpursTaskBinInfo _binary_task_task_spufloat_elf_taskbininfo;

extern const CellSpursTaskBinInfo _binary_task_task_putlluc_elf_taskbininfo;
extern const CellSpursTaskBinInfo _binary_task_task_putllc_elf_taskbininfo;
extern const CellSpursTaskBinInfo _binary_task_task_put_elf_taskbininfo;
extern const CellSpursTaskBinInfo _binary_task_task_largeput_elf_taskbininfo;

bool verbose = false;

uint64_t get_time()
{
	sys_time_sec_t secs;
	sys_time_nsec_t nsecs;
	sys_time_get_current_time(&secs, &nsecs);
	return (secs * 1000ull) + (nsecs / 1000000);
}

int main(int argc, char *argv[])
{
	printf("SPU Test v1.1.1 by GalCiv\n");

	unsigned int seed = 12345678;
	unsigned int repeat = 1;
	bool tests_to_run[NUM_TESTS];

	for (int index = 0; index < NUM_TESTS; index++)
		tests_to_run[index] = true;

	for (int index = 1; index < argc; index++)
	{
		if (strcmp(argv[index], "h") == 0)
		{
			printf("Usage:\n");
			printf("spu_test <s seed> <r number> <t tests> <v> <h>\n");
			printf("s seed   : sets the seed used for random generation\n");
			printf("r number : repeats the test <number> times\n");
			printf("t tests  : only include specified tests(see list below)\n");
			printf("v        : sets verbose mode on to return the results of tests(hash, final value, etc)\n");
			printf("h        : shows help\n");
			printf("----\n");
			printf("List of tests:\n");
			for (int subdex = 0; subdex < NUM_TESTS; subdex++)
				printf("%c %s\n", arg_conv[subdex].character, arg_conv[subdex].test_name);
			return 0;
		}

		if (strcmp(argv[index], "t") == 0)
		{
			if ((index + 1) >= argc)
			{
				printf("t needs an argument describing the tests to use\n");
				return -1;
			}
			index++;
			for (int subdex = 0; subdex < NUM_TESTS; subdex++)
			{
				if (strchr(argv[index], arg_conv[subdex].character))
					tests_to_run[subdex] = true;
				else
					tests_to_run[subdex] = false;
			}
		}

		if (strcmp(argv[index], "v") == 0)
		{
			printf("Verbose mode on\n");
			verbose = true;
			continue;
		}

		if (strcmp(argv[index], "s") == 0)
		{
			if ((index + 1) >= argc)
			{
				printf("s needs a seed value\n");
				return -1;
			}
			index++;
			seed = (unsigned int)strtoul(argv[index], NULL, 10);
			if (seed == 0L)
			{
				printf("%s is not a valid seed\n", argv[index]);
				return -1;
			}
			printf("Using seed %u\n", seed);
			continue;
		}

		if (strcmp(argv[index], "r") == 0)
		{
			if ((index + 1) >= argc)
			{
				printf("r needs a repeat value\n");
				return -1;
			}
			index++;
			repeat = (unsigned int)strtoul(argv[index], NULL, 10);
			if (seed == 0L)
			{
				printf("%s is not a valid repeat value\n", argv[index]);
				return -1;
			}
			printf("Repeating test loop %u times\n", repeat);
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

	uint64_t time1, time2, timestart, timeend;
	uint32_t testnumber = 0;

	timestart = get_time();

	for (unsigned int index = 0; index < repeat; index++)
	{
		if (tests_to_run[TEST_AVALANCHE])
			DO_A_TEST(AVALANCHE_NAME, test_avalanche(spurs2), 2740);
		if (tests_to_run[TEST_PINGPONG])
			DO_A_TEST(PINGPONG_NAME, test_pingpong(spurs2), 3045);
		if (tests_to_run[TEST_SPU64WAR])
			DO_A_TEST(SPU64WAR_NAME, test_mfc64(spurs2, 6, 0), 3370);
		if (tests_to_run[TEST_PPUSPU64WAR])
			DO_A_TEST(PPUSPU64WAR_NAME, test_mfc64(spurs2, 6, 2), 4443);
		if (tests_to_run[TEST_SPUINTEGER])
			DO_A_TEST(SPUINTEGER_NAME, test_spu_inst(spurs2, &_binary_task_task_spuint_elf_taskbininfo), 8666);
		if (tests_to_run[TEST_SPUFLOAT])
			DO_A_TEST(SPUFLOAT_NAME, test_spu_inst(spurs2, &_binary_task_task_spufloat_elf_taskbininfo), 2379);
		if (tests_to_run[TEST_SPUSPINLOCK])
			DO_A_TEST(SPUSPINLOCK_NAME, test_spuspinlock(spurs2), 4409);
		if (tests_to_run[TEST_SPUPUTLLUC])
			DO_A_TEST(SPUPUTLLUC_NAME, test_block(spurs2, &_binary_task_task_putlluc_elf_taskbininfo), 3853);
		if (tests_to_run[TEST_SPUPUTLLC])
			DO_A_TEST(SPUPUTLLC_NAME, test_block(spurs2, &_binary_task_task_putllc_elf_taskbininfo), 3364);
		if (tests_to_run[TEST_SPUPUT])
			DO_A_TEST(SPUPUT_NAME, test_block(spurs2, &_binary_task_task_put_elf_taskbininfo), 3984);
		if (tests_to_run[TEST_SPULARGEPUT])
			DO_A_TEST(SPULARGEPUT_NAME, test_largeblock(spurs2, &_binary_task_task_largeput_elf_taskbininfo), 0);
	}

	timeend = get_time();

	printf("--Completed %d tests in %u ms--\n", testnumber, timeend - timestart);

	free_spurs(spurs2);

	return 0;
}
