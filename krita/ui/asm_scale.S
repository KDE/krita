#ifdef HAVE_X86_MMX

#ifdef __EMX__
/* Due to strange behaviour of as.exe we use this macros */
/* For all OS/2 coders - please use PGCC to compile this code */
#define PR_(foo) ___##foo
#define PT_(foo,func) ___##foo,func
#define SIZE(sym)                              \
	.___end_##sym:;                        \
	.size ___##sym,.___end_##sym-___##sym; \
	.align 8;
#else
#define PR_(foo) __##foo
#define PT_(foo,func) __##foo,func
#define SIZE(sym)                           \
	.__end_##sym:;                      \
	.size __##sym,.__end_##sym-__##sym; \
	.align 8;
#endif

/*\ 
|*| MMX assembly scaling routine for Imlib2
|*| Written by Willem Monsuwe <willem@stack.nl>
\*/

.text
        .align 8
.globl PR_(mimageScale_mmx_AARGBA)
/*	.type PT_(mimageScale_mmx_AARGBA,@function) */ 


/*\ Prototype: __mimageScale_mmx_AARGBA(ImlibScaleInfo *isi, DATA32 *dest,
|*|	int dxx, int dyy, int dx, int dy, int dw, int dh, int dow, int sow)
\*/

#define isi	8(%ebp)
#define dest	12(%ebp)
#define dxx	16(%ebp)
#define dyy	20(%ebp)
#define dx	24(%ebp)
#define dy	28(%ebp)
#define dw	32(%ebp)
#define dh	36(%ebp)
#define dow	40(%ebp)
#define sow	44(%ebp)

/*\ Local variables that didn't fit in registers \*/
#define y	-4(%ebp)
#define yp	-8(%ebp)
#define yap	-12(%ebp)
#define xp	-16(%ebp)
#define xap	-20(%ebp)
#define Cx	-24(%ebp)
#define Mx	-28(%ebp)
#define Cy	-32(%ebp)
#define My	-36(%ebp)
#define sow_4	-40(%ebp)

/*\ When %edx points to ImlibScaleInfo, these are the members \*/
#define xpoints		(%edx)
#define ypoints		4(%edx)
#define xapoints	8(%edx)
#define yapoints	12(%edx)
#define xup_yup		16(%edx)

PR_(mimageScale_mmx_AARGBA):
	pushl %ebp
	movl %esp, %ebp
	subl $40, %esp
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %edi
	pushl %esi
	movl isi, %edx

	/*\ Check (dw > 0) && (dh > 0) \*/
	cmpl $0, dw
	jle .scale_leave
	cmpl $0, dh
	jle .scale_leave

	/*\ X-based array pointers point to the end; we're looping up to 0 \*/
	/*\ %edi = dest + dow * dy + dx + dw \*/
	movl dow, %eax
	imull dy, %eax
	addl dx, %eax
	addl dw, %eax
	movl dest, %edi
	leal (%edi, %eax, 4), %edi
	/*\ xp = xpoints + dxx + dw \*/
	movl dxx, %ebx
	addl dw, %ebx
	movl xpoints, %eax
	leal (%eax, %ebx, 4), %eax
	movl %eax, xp
	/*\ xap = xapoints + dxx + dw \*/
	movl xapoints, %eax
	leal (%eax, %ebx, 4), %eax
	movl %eax, xap
	/*\ y = dh \*/
	movl dh, %eax
	movl %eax, y
	/*\ yp = ypoints + dyy \*/
	movl dyy, %ebx
	movl ypoints, %eax
	leal (%eax, %ebx, 4), %eax
	movl %eax, yp
	/*\ yap = yapoints + dyy \*/
	movl yapoints, %eax
	leal (%eax, %ebx, 4), %eax
	movl %eax, yap

	pxor %mm7, %mm7

	/*\ Test xup bit \*/
	movl xup_yup, %eax
	sarl $1, %eax
	jnc .scale_x_down

.scale_x_up:
	/*\ Test yup bit \*/
	sarl $1, %eax
	jnc .scale_x_up_y_down


/*\ Scaling up both ways \*/

.scale_x_up_y_up:
	movl sow, %ebx

.up_up_loop_y:

	/*\ x = -dw \*/
	movl dw, %ecx
	negl %ecx

	/*\ %eax = *yap << 4 \*/
	movl yap, %eax
	movl (%eax), %eax
	sall $4, %eax
	jz .up_up_yap_0
	movd %eax, %mm1
	punpcklwd %mm1, %mm1
	punpckldq %mm1, %mm1

.up_up_loop1_x:
	/*\ %esi = *yp + xp[x] \*/
	movl yp, %eax
	movl (%eax), %esi
	movl xp, %eax
	movl (%eax, %ecx, 4), %eax
	leal (%esi, %eax, 4), %esi

	/*\ %eax = xap[x] << 4 \*/
	movl xap, %eax
	movl (%eax, %ecx, 4), %eax
	sall $4, %eax
	jz .up_up_xap_0

	/*\ %mm0 = xap[x] << 4 \*/
	movd %eax, %mm0
	punpcklwd %mm0, %mm0
	punpckldq %mm0, %mm0

	/*\ Load and unpack four pixels in parralel
	|*| %mm2 = ptr[0],   %mm3 = ptr[1]
	|*| %mm4 = ptr[sow], %mm5 = ptr[sow + 1]
	\*/
	movq (%esi), %mm2
	movq (%esi, %ebx, 4), %mm4
	movq %mm2, %mm3
	movq %mm4, %mm5
	punpcklbw %mm7, %mm2
	punpcklbw %mm7, %mm4
	punpckhbw %mm7, %mm3
	punpckhbw %mm7, %mm5

	/*\ X interpolation: r = l + (r - l) * xap \*/
	psubw %mm2, %mm3
	psubw %mm4, %mm5
	psllw $4, %mm3
	psllw $4, %mm5
	pmulhw %mm0, %mm3
	pmulhw %mm0, %mm5
	paddw %mm2, %mm3
	paddw %mm4, %mm5
	/*\ Now %mm3 = I(ptr[0], ptr[1]), %mm5 = I(ptr[sow], ptr[sow + 1]) \*/
	jmp .up_up_common
.up_up_xap_0:
	/*\ Load and unpack two pixels
	|*| %mm3 = ptr[0], %mm5 = ptr[sow]
	\*/
	movd (%esi), %mm3
	movd (%esi, %ebx, 4), %mm5
	punpcklbw %mm7, %mm3
	punpcklbw %mm7, %mm5
.up_up_common:
	/*\ Y interpolation: d = u + (d - u) * yap \*/
	psubw %mm3, %mm5
	psllw $4, %mm5
	pmulhw %mm1, %mm5
	paddw %mm3, %mm5
	packuswb %mm5, %mm5
	movd %mm5, (%edi, %ecx, 4)

	/*\ while (++x) \*/
	incl %ecx
	jnz .up_up_loop1_x
	jmp .up_up_yap_end
.up_up_yap_0:

.up_up_loop2_x:
	/*\ %esi = *yp + xp[x] \*/
	movl yp, %eax
	movl (%eax), %esi
	movl xp, %eax
	movl (%eax, %ecx, 4), %eax
	leal (%esi, %eax, 4), %esi

	/*\ %eax = xap[x] << 4 \*/
	movl xap, %eax
	movl (%eax, %ecx, 4), %eax
	sall $4, %eax
	jz .up_up_0

	/*\ %mm0 = xap[x] << 4 \*/
	movd %eax, %mm0
	punpcklwd %mm0, %mm0
	punpckldq %mm0, %mm0

	/*\ Load and unpack two pixels in parralel
	|*| %mm2 = ptr[0], %mm3 = ptr[1]
	\*/
	movq (%esi), %mm2
	movq %mm2, %mm3
	punpcklbw %mm7, %mm2
	punpckhbw %mm7, %mm3

	/*\ X interpolation: r = l + (r - l) * xap \*/
	psubw %mm2, %mm3
	psllw $4, %mm3
	pmulhw %mm0, %mm3
	paddw %mm2, %mm3
	packuswb %mm3, %mm3
	movd %mm3, (%edi, %ecx, 4)
	jmp .up_up_1
.up_up_0:
	/*\ dptr[x] = *sptr \*/
	movl (%esi), %eax
	movl %eax, (%edi, %ecx, 4)
.up_up_1:
	incl %ecx
	jnz .up_up_loop2_x

.up_up_yap_end:
	/*\ dptr += dow \*/
	movl dow, %eax
	leal (%edi, %eax, 4), %edi
	/*\ yap++; yp++ \*/
	addl $4, yap
	addl $4, yp
	/*\ while (y--) \*/
	decl y
	jnz .up_up_loop_y

	jmp .scale_leave


/*\ Scaling down vertically \*/

.scale_x_up_y_down:
	/*\ sow_4 = sow * 4 \*/
	movl sow, %eax
	sall $2, %eax
	movl %eax, sow_4

.up_down_loop_y:

	/*\ Setup My and Cy \*/
	movl yap, %eax
	movzwl (%eax), %ebx
	movl %ebx, My
	movzwl 2(%eax), %eax
	movl %eax, Cy

	/*\ mm4 = Cy \*/
	movd %eax, %mm4
	punpcklwd %mm4, %mm4
	punpckldq %mm4, %mm4
	/*\ mm5 = My \*/
	movd %ebx, %mm5
	punpcklwd %mm5, %mm5
	punpckldq %mm5, %mm5

	/*\ x = -dw \*/
	movl dw, %ecx
	negl %ecx
.up_down_loop_x:
	/*\ %esi = *yp + xp[x] \*/
	movl yp, %eax
	movl (%eax), %esi
	movl xp, %eax
	movl (%eax, %ecx, 4), %eax
	leal (%esi, %eax, 4), %esi

	movl %esi, %eax
	/*\ v = (*p * My) >> 10 \*/
	movd (%eax), %mm0
	punpcklbw %mm7, %mm0
	psllw $6, %mm0
	pmulhw %mm5, %mm0
	
	/*\ i = 0x4000 - My \*/
	movl $0x4000, %ebx
	subl My, %ebx
	jbe 5f
	jmp 2f
1:
	/*\ p += sow; v += (*p * Cy) >> 10 \*/
	addl sow_4, %eax
	movd (%eax), %mm1
	punpcklbw %mm7, %mm1
	psllw $6, %mm1
	pmulhw %mm4, %mm1
	paddw %mm1, %mm0
	
	/*\ i -= Cy; while (i > Cy) \*/
	subl Cy, %ebx
2:
	cmpl Cy, %ebx
	jg 1b
	
	/*\ mm6 = i \*/
	movd %ebx, %mm6
	punpcklwd %mm6, %mm6
	punpckldq %mm6, %mm6
	
	/*\ p += sow; v += (*p * i) >> 10 \*/
	addl sow_4, %eax
	movd (%eax), %mm1
	punpcklbw %mm7, %mm1
	psllw $6, %mm1
	pmulhw %mm6, %mm1
	paddw %mm1, %mm0
5:
	/*\ %eax = xap[x] << 5 \*/
	movl xap, %eax
	movl (%eax, %ecx, 4), %eax
	sall $5, %eax
	jz 6f
	/*\ mm3 = xap[x] << 5 \*/
	movd %eax, %mm3
	punpcklwd %mm3, %mm3
	punpckldq %mm3, %mm3
	
	/*\ p + 1 \*/
	movl %esi, %eax
	addl $4, %eax
	/*\ vv = (*p * My) >> 10 \*/
	movd (%eax), %mm2
	punpcklbw %mm7, %mm2
	psllw $6, %mm2
	pmulhw %mm5, %mm2
	
	/*\ i = 0x4000 - My \*/
	movl $0x4000, %ebx
	subl My, %ebx
	jbe 5f
	jmp 2f
1:
	/*\ p += sow; vv += (*p * Cy) >> 10 \*/
	addl sow_4, %eax
	movd (%eax), %mm1
	punpcklbw %mm7, %mm1
	psllw $6, %mm1
	pmulhw %mm4, %mm1
	paddw %mm1, %mm2
	
	/*\ i -= Cy; while (i > Cy) \*/
	subl Cy, %ebx
2:
	cmpl Cy, %ebx
	jg 1b
	
	/*\ p += sow; v += (*p * i) >> 10 \*/
	addl sow_4, %eax
	movd (%eax), %mm1
	punpcklbw %mm7, %mm1
	psllw $6, %mm1
	pmulhw %mm6, %mm1
	paddw %mm1, %mm2
5:
	/*\ v = v + (vv - v) * xap \*/
	psubw %mm0, %mm2
	psllw $3, %mm2
	pmulhw %mm3, %mm2
	paddw %mm2, %mm0
6:
	/*\ dest[x] = v >> 4 \*/
	psrlw $4, %mm0
	packuswb %mm0, %mm0
	movd %mm0, (%edi, %ecx, 4)

	/*\ while (++x) \*/
	incl %ecx
	jnz .up_down_loop_x

	/*\ dptr += dow \*/
	movl dow, %eax
	leal (%edi, %eax, 4), %edi
	/*\ yap++; yp++ \*/
	addl $4, yap
	addl $4, yp
	/*\ while (y--) \*/
	decl y
	jnz .up_down_loop_y

	jmp .scale_leave

.scale_x_down:
	/*\ Test yup bit \*/
	sarl $1, %eax
	jnc .scale_x_down_y_down


/*\ Scaling down horizontally \*/

.scale_x_down_y_up:
	/*\ sow_4 = sow * 4 \*/
	movl sow, %eax
	sall $2, %eax
	movl %eax, sow_4

.down_up_loop_y:

	/*\ %eax = *yap << 5 \*/
	movl yap, %eax
	movl (%eax), %eax
	sall $5, %eax
	/*\ mm3 = *yap << 5 \*/
	movd %eax, %mm3
	punpcklwd %mm3, %mm3
	punpckldq %mm3, %mm3
	
	/*\ x = -dw \*/
	movl dw, %ecx
	negl %ecx
.down_up_loop_x:
	/*\ %esi = *yp + xp[x] \*/
	movl yp, %eax
	movl (%eax), %esi
	movl xp, %eax
	movl (%eax, %ecx, 4), %eax
	leal (%esi, %eax, 4), %esi

	/*\ Setup Mx and Cx \*/
	movl xap, %eax
	movzwl (%eax, %ecx, 4), %ebx
	movl %ebx, Mx
	movzwl 2(%eax, %ecx, 4), %eax
	movl %eax, Cx

	/*\ mm4 = Cx \*/
	movd %eax, %mm4
	punpcklwd %mm4, %mm4
	punpckldq %mm4, %mm4
	/*\ mm5 = Mx \*/
	movd %ebx, %mm5
	punpcklwd %mm5, %mm5
	punpckldq %mm5, %mm5

	movl %esi, %eax
	/*\ v = (*p * Mx) >> 10 \*/
	movd (%eax), %mm0
	punpcklbw %mm7, %mm0
	psllw $6, %mm0
	pmulhw %mm5, %mm0
	
	/*\ i = 0x4000 - Mx \*/
	movl $0x4000, %ebx
	subl Mx, %ebx
	jbe 5f
	jmp 2f
1:
	/*\ p += sow; v += (*p * Cx) >> 10 \*/
	addl $4, %eax
	movd (%eax), %mm1
	punpcklbw %mm7, %mm1
	psllw $6, %mm1
	pmulhw %mm4, %mm1
	paddw %mm1, %mm0
	
	/*\ i -= Cx; while (i > Cx) \*/
	subl Cx, %ebx
2:
	cmpl Cx, %ebx
	jg 1b
	
	/*\ mm6 = i \*/
	movd %ebx, %mm6
	punpcklwd %mm6, %mm6
	punpckldq %mm6, %mm6
	
	/*\ p += sow; v += (*p * i) >> 10 \*/
	addl $4, %eax
	movd (%eax), %mm1
	punpcklbw %mm7, %mm1
	psllw $6, %mm1
	pmulhw %mm6, %mm1
	paddw %mm1, %mm0
5:
	movd %mm3, %eax
	testl %eax, %eax
	jz 6f
	/*\ p + sow \*/
	movl %esi, %eax
	addl sow_4, %eax
	/*\ vv = (*p * Mx) >> 10 \*/
	movd (%eax), %mm2
	punpcklbw %mm7, %mm2
	psllw $6, %mm2
	pmulhw %mm5, %mm2
	
	/*\ i = 0x4000 - Mx \*/
	movl $0x4000, %ebx
	subl Mx, %ebx
	jbe 5f
	jmp 2f
1:
	/*\ p += sow; vv += (*p * Cx) >> 10 \*/
	addl $4, %eax
	movd (%eax), %mm1
	punpcklbw %mm7, %mm1
	psllw $6, %mm1
	pmulhw %mm4, %mm1
	paddw %mm1, %mm2
	
	/*\ i -= Cx; while (i > Cx) \*/
	subl Cx, %ebx
2:
	cmpl Cx, %ebx
	jg 1b
	
	/*\ p += sow; v += (*p * i) >> 10 \*/
	addl $4, %eax
	movd (%eax), %mm1
	punpcklbw %mm7, %mm1
	psllw $6, %mm1
	pmulhw %mm6, %mm1
	paddw %mm1, %mm2
5:
	/*\ v = v + (vv - v) * yap \*/
	psubw %mm0, %mm2
	psllw $3, %mm2
	pmulhw %mm3, %mm2
	paddw %mm2, %mm0
6:
	/*\ dest[x] = v >> 4 \*/
	psrlw $4, %mm0
	packuswb %mm0, %mm0
	movd %mm0, (%edi, %ecx, 4)

	/*\ while (++x) \*/
	incl %ecx
	jnz .down_up_loop_x

	/*\ dptr += dow \*/
	movl dow, %eax
	leal (%edi, %eax, 4), %edi
	/*\ yap++; yp++ \*/
	addl $4, yap
	addl $4, yp
	/*\ while (y--) \*/
	decl y
	jnz .down_up_loop_y

	jmp .scale_leave


/*\ Scaling down both ways \*/

.scale_x_down_y_down:
	/*\ sow_4 = sow * 4 \*/
	movl sow, %eax
	sall $2, %eax
	movl %eax, sow_4

.down_down_loop_y:

	/*\ Setup My and Cy \*/
	movl yap, %eax
	movzwl (%eax), %ebx
	movl %ebx, My
	movzwl 2(%eax), %eax
	movl %eax, Cy

	/*\ x = -dw \*/
	movl dw, %ecx
	negl %ecx
.down_down_loop_x:
	/*\ %esi = *yp + xp[x] \*/
	movl yp, %eax
	movl (%eax), %esi
	movl xp, %eax
	movl (%eax, %ecx, 4), %eax
	leal (%esi, %eax, 4), %esi

	/*\ Setup Mx and Cx \*/
	movl xap, %eax
	movzwl (%eax, %ecx, 4), %ebx
	movl %ebx, Mx
	movzwl 2(%eax, %ecx, 4), %eax
	movl %eax, Cx

	/*\ mm3 = Cx \*/
	movd %eax, %mm3
	punpcklwd %mm3, %mm3
	punpckldq %mm3, %mm3
	/*\ mm5 = Mx \*/
	movd %ebx, %mm5
	punpcklwd %mm5, %mm5
	punpckldq %mm5, %mm5
	
	/*\ p = sptr; v = (*p * Mx) >> 9 \*/
	movl %esi, %eax
	movd (%eax), %mm0
	punpcklbw %mm7, %mm0
	psllw $7, %mm0
	pmulhw %mm5, %mm0
	
	/*\ i = 0x4000 - Mx \*/
	movl $0x4000, %ebx
	subl Mx, %ebx
	jbe 5f
	jmp 2f
1:
	/*\ v += (*++p * Cx) >> 9 \*/
	addl $4, %eax
	movd (%eax), %mm1
	punpcklbw %mm7, %mm1
	psllw $7, %mm1
	pmulhw %mm3, %mm1
	paddw %mm1, %mm0
	
	/*\ i -= Cx; while (i > Cx) \*/
	subl Cx, %ebx
2:
	cmpl Cx, %ebx
	jg 1b
	
	/*\ mm6 = i \*/
	movd %ebx, %mm6
	punpcklwd %mm6, %mm6
	punpckldq %mm6, %mm6
	
	/*\ v += (*++p * i) >> 9 \*/
	addl $4, %eax
	movd (%eax), %mm1
	punpcklbw %mm7, %mm1
	psllw $7, %mm1
	pmulhw %mm6, %mm1
	paddw %mm1, %mm0
5:
	/*\ v *= My \*/
	movd My, %mm4
	punpcklwd %mm4, %mm4
	punpckldq %mm4, %mm4
	psllw $2, %mm0
	pmulhw %mm4, %mm0
	
	/*\ j = 0x4000 - My \*/
	movl $0x4000, %edx
	subl My, %edx
	jbe 6f
	jmp 4f
3:
	/*\ sptr += sow; p = sptr \*/
	addl sow_4, %esi
	movl %esi, %eax
	/*\ vx = (*p * Mx) >> 9 \*/
	movd (%eax), %mm1
	punpcklbw %mm7, %mm1
	psllw $7, %mm1
	pmulhw %mm5, %mm1
	
	/*\ i = 0x4000 - Mx \*/
	movl $0x4000, %ebx
	subl Mx, %ebx
	jbe 5f
	jmp 2f
1:
	/*\ vx += (*++p * Cx) >> 9 \*/
	addl $4, %eax
	movd (%eax), %mm2
	punpcklbw %mm7, %mm2
	psllw $7, %mm2
	pmulhw %mm3, %mm2
	paddw %mm2, %mm1
	
	/*\ i -= Cx; while (i > Cx) \*/
	subl Cx, %ebx
2:
	cmpl Cx, %ebx
	jg 1b
	
	/*\ vx += (*++p * i) >> 9 \*/
	addl $4, %eax
	movd (%eax), %mm2
	punpcklbw %mm7, %mm2
	psllw $7, %mm2
	pmulhw %mm6, %mm2
	paddw %mm2, %mm1
5:
	/*\ v += (vx * Cy) >> 14 \*/
	movd Cy, %mm4
	punpcklwd %mm4, %mm4
	punpckldq %mm4, %mm4
	psllw $2, %mm1
	pmulhw %mm4, %mm1
	paddw %mm1, %mm0
	
	/*\ j -= Cy; while (j > Cy) \*/
	subl Cy, %edx
4:
	cmpl Cy, %edx
	jg 3b
	
	/*\ sptr += sow; p = sptr \*/
	addl sow_4, %esi
	movl %esi, %eax
	/*\ vx = (*p * Mx) >> 9 \*/
	movd (%eax), %mm1
	punpcklbw %mm7, %mm1
	psllw $7, %mm1
	pmulhw %mm5, %mm1
	
	/*\ i = 0x4000 - Mx \*/
	movl $0x4000, %ebx
	subl Mx, %ebx
	jbe 5f
	jmp 2f
1:
	/*\ vx += (*++p * Cx) >> 9 \*/
	addl $4, %eax
	movd (%eax), %mm2
	punpcklbw %mm7, %mm2
	psllw $7, %mm2
	pmulhw %mm3, %mm2
	paddw %mm2, %mm1
	
	/*\ i -= Cx; while (i > Cx) \*/
	subl Cx, %ebx
2:
	cmpl Cx, %ebx
	jg 1b
	
	/*\ vx += (*++p * i) >> 9 \*/
	addl $4, %eax
	movd (%eax), %mm2
	punpcklbw %mm7, %mm2
	psllw $7, %mm2
	pmulhw %mm6, %mm2
	paddw %mm2, %mm1
5:
	/*\ v += (vx * j) >> 14 \*/
	movd %edx, %mm4
	punpcklwd %mm4, %mm4
	punpckldq %mm4, %mm4
	psllw $2, %mm1
	pmulhw %mm4, %mm1
	paddw %mm1, %mm0
6:
	/*\ dptr[x] = mm0 >> 5 \*/
	psrlw $5, %mm0
	packuswb %mm0, %mm0
	movd %mm0, (%edi, %ecx, 4)

	/*\ while (++x) \*/
	incl %ecx
	jnz .down_down_loop_x

	/*\ dptr += dow \*/
	movl dow, %eax
	leal (%edi, %eax, 4), %edi
	/*\ yap++; yp++ \*/
	addl $4, yap
	addl $4, yp
	/*\ while (y--) \*/
	decl y
	jnz .down_down_loop_y

	jmp .scale_leave

.scale_leave:
	emms
	popl %esi
	popl %edi
	popl %edx
	popl %ecx
	popl %ebx
	movl %ebp, %esp
	popl %ebp
	ret

SIZE(mimageScale_mmx_AARGBA)

#endif

.section	.note.GNU-stack,"",%progbits
