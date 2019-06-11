@echo off
spu-lv2-gcc -O3 -c task.c -o task.o
spu-lv2-gcc -O3 -c task_pingpong.c -o task_pingpong.o
spu-lv2-gcc -O3 -c task_mfc64.c -o task_mfc64.o
spu-lv2-gcc -O3 -mspurs-task task.o -o task.elf
spu-lv2-gcc -O3 -mspurs-task task_pingpong.o -o task_pingpong.elf
spu-lv2-gcc -O3 -mspurs-task task_mfc64.o -latomic -o task_mfc64.elf
spu_elf-to-ppu_obj task.elf task.ppu.o
spu_elf-to-ppu_obj task_pingpong.elf task_pingpong.ppu.o
spu_elf-to-ppu_obj task_mfc64.elf task_mfc64.ppu.o

ppu-lv2-gcc -O3 spurs_helpers.c test_sha512.c test_avalanche.c test_pingpong.c test_mfc64.c main.c task.ppu.o task_pingpong.ppu.o task_mfc64.ppu.o -LE:\usr\local\cell\target\ppu\lib\hash\ -lsysmodule_stub -lspurs_stub -lsha512 -lsha512SPURS -o spurs_test.elf
make_fself spurs_test.elf spurs_test.self
