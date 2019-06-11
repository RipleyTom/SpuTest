#pragma once

int initialize_spurs(CellSpurs2 **spurs2);
int initialize_taskset(CellSpurs2* spurs2, CellSpursTaskset2** taskset, uint64_t arg_value);
int free_spurs(CellSpurs2 *spurs2);
int free_taskset(CellSpursTaskset2* taskset);

