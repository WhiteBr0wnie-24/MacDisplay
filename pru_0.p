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

.origin 0
.entrypoint START

#include "pru_constants.hp"

START:
	// Temporary loop register (DEBUG ONLY)
	// TODO: Remove from finished code (Replace with endless loop)
	MOV		r0, 1000
	MOV		r7, 342
	QBA		RUN
	
SEND_VSYNC:
	SEND_R30_3_PULSE 18450, 26550
	// Notify the host, that a new frame can be buffered
	QBA	RUN
	
RUN:
	SEND_R30_5_PULSE 18450, 26550
	SUB		r7, r7, 1
	QBNE	RUN, r7, 0
	MOV		r7, 342
	SUB		r0, r0, 1
	QBEQ	END, r0, 0
	QBA		SEND_VSYNC

END:
	MOV		R31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
	HALT                     // halt the pru program