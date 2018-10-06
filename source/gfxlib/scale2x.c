/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1999-2002 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * This file contains a C and MMX implentation of the Scale2x effect.
 *
 * You can found an high level description of the effect at :
 *
 * http://scale2x.sourceforge.net/scale2x.html
 *
 * Alternatively at the previous license terms, you are allowed to use this
 * code in your program with these conditions:
 * - the program is not used in commercial activities.
 * - the whole source code of the program is released with the binary.
 * - derivative works of the program are allowed.
 */

/*
 * Code adapted To OpenBOR by SX
 * scale2x.c - Trying to scale 2x.
 *
 * Updated: 5/05/08 - SX
 *
 */


#include "gfx.h"
#include "gfxtypes.h"

/* Suggested in "Intel Optimization" for Pentium II */
#define ASM_JUMP_ALIGN ".p2align 4\n"

static void internal_scale2x_16_def(u16 * dst0, u16 * dst1, const u16 * src0, const u16 * src1, const u16 * src2,
				    unsigned count) {
	/* first pixel */
	dst0[0] = src1[0];
	dst1[0] = src1[0];
	if(src1[1] == src0[0] && src2[0] != src0[0])
		dst0[1] = src0[0];
	else
		dst0[1] = src1[0];
	if(src1[1] == src2[0] && src0[0] != src2[0])
		dst1[1] = src2[0];
	else
		dst1[1] = src1[0];
	++src0;
	++src1;
	++src2;
	dst0 += 2;
	dst1 += 2;

	/* central pixels */
	count -= 2;
	while(count) {
		if(src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0])
			dst0[0] = src0[0];
		else
			dst0[0] = src1[0];
		if(src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0])
			dst0[1] = src0[0];
		else
			dst0[1] = src1[0];

		if(src1[-1] == src2[0] && src0[0] != src2[0] && src1[1] != src2[0])
			dst1[0] = src2[0];
		else
			dst1[0] = src1[0];
		if(src1[1] == src2[0] && src0[0] != src2[0] && src1[-1] != src2[0])
			dst1[1] = src2[0];
		else
			dst1[1] = src1[0];

		++src0;
		++src1;
		++src2;
		dst0 += 2;
		dst1 += 2;
		--count;
	}

	/* last pixel */
	if(src1[-1] == src0[0] && src2[0] != src0[0])
		dst0[0] = src0[0];
	else
		dst0[0] = src1[0];
	if(src1[-1] == src2[0] && src0[0] != src2[0])
		dst1[0] = src2[0];
	else
		dst1[0] = src1[0];
	dst0[1] = src1[0];
	dst1[1] = src1[0];
}

static void internal_scale2x_32_def(u32 * dst0, u32 * dst1, const u32 * src0, const u32 * src1, const u32 * src2,
				    unsigned count) {
	/* first pixel */
	dst0[0] = src1[0];
	dst1[0] = src1[0];
	if(src1[1] == src0[0] && src2[0] != src0[0])
		dst0[1] = src0[0];
	else
		dst0[1] = src1[0];
	if(src1[1] == src2[0] && src0[0] != src2[0])
		dst1[1] = src2[0];
	else
		dst1[1] = src1[0];
	++src0;
	++src1;
	++src2;
	dst0 += 2;
	dst1 += 2;

	/* central pixels */
	count -= 2;
	while(count) {
		if(src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0])
			dst0[0] = src0[0];
		else
			dst0[0] = src1[0];
		if(src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0])
			dst0[1] = src0[0];
		else
			dst0[1] = src1[0];

		if(src1[-1] == src2[0] && src0[0] != src2[0] && src1[1] != src2[0])
			dst1[0] = src2[0];
		else
			dst1[0] = src1[0];
		if(src1[1] == src2[0] && src0[0] != src2[0] && src1[-1] != src2[0])
			dst1[1] = src2[0];
		else
			dst1[1] = src1[0];

		++src0;
		++src1;
		++src2;
		dst0 += 2;
		dst1 += 2;
		--count;
	}

	/* last pixel */
	if(src1[-1] == src0[0] && src2[0] != src0[0])
		dst0[0] = src0[0];
	else
		dst0[0] = src1[0];
	if(src1[-1] == src2[0] && src0[0] != src2[0])
		dst1[0] = src2[0];
	else
		dst1[0] = src1[0];
	dst0[1] = src1[0];
	dst1[1] = src1[0];
}

#ifdef MMX
static void internal_scale2x_16_mmx_single(u16 * dst, const u16 * src0, const u16 * src1, const u16 * src2,
					   unsigned count) {
	/* always do the first and last run */
	count -= 2 * 4;

#ifdef __GNUC__
	__asm__ __volatile__(
				    /* first run */
				    /* set the current, current_pre, current_next registers */
				    "pxor %%mm0,%%mm0\n"	/* use a fake black out of screen */
				    "movq 0(%1),%%mm7\n"
				    "movq 8(%1),%%mm1\n"
				    "psrlq $48,%%mm0\n"
				    "psllq $48,%%mm1\n"
				    "movq %%mm7,%%mm2\n"
				    "movq %%mm7,%%mm3\n"
				    "psllq $16,%%mm2\n" "psrlq $16,%%mm3\n" "por %%mm2,%%mm0\n" "por %%mm3,%%mm1\n"
				    /* current_upper */
				    "movq (%0),%%mm6\n"
				    /* compute the upper-left pixel for dst0 on %%mm2 */
				    /* compute the upper-right pixel for dst0 on %%mm4 */
				    "movq %%mm0,%%mm2\n"
				    "movq %%mm1,%%mm4\n"
				    "movq %%mm0,%%mm3\n"
				    "movq %%mm1,%%mm5\n"
				    "pcmpeqw %%mm6,%%mm2\n"
				    "pcmpeqw %%mm6,%%mm4\n"
				    "pcmpeqw (%2),%%mm3\n"
				    "pcmpeqw (%2),%%mm5\n"
				    "pandn %%mm2,%%mm3\n"
				    "pandn %%mm4,%%mm5\n"
				    "movq %%mm0,%%mm2\n"
				    "movq %%mm1,%%mm4\n"
				    "pcmpeqw %%mm1,%%mm2\n"
				    "pcmpeqw %%mm0,%%mm4\n"
				    "pandn %%mm3,%%mm2\n"
				    "pandn %%mm5,%%mm4\n"
				    "movq %%mm2,%%mm3\n"
				    "movq %%mm4,%%mm5\n"
				    "pand %%mm6,%%mm2\n"
				    "pand %%mm6,%%mm4\n"
				    "pandn %%mm7,%%mm3\n" "pandn %%mm7,%%mm5\n" "por %%mm3,%%mm2\n" "por %%mm5,%%mm4\n"
				    /* set *dst0 */
				    "movq %%mm2,%%mm3\n"
				    "punpcklwd %%mm4,%%mm2\n"
				    "punpckhwd %%mm4,%%mm3\n" "movq %%mm2,(%3)\n" "movq %%mm3,8(%3)\n"
				    /* next */
				    "add $8,%0\n" "add $8,%1\n" "add $8,%2\n" "add $16,%3\n"
				    /* central runs */
				    "shr $2,%4\n" "jz 1f\n" ASM_JUMP_ALIGN "0:\n"
				    /* set the current, current_pre, current_next registers */
				    "movq -8(%1),%%mm0\n"
				    "movq (%1),%%mm7\n"
				    "movq 8(%1),%%mm1\n"
				    "psrlq $48,%%mm0\n"
				    "psllq $48,%%mm1\n"
				    "movq %%mm7,%%mm2\n"
				    "movq %%mm7,%%mm3\n"
				    "psllq $16,%%mm2\n" "psrlq $16,%%mm3\n" "por %%mm2,%%mm0\n" "por %%mm3,%%mm1\n"
				    /* current_upper */
				    "movq (%0),%%mm6\n"
				    /* compute the upper-left pixel for dst0 on %%mm2 */
				    /* compute the upper-right pixel for dst0 on %%mm4 */
				    "movq %%mm0,%%mm2\n"
				    "movq %%mm1,%%mm4\n"
				    "movq %%mm0,%%mm3\n"
				    "movq %%mm1,%%mm5\n"
				    "pcmpeqw %%mm6,%%mm2\n"
				    "pcmpeqw %%mm6,%%mm4\n"
				    "pcmpeqw (%2),%%mm3\n"
				    "pcmpeqw (%2),%%mm5\n"
				    "pandn %%mm2,%%mm3\n"
				    "pandn %%mm4,%%mm5\n"
				    "movq %%mm0,%%mm2\n"
				    "movq %%mm1,%%mm4\n"
				    "pcmpeqw %%mm1,%%mm2\n"
				    "pcmpeqw %%mm0,%%mm4\n"
				    "pandn %%mm3,%%mm2\n"
				    "pandn %%mm5,%%mm4\n"
				    "movq %%mm2,%%mm3\n"
				    "movq %%mm4,%%mm5\n"
				    "pand %%mm6,%%mm2\n"
				    "pand %%mm6,%%mm4\n"
				    "pandn %%mm7,%%mm3\n" "pandn %%mm7,%%mm5\n" "por %%mm3,%%mm2\n" "por %%mm5,%%mm4\n"
				    /* set *dst0 */
				    "movq %%mm2,%%mm3\n"
				    "punpcklwd %%mm4,%%mm2\n"
				    "punpckhwd %%mm4,%%mm3\n" "movq %%mm2,(%3)\n" "movq %%mm3,8(%3)\n"
				    /* next */
				    "add $8,%0\n"
				    "add $8,%1\n" "add $8,%2\n" "add $16,%3\n" "decl %4\n" "jnz 0b\n" "1:\n"
				    /* final run */
				    /* set the current, current_pre, current_next registers */
				    "movq -8(%1),%%mm0\n" "movq (%1),%%mm7\n" "pxor %%mm1,%%mm1\n"	/* use a fake black out of screen */
				    "psrlq $48,%%mm0\n"
				    "psllq $48,%%mm1\n"
				    "movq %%mm7,%%mm2\n"
				    "movq %%mm7,%%mm3\n"
				    "psllq $16,%%mm2\n" "psrlq $16,%%mm3\n" "por %%mm2,%%mm0\n" "por %%mm3,%%mm1\n"
				    /* current_upper */
				    "movq (%0),%%mm6\n"
				    /* compute the upper-left pixel for dst0 on %%mm2 */
				    /* compute the upper-right pixel for dst0 on %%mm4 */
				    "movq %%mm0,%%mm2\n"
				    "movq %%mm1,%%mm4\n"
				    "movq %%mm0,%%mm3\n"
				    "movq %%mm1,%%mm5\n"
				    "pcmpeqw %%mm6,%%mm2\n"
				    "pcmpeqw %%mm6,%%mm4\n"
				    "pcmpeqw (%2),%%mm3\n"
				    "pcmpeqw (%2),%%mm5\n"
				    "pandn %%mm2,%%mm3\n"
				    "pandn %%mm4,%%mm5\n"
				    "movq %%mm0,%%mm2\n"
				    "movq %%mm1,%%mm4\n"
				    "pcmpeqw %%mm1,%%mm2\n"
				    "pcmpeqw %%mm0,%%mm4\n"
				    "pandn %%mm3,%%mm2\n"
				    "pandn %%mm5,%%mm4\n"
				    "movq %%mm2,%%mm3\n"
				    "movq %%mm4,%%mm5\n"
				    "pand %%mm6,%%mm2\n"
				    "pand %%mm6,%%mm4\n"
				    "pandn %%mm7,%%mm3\n" "pandn %%mm7,%%mm5\n" "por %%mm3,%%mm2\n" "por %%mm5,%%mm4\n"
				    /* set *dst0 */
				    "movq %%mm2,%%mm3\n"
				    "punpcklwd %%mm4,%%mm2\n"
				    "punpckhwd %%mm4,%%mm3\n"
				    "movq %%mm2,(%3)\n"
				    "movq %%mm3,8(%3)\n"
				    "emms\n":"+r"(src0), "+r"(src1), "+r"(src2), "+r"(dst), "+r"(count)
				    ::"cc");
#else
	__asm {
		mov eax, src0;
		mov ebx, src1;
		mov ecx, src2;
		mov edx, dst;
		mov esi, count;

		/* first run */
		/* set the current, current_pre, current_next registers */
		pxor mm0, mm0;	/* use a fake black out of screen */
		movq mm7, qword ptr[ebx];
		movq mm1, qword ptr[ebx + 8];
		psrlq mm0, 48;
		psllq mm1, 48;
		movq mm2, mm7;
		movq mm3, mm7;
		psllq mm2, 16;
		psrlq mm3, 16;
		por mm0, mm2;
		por mm1, mm3;

		/* current_upper */
		movq mm6, qword ptr[eax];

		/* compute the upper-left pixel for dst0 on %%mm2 */
		/* compute the upper-right pixel for dst0 on %%mm4 */
		movq mm2, mm0;
		movq mm4, mm1;
		movq mm3, mm0;
		movq mm5, mm1;
		pcmpeqw mm2, mm6;
		pcmpeqw mm4, mm6;
		pcmpeqw mm3, qword ptr[ecx];
		pcmpeqw mm5, qword ptr[ecx];
		pandn mm3, mm2;
		pandn mm5, mm4;
		movq mm2, mm0;
		movq mm4, mm1;
		pcmpeqw mm2, mm1;
		pcmpeqw mm4, mm0;
		pandn mm2, mm3;
		pandn mm4, mm5;
		movq mm3, mm2;
		movq mm5, mm4;
		pand mm2, mm6;
		pand mm4, mm6;
		pandn mm3, mm7;
		pandn mm5, mm7;
		por mm2, mm3;
		por mm4, mm5;

		/* set *dst0 */
		movq mm3, mm2;
		punpcklwd mm2, mm4;
		punpckhwd mm3, mm4;
		movq qword ptr[edx], mm2;
		movq qword ptr[edx + 8], mm3;

		/* next */
		add eax, 8;
		add ebx, 8;
		add ecx, 8;
		add edx, 16;

		/* central runs */
		shr esi, 2;
		jz label1;
		align 4;
	label0:

		/* set the current, current_pre, current_next registers */
		movq mm0, qword ptr[ebx - 8];
		movq mm7, qword ptr[ebx];
		movq mm1, qword ptr[ebx + 8];
		psrlq mm0, 48;
		psllq mm1, 48;
		movq mm2, mm7;
		movq mm3, mm7;
		psllq mm2, 16;
		psrlq mm3, 16;
		por mm0, mm2;
		por mm1, mm3;

		/* current_upper */
		movq mm6, qword ptr[eax];

		/* compute the upper-left pixel for dst0 on %%mm2 */
		/* compute the upper-right pixel for dst0 on %%mm4 */
		movq mm2, mm0;
		movq mm4, mm1;
		movq mm3, mm0;
		movq mm5, mm1;
		pcmpeqw mm2, mm6;
		pcmpeqw mm4, mm6;
		pcmpeqw mm3, qword ptr[ecx];
		pcmpeqw mm5, qword ptr[ecx];
		pandn mm3, mm2;
		pandn mm5, mm4;
		movq mm2, mm0;
		movq mm4, mm1;
		pcmpeqw mm2, mm1;
		pcmpeqw mm4, mm0;
		pandn mm2, mm3;
		pandn mm4, mm5;
		movq mm3, mm2;
		movq mm5, mm4;
		pand mm2, mm6;
		pand mm4, mm6;
		pandn mm3, mm7;
		pandn mm5, mm7;
		por mm2, mm3;
		por mm4, mm5;

		/* set *dst0 */
		movq mm3, mm2;
		punpcklwd mm2, mm4;
		punpckhwd mm3, mm4;
		movq qword ptr[edx], mm2;
		movq qword ptr[edx + 8], mm3;

		/* next */
		add eax, 8;
		add ebx, 8;
		add ecx, 8;
		add edx, 16;

		dec esi;
		jnz label0;
	label1:

		/* final run */
		/* set the current, current_pre, current_next registers */
		movq mm0, qword ptr[ebx - 8];
		movq mm7, qword ptr[ebx];
		pxor mm1, mm1;	/* use a fake black out of screen */
		psrlq mm0, 48;
		psllq mm1, 48;
		movq mm2, mm7;
		movq mm3, mm7;
		psllq mm2, 16;
		psrlq mm3, 16;
		por mm0, mm2;
		por mm1, mm3;

		/* current_upper */
		movq mm6, qword ptr[eax];

		/* compute the upper-left pixel for dst0 on %%mm2 */
		/* compute the upper-right pixel for dst0 on %%mm4 */
		movq mm2, mm0;
		movq mm4, mm1;
		movq mm3, mm0;
		movq mm5, mm1;
		pcmpeqw mm2, mm6;
		pcmpeqw mm4, mm6;
		pcmpeqw mm3, qword ptr[ecx];
		pcmpeqw mm5, qword ptr[ecx];
		pandn mm3, mm2;
		pandn mm5, mm4;
		movq mm2, mm0;
		movq mm4, mm1;
		pcmpeqw mm2, mm1;
		pcmpeqw mm4, mm0;
		pandn mm2, mm3;
		pandn mm4, mm5;
		movq mm3, mm2;
		movq mm5, mm4;
		pand mm2, mm6;
		pand mm4, mm6;
		pandn mm3, mm7;
		pandn mm5, mm7;
		por mm2, mm3;
		por mm4, mm5;

		/* set *dst0 */
		movq mm3, mm2;
		punpcklwd mm2, mm4;
		punpckhwd mm3, mm4;
		movq qword ptr[edx], mm2;
		movq qword ptr[edx + 8], mm3;

		mov src0, eax;
		mov src1, ebx;
		mov src2, ecx;
		mov dst, edx;
		mov count, esi;

		emms;
	}
#endif
}

static void internal_scale2x_32_mmx_single(u32 * dst, const u32 * src0, const u32 * src1, const u32 * src2,
					   unsigned count) {
	/* always do the first and last run */
	count -= 2 * 2;

#ifdef __GNUC__
	__asm__ __volatile__(
				    /* first run */
				    /* set the current, current_pre, current_next registers */
				    "pxor %%mm0,%%mm0\n"	/* use a fake black out of screen */
				    "movq 0(%1),%%mm7\n"
				    "movq 8(%1),%%mm1\n"
				    "psrlq $32,%%mm0\n"
				    "psllq $32,%%mm1\n"
				    "movq %%mm7,%%mm2\n"
				    "movq %%mm7,%%mm3\n"
				    "psllq $32,%%mm2\n" "psrlq $32,%%mm3\n" "por %%mm2,%%mm0\n" "por %%mm3,%%mm1\n"
				    /* current_upper */
				    "movq (%0),%%mm6\n"
				    /* compute the upper-left pixel for dst0 on %%mm2 */
				    /* compute the upper-right pixel for dst0 on %%mm4 */
				    "movq %%mm0,%%mm2\n"
				    "movq %%mm1,%%mm4\n"
				    "movq %%mm0,%%mm3\n"
				    "movq %%mm1,%%mm5\n"
				    "pcmpeqd %%mm6,%%mm2\n"
				    "pcmpeqd %%mm6,%%mm4\n"
				    "pcmpeqd (%2),%%mm3\n"
				    "pcmpeqd (%2),%%mm5\n"
				    "pandn %%mm2,%%mm3\n"
				    "pandn %%mm4,%%mm5\n"
				    "movq %%mm0,%%mm2\n"
				    "movq %%mm1,%%mm4\n"
				    "pcmpeqd %%mm1,%%mm2\n"
				    "pcmpeqd %%mm0,%%mm4\n"
				    "pandn %%mm3,%%mm2\n"
				    "pandn %%mm5,%%mm4\n"
				    "movq %%mm2,%%mm3\n"
				    "movq %%mm4,%%mm5\n"
				    "pand %%mm6,%%mm2\n"
				    "pand %%mm6,%%mm4\n"
				    "pandn %%mm7,%%mm3\n" "pandn %%mm7,%%mm5\n" "por %%mm3,%%mm2\n" "por %%mm5,%%mm4\n"
				    /* set *dst0 */
				    "movq %%mm2,%%mm3\n"
				    "punpckldq %%mm4,%%mm2\n"
				    "punpckhdq %%mm4,%%mm3\n" "movq %%mm2,(%3)\n" "movq %%mm3,8(%3)\n"
				    /* next */
				    "add $8,%0\n" "add $8,%1\n" "add $8,%2\n" "add $16,%3\n"
				    /* central runs */
				    "shr $1,%4\n" "jz 1f\n" ASM_JUMP_ALIGN "0:\n"
				    /* set the current, current_pre, current_next registers */
				    "movq -8(%1),%%mm0\n"
				    "movq (%1),%%mm7\n"
				    "movq 8(%1),%%mm1\n"
				    "psrlq $32,%%mm0\n"
				    "psllq $32,%%mm1\n"
				    "movq %%mm7,%%mm2\n"
				    "movq %%mm7,%%mm3\n"
				    "psllq $32,%%mm2\n" "psrlq $32,%%mm3\n" "por %%mm2,%%mm0\n" "por %%mm3,%%mm1\n"
				    /* current_upper */
				    "movq (%0),%%mm6\n"
				    /* compute the upper-left pixel for dst0 on %%mm2 */
				    /* compute the upper-right pixel for dst0 on %%mm4 */
				    "movq %%mm0,%%mm2\n"
				    "movq %%mm1,%%mm4\n"
				    "movq %%mm0,%%mm3\n"
				    "movq %%mm1,%%mm5\n"
				    "pcmpeqd %%mm6,%%mm2\n"
				    "pcmpeqd %%mm6,%%mm4\n"
				    "pcmpeqd (%2),%%mm3\n"
				    "pcmpeqd (%2),%%mm5\n"
				    "pandn %%mm2,%%mm3\n"
				    "pandn %%mm4,%%mm5\n"
				    "movq %%mm0,%%mm2\n"
				    "movq %%mm1,%%mm4\n"
				    "pcmpeqd %%mm1,%%mm2\n"
				    "pcmpeqd %%mm0,%%mm4\n"
				    "pandn %%mm3,%%mm2\n"
				    "pandn %%mm5,%%mm4\n"
				    "movq %%mm2,%%mm3\n"
				    "movq %%mm4,%%mm5\n"
				    "pand %%mm6,%%mm2\n"
				    "pand %%mm6,%%mm4\n"
				    "pandn %%mm7,%%mm3\n" "pandn %%mm7,%%mm5\n" "por %%mm3,%%mm2\n" "por %%mm5,%%mm4\n"
				    /* set *dst0 */
				    "movq %%mm2,%%mm3\n"
				    "punpckldq %%mm4,%%mm2\n"
				    "punpckhdq %%mm4,%%mm3\n" "movq %%mm2,(%3)\n" "movq %%mm3,8(%3)\n"
				    /* next */
				    "add $8,%0\n"
				    "add $8,%1\n" "add $8,%2\n" "add $16,%3\n" "decl %4\n" "jnz 0b\n" "1:\n"
				    /* final run */
				    /* set the current, current_pre, current_next registers */
				    "movq -8(%1),%%mm0\n" "movq (%1),%%mm7\n" "pxor %%mm1,%%mm1\n"	/* use a fake black out of screen */
				    "psrlq $32,%%mm0\n"
				    "psllq $32,%%mm1\n"
				    "movq %%mm7,%%mm2\n"
				    "movq %%mm7,%%mm3\n"
				    "psllq $32,%%mm2\n" "psrlq $32,%%mm3\n" "por %%mm2,%%mm0\n" "por %%mm3,%%mm1\n"
				    /* current_upper */
				    "movq (%0),%%mm6\n"
				    /* compute the upper-left pixel for dst0 on %%mm2 */
				    /* compute the upper-right pixel for dst0 on %%mm4 */
				    "movq %%mm0,%%mm2\n"
				    "movq %%mm1,%%mm4\n"
				    "movq %%mm0,%%mm3\n"
				    "movq %%mm1,%%mm5\n"
				    "pcmpeqd %%mm6,%%mm2\n"
				    "pcmpeqd %%mm6,%%mm4\n"
				    "pcmpeqd (%2),%%mm3\n"
				    "pcmpeqd (%2),%%mm5\n"
				    "pandn %%mm2,%%mm3\n"
				    "pandn %%mm4,%%mm5\n"
				    "movq %%mm0,%%mm2\n"
				    "movq %%mm1,%%mm4\n"
				    "pcmpeqd %%mm1,%%mm2\n"
				    "pcmpeqd %%mm0,%%mm4\n"
				    "pandn %%mm3,%%mm2\n"
				    "pandn %%mm5,%%mm4\n"
				    "movq %%mm2,%%mm3\n"
				    "movq %%mm4,%%mm5\n"
				    "pand %%mm6,%%mm2\n"
				    "pand %%mm6,%%mm4\n"
				    "pandn %%mm7,%%mm3\n" "pandn %%mm7,%%mm5\n" "por %%mm3,%%mm2\n" "por %%mm5,%%mm4\n"
				    /* set *dst0 */
				    "movq %%mm2,%%mm3\n"
				    "punpckldq %%mm4,%%mm2\n"
				    "punpckhdq %%mm4,%%mm3\n"
				    "movq %%mm2,(%3)\n"
				    "movq %%mm3,8(%3)\n"
				    "emms\n":"+r"(src0), "+r"(src1), "+r"(src2), "+r"(dst), "+r"(count)
				    ::"cc");
#else
	__asm {
		mov eax, src0;
		mov ebx, src1;
		mov ecx, src2;
		mov edx, dst;
		mov esi, count;

		/* first run */
		/* set the current, current_pre, current_next registers */
		pxor mm0, mm0;
		movq mm7, qword ptr[ebx];
		movq mm1, qword ptr[ebx + 8];
		psrlq mm0, 32;
		psllq mm1, 32;
		movq mm2, mm7;
		movq mm3, mm7;
		psllq mm2, 32;
		psrlq mm3, 32;
		por mm0, mm2;
		por mm1, mm3;

		/* current_upper */
		movq mm6, qword ptr[eax];

		/* compute the upper-left pixel for dst0 on %%mm2 */
		/* compute the upper-right pixel for dst0 on %%mm4 */
		movq mm2, mm0;
		movq mm4, mm1;
		movq mm3, mm0;
		movq mm5, mm1;
		pcmpeqd mm2, mm6;
		pcmpeqd mm4, mm6;
		pcmpeqd mm3, qword ptr[ecx];
		pcmpeqd mm5, qword ptr[ecx];
		pandn mm3, mm2;
		pandn mm5, mm4;
		movq mm2, mm0;
		movq mm4, mm1;
		pcmpeqd mm2, mm1;
		pcmpeqd mm4, mm0;
		pandn mm2, mm3;
		pandn mm4, mm5;
		movq mm3, mm2;
		movq mm5, mm4;
		pand mm2, mm6;
		pand mm4, mm6;
		pandn mm3, mm7;
		pandn mm5, mm7;
		por mm2, mm3;
		por mm4, mm5;

		/* set *dst0 */
		movq mm3, mm2;
		punpckldq mm2, mm4;
		punpckhdq mm3, mm4;
		movq qword ptr[edx], mm2;
		movq qword ptr[edx + 8], mm3;

		/* next */
		add eax, 8;
		add ebx, 8;
		add ecx, 8;
		add edx, 16;

		/* central runs */
		shr esi, 1;
		jz label1;
	label0:

		/* set the current, current_pre, current_next registers */
		movq mm0, qword ptr[ebx - 8];
		movq mm7, qword ptr[ebx];
		movq mm1, qword ptr[ebx + 8];
		psrlq mm0, 32;
		psllq mm1, 32;
		movq mm2, mm7;
		movq mm3, mm7;
		psllq mm2, 32;
		psrlq mm3, 32;
		por mm0, mm2;
		por mm1, mm3;

		/* current_upper */
		movq mm6, qword ptr[eax];

		/* compute the upper-left pixel for dst0 on %%mm2 */
		/* compute the upper-right pixel for dst0 on %%mm4 */
		movq mm2, mm0;
		movq mm4, mm1;
		movq mm3, mm0;
		movq mm5, mm1;
		pcmpeqd mm2, mm6;
		pcmpeqd mm4, mm6;
		pcmpeqd mm3, qword ptr[ecx];
		pcmpeqd mm5, qword ptr[ecx];
		pandn mm3, mm2;
		pandn mm5, mm4;
		movq mm2, mm0;
		movq mm4, mm1;
		pcmpeqd mm2, mm1;
		pcmpeqd mm4, mm0;
		pandn mm2, mm3;
		pandn mm4, mm5;
		movq mm3, mm2;
		movq mm5, mm4;
		pand mm2, mm6;
		pand mm4, mm6;
		pandn mm3, mm7;
		pandn mm5, mm7;
		por mm2, mm3;
		por mm4, mm5;

		/* set *dst0 */
		movq mm3, mm2;
		punpckldq mm2, mm4;
		punpckhdq mm3, mm4;
		movq qword ptr[edx], mm2;
		movq qword ptr[edx + 8], mm3;

		/* next */
		add eax, 8;
		add ebx, 8;
		add ecx, 8;
		add edx, 16;

		dec esi;
		jnz label0;
	label1:

		/* final run */
		/* set the current, current_pre, current_next registers */
		movq mm0, qword ptr[ebx - 8];
		movq mm7, qword ptr[ebx];
		pxor mm1, mm1;
		psrlq mm0, 32;
		psllq mm1, 32;
		movq mm2, mm7;
		movq mm3, mm7;
		psllq mm2, 32;
		psrlq mm3, 32;
		por mm0, mm2;
		por mm1, mm3;

		/* current_upper */
		movq mm6, qword ptr[eax];

		/* compute the upper-left pixel for dst0 on %%mm2 */
		/* compute the upper-right pixel for dst0 on %%mm4 */
		movq mm2, mm0;
		movq mm4, mm1;
		movq mm3, mm0;
		movq mm5, mm1;
		pcmpeqd mm2, mm6;
		pcmpeqd mm4, mm6;
		pcmpeqd mm3, qword ptr[ecx];
		pcmpeqd mm5, qword ptr[ecx];
		pandn mm3, mm2;
		pandn mm5, mm4;
		movq mm2, mm0;
		movq mm4, mm1;
		pcmpeqd mm2, mm1;
		pcmpeqd mm4, mm0;
		pandn mm2, mm3;
		pandn mm4, mm5;
		movq mm3, mm2;
		movq mm5, mm4;
		pand mm2, mm6;
		pand mm4, mm6;
		pandn mm3, mm7;
		pandn mm5, mm7;
		por mm2, mm3;
		por mm4, mm5;

		/* set *dst0 */
		movq mm3, mm2;
		punpckldq mm2, mm4;
		punpckhdq mm3, mm4;
		movq qword ptr[edx], mm2;
		movq qword ptr[edx + 8], mm3;

		mov src0, eax;
		mov src1, ebx;
		mov src2, ecx;
		mov dst, edx;
		mov count, esi;

		emms;
	}
#endif
}

static void internal_scale2x_16_mmx(u16 * dst0, u16 * dst1, const u16 * src0, const u16 * src1, const u16 * src2,
				    unsigned count) {
	//assert( count >= 2*4 );
	internal_scale2x_16_mmx_single(dst0, src0, src1, src2, count);
	internal_scale2x_16_mmx_single(dst1, src2, src1, src0, count);
}

static void internal_scale2x_32_mmx(u32 * dst0, u32 * dst1, const u32 * src0, const u32 * src1, const u32 * src2,
				    unsigned count) {
	//assert( count >= 2*2 );
	internal_scale2x_32_mmx_single(dst0, src0, src1, src2, count);
	internal_scale2x_32_mmx_single(dst1, src2, src1, src0, count);
}
#endif

void AdMame2x(u8 * srcPtr, u32 srcPitch, u8 * deltaPtr, u8 * dstPtr, u32 dstPitch, int width, int height) {
	int count;
	u16 *dst0 = (u16 *) dstPtr;
	u16 *dst1 = dst0 + (dstPitch / 2);
	u16 *src0 = (u16 *) srcPtr;
	u16 *src1 = src0 + (srcPitch / 2);
	u16 *src2 = src1 + (srcPitch / 2);

#ifdef MMX
	if(GetMMX()) {
		internal_scale2x_16_mmx(dst0, dst1, src0, src0, src1, width);

		count = height;
		count -= 2;
		while(count) {
			dst0 += dstPitch;
			dst1 += dstPitch;
			internal_scale2x_16_mmx(dst0, dst1, src0, src1, src2, width);
			src0 = src1;
			src1 = src2;
			src2 += srcPitch / 2;
			--count;
		}
		dst0 += dstPitch;
		dst1 += dstPitch;
		internal_scale2x_16_mmx(dst0, dst1, src0, src1, src1, width);
	} else
#endif
	{
		internal_scale2x_16_def(dst0, dst1, src0, src0, src1, width);

		count = height;
		count -= 2;
		while(count) {
			dst0 += dstPitch;
			dst1 += dstPitch;
			internal_scale2x_16_def(dst0, dst1, src0, src1, src2, width);
			src0 = src1;
			src1 = src2;
			src2 += srcPitch / 2;
			--count;
		}
		dst0 += dstPitch;
		dst1 += dstPitch;
		internal_scale2x_16_def(dst0, dst1, src0, src1, src1, width);
	}
}

void AdMame2x32(u8 * srcPtr, u32 srcPitch, u8 * deltaPtr, u8 * dstPtr, u32 dstPitch, int width, int height) {
	int count;
	u32 *dst0 = (u32 *) dstPtr;
	u32 *dst1 = dst0 + (dstPitch / 4);
	u32 *src0 = (u32 *) srcPtr;
	u32 *src1 = src0 + (srcPitch / 4);
	u32 *src2 = src1 + (srcPitch / 4);

#ifdef MMX
	if(GetMMX()) {
		internal_scale2x_32_mmx(dst0, dst1, src0, src0, src1, width);

		count = height;
		count -= 2;
		while(count) {
			dst0 += dstPitch / 2;
			dst1 += dstPitch / 2;
			internal_scale2x_32_mmx(dst0, dst1, src0, src1, src2, width);
			src0 = src1;
			src1 = src2;
			src2 += srcPitch / 4;
			--count;
		}
		dst0 += dstPitch / 2;
		dst1 += dstPitch / 2;
		internal_scale2x_32_mmx(dst0, dst1, src0, src1, src1, width);
	} else
#endif
	{
		internal_scale2x_32_def(dst0, dst1, src0, src0, src1, width);

		count = height;
		count -= 2;
		while(count) {
			dst0 += dstPitch / 2;
			dst1 += dstPitch / 2;
			internal_scale2x_32_def(dst0, dst1, src0, src1, src2, width);
			src0 = src1;
			src1 = src2;
			src2 += srcPitch / 4;
			--count;
		}
		dst0 += dstPitch / 2;
		dst1 += dstPitch / 2;
		internal_scale2x_32_def(dst0, dst1, src0, src1, src1, width);
	}
}
