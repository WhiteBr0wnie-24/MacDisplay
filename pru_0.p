/*
* Copyright (C) 2016, Daniel Herczeg
*
* Kompilation mithilfe von: pasm -b [INPUT_FILE].p
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 3 as
* published by the Free Software Foundation.
*
* It can be viewed here: http://www.gnu.org/licenses/gpl-3.0.de.html
*/

#define PRU0_R31_VEC_VALID		32    // allows notification of program completion
#define PRU_EVTOUT_0			3     // the event number that is sent back, #define PRU_EVTOUT_1 4

.origin 0
.entrypoint START

.macro DELAY_NS
.mparam	wait_number
    MOV r29, ((wait_number/10)-5)
	// MOV r29, ((wait_number/5)-5)
    DELAY:
        SUB r29, r29, 1
        QBNE DELAY, r29, 0
.endm

.macro READVRAMDATA
	LBBO		r4, r27, 0, 64
	ADD			r27, r27, 0x00000040
	// ADD			r27, r27, 0x00000010
.endm

.macro WRITEREG
.mparam reg
	MOV			r21, 31
	QBA			WRITEPIXEL
	WRITEPIXEL:
		SUB			r21, r21, 1
		// DELAY_NS	80;
		DELAY_NS	85;
		QBBS		WRITEPIXEL_HIGH, reg.t31
		QBBC		WRITEPIXEL_LOW, reg.t31
	WRITEPIXEL_HIGH:
		SET			R30.t0
		LSL			reg, reg, #1
		QBNE		WRITEPIXEL, r21, 0
	WRITEPIXEL_LOW:
		CLR			R30.t0
		LSL			reg, reg, #1
		QBNE		WRITEPIXEL, r21, 0
	// SET			R30.t0
.endm

.macro SEND_R30_5_PULSE
// .mparam	LOWTIME, HIGHTIME
// .mparam	line
	READVRAMDATA
	CLR		R30.t5
	// DELAY_NS LOWTIME
	// DELAY_NS	 11185				// <--- alter Wert (funktionierte mit weissen linien !!!)
	
	// An der stelle dynamisch warten, je nachdem, wie lange das readvram data gedauert hat
	
	
	DELAY_NS	 11150
	// SET		R30.t0
	// START VIDEO SIGNAL (ersten 3 regs + 9 bits, dauer: 7245ns)
	// DELAY_NS	 7245
	WRITEREG	 r4
	WRITEREG	 r5
	WRITEREG	 r6
	WRITEREG	 r7
	SET			 R30.t5
	WRITEREG	 r8
	WRITEREG	 r9
	WRITEREG	 r10
	WRITEREG	 r11
	WRITEREG	 r12
	WRITEREG	 r13
	WRITEREG	 r14
	WRITEREG	 r15
	WRITEREG	 r16
	WRITEREG	 r17
	WRITEREG	 r18
	WRITEREG	 r19
	// DELAY_NS HIGHTIME
	// DELAY_NS	25445
	CLR		R30.t0
	DELAY_NS	995
.endm

.macro SEND_R30_3_PULSE
.mparam	LOWTIME, HIGHTIME
	MOV		R31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
	SET		R31.b0, R31.b0, 0
	CLR		R30.t5
	DELAY_NS LOWTIME/2
	CLR		R30.t3
	DELAY_NS LOWTIME/2
	SET		R30.t5
	DELAY_NS HIGHTIME
	CLR		R30.t5
	DELAY_NS 18450
	SET		R30.t5
	DELAY_NS 26540
	CLR		R30.t5
	DELAY_NS 18450
	SET		R30.t5
	DELAY_NS 26540
	CLR		R30.t5
	DELAY_NS 18450
	SET		R30.t5
	DELAY_NS 26540
	CLR		R30.t5
	DELAY_NS LOWTIME/2
	SET		R30.t3
	DELAY_NS LOWTIME/2
	SET		R30.t5
	DELAY_NS HIGHTIME
.endm

// DEFINE CONSTANTS //
#define SHARED_RAM_ADDRESS		0x10000000
#define CONST_VRAM				0x2202C
#define MEM_LENGTH				0x00000008

// REGISTER MAP //
// Die vom Host-Programm uebermittelte Position der Videodaten im VRAM
#define VRAM_ADDRESS			r1
#define SHARED_RAM				r2
// Das Register, in welches die Pixel-Daten aus dem RAM geladen werden
#define PIXELDATA				r4

#define DATA_ADDRESS			r22
#define	CURR_OFFSET				r24

START:
	// Enable OCP master ports in SYSCFG register
	// Siehe TRM, Seite 274: Table 207
	LBCO	r0, C4, 4, 4  
	CLR		r0, r0, 4  
	SBCO	r0, C4, 4, 4
	
	// Configure the programmable pointer register for PRU0 by setting c28_pointer[15:0] 
	// field to 0x0120.  This will make C28 point to 0x00012000 (PRU shared RAM).
	// MOV		r0, 0x00000120
	// SBBO	r0, CONST_SHARED, 0, 4
	
	// Configure the programmable pointer register for PRU0 by setting c31_pointer[15:0] 
	// field to 0x0010.  This will make C31 point to 0x80001000 (DDR memory).
	// Siehe RTM, Seite 27, Sektion 5.2.2.3, Tabelle 13 und Sekt. 5.2.2.3.2
	MOV		r0, 0x00100000
	MOV		r3, CONST_VRAM
	SBBO	r0, r3, 0, 4
	
	// Temporary loop register (DEBUG ONLY)
	// TODO: Remove from finished code (Replace with endless loop)
	MOV		r0, 1000
	MOV		r26, 342
	MOV		r25, 1000
	MOV		CURR_OFFSET, 0
	// MOV		r22, 0x80001000
	MOV		r22, 0x00080000
	MOV		r27, 0x80001000
	
	QBA		RUN

SEND_VSYNC:
	MOV		r27, 0x80001000
	SEND_R30_3_PULSE 18450, 26550
	// MOV		r22, 0x80001000
	MOV		r22, 0x00080000
	MOV		r26, 342
	// SUB		r25, r25, 1
	// Notify the host, that a new frame can be buffered
	// Endlosschleife !
	// QBNE	RUN, r25, 0
	// QBEQ	END, r25, 0
	
	// HSYNC Pulse waehrend 1,25ms weiter laufen lassen nach dem hsync
	// video daten beginnen nicht sofort nach dem hsync (fuehrt sonst zu
	// bildfehlern)
	MOV		r28, 28
	SEND_HSYNC:
		CLR		R30.t5
		DELAY_NS 18440
		SET		R30.t5
		DELAY_NS 26520
		SUB		r28, r28, 1
		QBNE	SEND_HSYNC, r28, 0
	
	QBA RUN
	
RUN:
	// SEND_R30_5_PULSE DATA_ADDRESS
	SEND_R30_5_PULSE
	// WRITELINE 0
	SUB		r26, r26, 1
	ADD		DATA_ADDRESS, DATA_ADDRESS, MEM_LENGTH*16
	QBEQ	SEND_VSYNC, r26, 0
	QBA 	RUN

END:                             // notify the calling app that finished
	MOV		R31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
	HALT                     // halt the pru program