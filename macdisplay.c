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
* Compile with: gcc [INPUT_FILE].c -lprussdrv -lpthread -o [OUTPUT_FILE]
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
#include <signal.h>

#include "cat1.xbm"

#define PRU_NUM					0
#define DISP_WIDTH				512
#define DISP_HEIGHT				342
#define INVERT_COLORS			0

#define DDR_BASE_ADDR			0x80000000
#define DDR_OFFSET				0x00001000
#define SRAM_OFFSET				2048
#define PRUSS0_SHARED_DATARAM	4
#define VRAM_SIZE				(DISP_WIDTH*DISP_HEIGHT)/8

#define LINE_REGISTERS			16
#define REGISTER_BYTES			4
#define	LINEBYTES				LINE_REGISTERS*REGISTER_BYTES

typedef int bool;
#define true 1
#define false 0

int mem_dev;
float cnt = 0;
void *vram;
volatile bool running = true;

// prototypes
void writeFrameToVRAM(void);

void ctrlCHandler(int dummy)
{
    running = false;
}

static bool initVRAM(void)
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
	
	vram = mmap(0, VRAM_SIZE*2, PROT_WRITE | PROT_READ, MAP_SHARED, mem_dev, DDR_BASE_ADDR);
	
	printf("OK.\n");
	
	if(vram == NULL)
	{
		printf("Error mapping memory.\n");
		close(mem_dev);
		return false;
	}
	
	printf("Testing VRAM-Size: %i Bytes ... ", VRAM_SIZE);
	printf("OK.\n");
	
	printf("Writing testimage.xbm to vram (%p)... ", vram+DDR_OFFSET);
	
	// Write the initial frame to the vram
	writeFrameToVRAM();
	
	printf("OK.\n");
	
	return true;
}

void writeFrameToVRAM(void)
{
	int i = 0;
	
	for(; i < DISP_HEIGHT; i++)
	{
		// LINE_REGISTERS should be 16 (for the mac classics display)
		// 16 mal 4 byte
		// 16 mal 4*8 bit = 512 -> eine komplette linie
		int u = 0;
		volatile unsigned long *data_addr = vram + DDR_OFFSET + LINEBYTES * i;
		volatile char lineData[64];
		int byteCounter = 0;
		
		// Read one line of pixel data from the cat image
		// 64 byte * 8 = 512 bit = 512 pixels = x_resolution of the macdisplay
		int o = 0;
		for(; o < 64; o++)
		{
			lineData[o] = test_image[i*64+o];
		}
		
		// Split one line into 16 registers
		for(; u < LINE_REGISTERS; u++)
		{
			volatile char registerBytes[REGISTER_BYTES];
			int c = 0;
			
			// die einzelnen 4 byte aus den bilddaten zu einem der 16
			// register zusammenfuegen
			for(; c < REGISTER_BYTES; c++)
			{
				volatile char currByte;
				
				if(INVERT_COLORS == 1)
					currByte = ~lineData[byteCounter];
				else
					currByte = lineData[byteCounter];

				registerBytes[c] = currByte;
				byteCounter++;
			}
			
			
			volatile unsigned long regData = 0l;
			
			regData += registerBytes[0] << 24;
			regData += registerBytes[1] << 16;
			regData += registerBytes[2] << 8;
			regData += registerBytes[3];
			
			*data_addr = regData;
			data_addr++;
		}
	}
}

int main (void)
{
	signal(SIGINT, ctrlCHandler);
	
	// Initialize structure used by prussdrv_pruintc_intc
	// PRUSS_INTC_INITDATA is found in pruss_intc_mapping.h
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	
	printf("Initialize PRU MEM ... ");
	
	// Allocate and initialize memory
	prussdrv_init ();
	prussdrv_open (PRU_EVTOUT_0);

	// Map PRU's interrupts
	prussdrv_pruintc_init(&pruss_intc_initdata);
	printf("OK\n");
	
	// VRAM initialisieren
	if(!(initVRAM()))
		return(12132);
	
	printf("Flashing PRU Program ... ");
	
	// Load and execute the PRU program on the PRU
	prussdrv_exec_program (PRU_NUM, "./pru_0.bin");
	
	printf("OK\n");
	
	int i = prussdrv_pru_send_wait_clear_event(32, PRU_EVTOUT_0, 32);
	
	// Wait for event completion from PRU, returns PRU_EVTOUT_0 number
	while(running)
	{
		int c = prussdrv_pru_send_wait_clear_event(32, PRU_EVTOUT_0, 32);
		
		// Optional if you want to refresh the frames
		// Otherwise the same frame is drawn over and over again
		// writeFrameToVRAM();
	}
	
	printf("Exiting ... ");
	
	// Disable PRU and close memory mappings
	prussdrv_pru_disable(PRU_NUM);
	prussdrv_exit ();
	
    munmap(vram, VRAM_SIZE);
    close(mem_dev);
	
	printf("OK\n");

    return(0);
}
