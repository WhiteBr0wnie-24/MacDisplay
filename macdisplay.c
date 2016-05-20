/*
* Copyright (C) 2016, Daniel Herczeg
*
* C-Host-Programm fuer den Macintosh Classic CRT-Monitor
* Dieses Programm speichert jeweils ein komplettes Frame
* in den DDR-Ram des BBB und verstaendigt das ausgefuehrte
* PRU-Programm ueber das Vorhandensein eines neuen Frames,
* welches gezeichnet werden kann.
*
* Kompilation mithilfe von: gcc [INPUT_FILE].c -lprussdrv -lpthread -o [OUTPUT_FILE]
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 3 as
* published by the Free Software Foundation.
*
* It can be viewed here: http://www.gnu.org/licenses/gpl-3.0.de.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include "testimage.xbm"

#define DISP_WIDTH				512
#define DISP_HEIGHT				342
#define VRAM_SIZE				(DISP_WIDTH*DISP_HEIGHT/8)

#define DDR_BASE_ADDR			0x80000000
#define DDR_OFFSET				0x00001000

typedef int bool;
#define true 1
#define false 0

static int mem_dev;
static void *vram;
static bool ended = false;

static bool initVRAM(void);
static void writeFrameToVRAM(void);
static void writeLineToVRAM(int);
static int cleanUp(void);
int main(void);

static bool initVRAM()
{
	printf("Opening /dev/mem ... ");
	
	mem_dev = open("/dev/mem", O_RDWR);
	
	if(mem_dev < 0)
	{
		printf("Error opening memory.\n");
		return false;
	}
	else
		printf("OK.\n");
	
	printf("Attempting to map memory ... ");
	
	vram = mmap(0, VRAM_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, mem_dev, DDR_BASE_ADDR);
	
	printf("OK.\n");
	
	if(vram == NULL)
	{
		printf("Error mapping memory.\n");
		close(mem_dev);
		return false;
	}
	
	printf("Testing VRAM-Size: %i kBytes ... ", VRAM_SIZE/1024);
	printf("OK.\n");
	
	printf("Writing testimage.xbm to vram (%p)... ", vram);

	// writeFrameToVRAM();
	writeLineToVRAM(0);
	
	printf("OK.\n");
	
	return true;
}

/**
static void writeFrameToVRAM()
{
	int o = 0;
	
	for(; o < DISP_HEIGHT; o++)
	{
		writeLineToVRAM(o);
	}
	
	// TODO: Notify PRU program that a new frame is available
}
**/

// 1 Zeile hat 64 Byte
static void writeLineToVRAM(int line)
{
	// DEBUG
	// printf("\n----- Line %i (%i Bits) -----\n", line, DISP_WIDTH);
	
	int i = 0;
	int lineLength = DISP_WIDTH / 8;
	int regBytes = 4;
	int lineOffset = line * lineLength;
	int regs = lineLength / regBytes;
	
	for(; i < regs; i++)
	{
		int regOffset = 0x00000004 * i;
		void *DDR_regaddr = vram + DDR_OFFSET + regOffset;
		char registerValue[4];
		
		int u=0;
		for(; u < regBytes; u++)
		{
			char currByte = test_image[u+i*regBytes+line*lineLength];
			registerValue[u]=currByte;
		}
		
		//printf("%p\n", DDR_regaddr);
		// printf("%p\n", *(unsigned long*)registerValue);
		*(unsigned long*) DDR_regaddr = *(unsigned long*)registerValue;
	}
	
	// printf("-----------------------------\n");
}

static int cleanUp(void)
{
	prussdrv_pru_disable(0);
	prussdrv_pru_disable(1);
	prussdrv_exit ();
	
    munmap(vram, VRAM_SIZE);
    close(mem_dev);
	
	return 0;
}

int main (void)
{
	// Initialize structure used by prussdrv_pruintc_intc
	// PRUSS_INTC_INITDATA is found in pruss_intc_mapping.h
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	
	printf("Initialize PRU_0_1_MEM and Host-Controller ... ");
	// Allocate and initialize memory
	prussdrv_init ();
	prussdrv_open (PRU_EVTOUT_0);
	// prussdrv_open (PRU_EVTOUT_1);

	// Map PRU's interrupts
	prussdrv_pruintc_init(&pruss_intc_initdata);
	printf("OK\n");
	
	// VRAM initialisieren
	if(!initVRAM())
		return 2;
	
	printf("Flashing PRU_0 Program ... ");
	
	// Load and execute the PRU program on the PRU
	prussdrv_exec_program (0, "./pru_0.bin");
	
	printf("OK\n");
	/**
	printf("Flashing PRU_1 Program ... ");
	
	// Load and execute the PRU program on the PRU
	prussdrv_exec_program (1, "./pru_1.bin");
	
	printf("OK\n");
	**/
	
	prussdrv_pru_wait_event (PRU_EVTOUT_0);
	// prussdrv_pru_wait_event (PRU_EVTOUT_1);
	
	printf("PRU program halted\n");
	
	/**
	while(!ended)
	{
		// PRU_1 sendet ein event, sobald das VSYNC erreicht wurde
		// in dieser Zeit soll der Host ein neues Frame in den
		// VRAM schreiben, damit dieses nach dem VSYNC gezeichnet
		// werden kann.
		// Wait for event completion from PRU, returns PRU_EVTOUT_0 number
		// prussdrv_pru_clear_event(PRU_EVTOUT_0, 32);
		int n = prussdrv_pru_wait_event (PRU_EVTOUT_1);
		writeFrameToVRAM();
		printf("Frame %d finished.\n", n);
	}
	**/
	
	// Disable PRU and close memory mappings
	return cleanUp();
}