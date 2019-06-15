@echo off
spu-lv2-gcc -O3 -c task.c -o task.o
spu-lv2-gcc -O3 -c task_pingpong.c -o task_pingpong.o
spu-lv2-gcc -O3 -c task_mfc64.c -o task_mfc64.o
spu-lv2-gcc -O3 -c task_spuint.c -o task_spuint.o
spu-lv2-gcc -O3 -c task_spufloat.c -o task_spufloat.o
spu-lv2-gcc -O3 -c task_spuspinlock.c -o task_spuspinlock.o
spu-lv2-gcc -O3 -c task_putlluc.c -o task_putlluc.o
spu-lv2-gcc -O3 -c task_putllc.c -o task_putllc.o

spu-lv2-gcc -O3 -mspurs-task task.o -o task.elf
spu-lv2-gcc -O3 -mspurs-task task_pingpong.o -o task_pingpong.elf
spu-lv2-gcc -O3 -mspurs-task task_mfc64.o -latomic -o task_mfc64.elf
spu-lv2-gcc -O3 -mspurs-task task_spuint.o -o task_spuint.elf
spu-lv2-gcc -O3 -mspurs-task task_spufloat.o -o task_spufloat.elf
spu-lv2-gcc -O3 -mspurs-task task_spuspinlock.o -latomic -o task_spuspinlock.elf
spu-lv2-gcc -O3 -mspurs-task task_putlluc.o -ldma -o task_putlluc.elf
spu-lv2-gcc -O3 -mspurs-task task_putllc.o -ldma -o task_putllc.elf

spu_elf-to-ppu_obj task.elf task.ppu.o
spu_elf-to-ppu_obj task_pingpong.elf task_pingpong.ppu.o
spu_elf-to-ppu_obj task_mfc64.elf task_mfc64.ppu.o
spu_elf-to-ppu_obj task_spuint.elf task_spuint.ppu.o
spu_elf-to-ppu_obj task_spufloat.elf task_spufloat.ppu.o
spu_elf-to-ppu_obj task_spuspinlock.elf task_spuspinlock.ppu.o
spu_elf-to-ppu_obj task_putlluc.elf task_putlluc.ppu.o
spu_elf-to-ppu_obj task_putllc.elf task_putllc.ppu.o

ppu-lv2-gcc -O3 -c spurs_helpers.c -o spurs_helpers.o
ppu-lv2-gcc -O3 -c test_avalanche.c -o test_avalanche.o
ppu-lv2-gcc -O3 -c test_pingpong.c -o test_pingpong.o
ppu-lv2-gcc -O3 -c test_mfc64.c -o test_mfc64.o
ppu-lv2-gcc -O3 -c test_spu_inst.c -o test_spu_inst.o
ppu-lv2-gcc -O3 -c test_spinlock.c -o test_spinlock.o
ppu-lv2-gcc -O3 -c test_block.c -o test_block.o
ppu-lv2-gcc -O3 -c main.c -o main.o


ppu-lv2-gcc -O3 spurs_helpers.o test_avalanche.o test_pingpong.o test_mfc64.o test_spu_inst.o test_spinlock.o test_block.o main.o task.ppu.o task_pingpong.ppu.o task_mfc64.ppu.o task_spuint.ppu.o task_spufloat.ppu.o task_spuspinlock.ppu.o task_putlluc.ppu.o task_putllc.ppu.o -lsysmodule_stub -lspurs_stub -o spurs_test.elf
make_fself spurs_test.elf spurs_test.self
