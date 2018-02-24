; ============================================================
; module: mmx alpha blending
;
; Copyright (C) 2000-2003 Twilight 3D Finland Oy Ltd.
; http://www.twilight3d.com
; ============================================================

%macro r_begin 0
	push    eax
	mov     eax, esp
	add     eax,4
	pushad
	mov     edi,[eax+4]    ; dest
	mov     esi,[eax+8]    ; sr0
	mov     edx,[eax+12]   ; sr1
	mov     ecx,[eax+16]   ; count
	
	%ifdef __LINUX__
	
	movq    mm4,[mmx_mult0]
	movq    mm5,[mmx_mult1]
	
	%else
	%ifdef __BEOS__
	
	movq    mm4,[mmx_mult0]
	movq    mm5,[mmx_mult1]
	
	%else
	
	movq    mm4,[_mmx_mult0]
	movq    mm5,[_mmx_mult1]
	
	%endif
	%endif
	
	movq    mm6,[mask64]
	pxor    mm7,mm7
%endmacro


%macro r_end 0
	emms
	popad
	pop     eax
	ret
%endmacro


	segment .data

	global _mmx_mult0
	global mmx_mult0_
	global mmx_mult0

_mmx_mult0:
mmx_mult0_:
mmx_mult0:	dd	0x0,0x0

	global _mmx_mult1
	global mmx_mult1_
	global mmx_mult1

_mmx_mult1:
mmx_mult1_:
mmx_mult1:	dd	0x0,0x0

mask64:     dd  0x00ff00ff,0x00ff00ff


	segment .text

	; function pointer table

	global _mmx_alpha
	global mmx_alpha_
	global mmx_alpha

_mmx_alpha:
mmx_alpha_:
mmx_alpha:
	dd mmx_zero_zero
	dd mmx_zero_one
	dd mmx_zero_srccolor
	dd mmx_zero_srcinvcolor
	dd mmx_zero_srcalpha
	dd mmx_zero_srcinvalpha
	dd mmx_zero_destcolor
	dd mmx_zero_destinvcolor
	dd mmx_zero_destalpha
	dd mmx_zero_destinvalpha
	dd mmx_one_zero
	dd mmx_one_one
	dd mmx_one_srccolor
	dd mmx_one_srcinvcolor
	dd mmx_one_srcalpha
	dd mmx_one_srcinvalpha
	dd mmx_one_destcolor
	dd mmx_one_destinvcolor
	dd mmx_one_destalpha
	dd mmx_one_destinvalpha
	dd mmx_srccolor_zero
	dd mmx_srccolor_one
	dd mmx_srccolor_srccolor
	dd mmx_srccolor_srcinvcolor
	dd mmx_srccolor_srcalpha
	dd mmx_srccolor_srcinvalpha
	dd mmx_srccolor_destcolor
	dd mmx_srccolor_destinvcolor
	dd mmx_srccolor_destalpha
	dd mmx_srccolor_destinvalpha
	dd mmx_srcinvcolor_zero
	dd mmx_srcinvcolor_one
	dd mmx_srcinvcolor_srccolor
	dd mmx_srcinvcolor_srcinvcolor
	dd mmx_srcinvcolor_srcalpha
	dd mmx_srcinvcolor_srcinvalpha
	dd mmx_srcinvcolor_destcolor
	dd mmx_srcinvcolor_destinvcolor
	dd mmx_srcinvcolor_destalpha
	dd mmx_srcinvcolor_destinvalpha
	dd mmx_srcalpha_zero
	dd mmx_srcalpha_one
	dd mmx_srcalpha_srccolor
	dd mmx_srcalpha_srcinvcolor
	dd mmx_srcalpha_srcalpha
	dd mmx_srcalpha_srcinvalpha
	dd mmx_srcalpha_destcolor
	dd mmx_srcalpha_destinvcolor
	dd mmx_srcalpha_destalpha
	dd mmx_srcalpha_destinvalpha
	dd mmx_srcinvalpha_zero
	dd mmx_srcinvalpha_one
	dd mmx_srcinvalpha_srccolor
	dd mmx_srcinvalpha_srcinvcolor
	dd mmx_srcinvalpha_srcalpha
	dd mmx_srcinvalpha_srcinvalpha
	dd mmx_srcinvalpha_destcolor
	dd mmx_srcinvalpha_destinvcolor
	dd mmx_srcinvalpha_destalpha
	dd mmx_srcinvalpha_destinvalpha
	dd mmx_destcolor_zero
	dd mmx_destcolor_one
	dd mmx_destcolor_srccolor
	dd mmx_destcolor_srcinvcolor
	dd mmx_destcolor_srcalpha
	dd mmx_destcolor_srcinvalpha
	dd mmx_destcolor_destcolor
	dd mmx_destcolor_destinvcolor
	dd mmx_destcolor_destalpha
	dd mmx_destcolor_destinvalpha
	dd mmx_destinvcolor_zero
	dd mmx_destinvcolor_one
	dd mmx_destinvcolor_srccolor
	dd mmx_destinvcolor_srcinvcolor
	dd mmx_destinvcolor_srcalpha
	dd mmx_destinvcolor_srcinvalpha
	dd mmx_destinvcolor_destcolor
	dd mmx_destinvcolor_destinvcolor
	dd mmx_destinvcolor_destalpha
	dd mmx_destinvcolor_destinvalpha
	dd mmx_destalpha_zero
	dd mmx_destalpha_one
	dd mmx_destalpha_srccolor
	dd mmx_destalpha_srcinvcolor
	dd mmx_destalpha_srcalpha
	dd mmx_destalpha_srcinvalpha
	dd mmx_destalpha_destcolor
	dd mmx_destalpha_destinvcolor
	dd mmx_destalpha_destalpha
	dd mmx_destalpha_destinvalpha
	dd mmx_destinvalpha_zero
	dd mmx_destinvalpha_one
	dd mmx_destinvalpha_srccolor
	dd mmx_destinvalpha_srcinvcolor
	dd mmx_destinvalpha_srcalpha
	dd mmx_destinvalpha_srcinvalpha
	dd mmx_destinvalpha_destcolor
	dd mmx_destinvalpha_destinvcolor
	dd mmx_destinvalpha_destalpha
	dd mmx_destinvalpha_destinvalpha


mmx_zero_zero:
	r_begin
	push        ecx
	shr         ecx,1
	jz          .noxloop
	.xloop:
	movq        [edi],mm7
	add         edi,8
	dec         ecx
	jnz         .xloop
	.noxloop:
	pop         ecx
	and         ecx,1
	xor         eax,eax
	rep         stosd
	r_end


mmx_zero_one:
	r_begin
	push        ecx
	shr         ecx,1
	jz          .noxloop
	.xloop:
	movq        mm0,[edx]
	add         edx,8
	movq        [edi],mm0
	add         edi,8
	dec         ecx
	jnz         .xloop
	.noxloop:
	mov			esi,edx
	pop         ecx
	and         ecx,1
	rep         movsd
	r_end


mmx_zero_srccolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm0,mm1
	add         esi,4
	psrlw       mm0,8
	add         edx,4
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_zero_srcinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pxor        mm0,mm6
	add         esi,4
	pmullw      mm0,mm1
	add         edx,4
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_zero_srcalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm0
	add         esi,4
	punpckhwd   mm0,mm0
	punpcklbw   mm1,mm7
	punpckhbw   mm0,mm7
	pmullw      mm0,mm4
	psrlw       mm0,8
	add         edx,4
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_zero_srcinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm0
	add         esi,4
	punpckhwd   mm0,mm0
	punpcklbw   mm1,mm7
	punpckhbw   mm0,mm7
	pmullw      mm0,mm4
	psrlw       mm0,8
	pxor        mm0,mm6
	add         edx,4
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_zero_destcolor:
	r_begin
	.xloop:
	movd        mm0,[edx]
	punpcklbw   mm0,mm7
	pmullw      mm0,mm0
	psrlw       mm0,8
	packuswb    mm0,mm7
	add         edx,4
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_zero_destinvcolor:
	r_begin
	.xloop:
	movd        mm0,[edx]
	punpcklbw   mm0,mm7
	movq        mm1,mm0
	pxor        mm1,mm6
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	add         edx,4
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_zero_destalpha:
	r_begin
	.xloop:
	movd        mm0,[edx]
	movq        mm1,mm0
	punpcklbw   mm0,mm0
	add         edx,4
	punpckhwd   mm0,mm0
	punpckhbw   mm0,mm7
	punpckhbw   mm1,mm7
	pmullw      mm0,mm5
	psrlw       mm0,8
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_zero_destinvalpha:
	r_begin
	.xloop:
	movd        mm0,[edx]
	movq        mm1,mm0
	punpcklbw   mm0,mm0
	add         edx,4
	punpckhwd   mm0,mm0
	punpckhbw   mm0,mm7
	punpckhbw   mm1,mm7
	pmullw      mm0,mm5
	psrlw       mm0,8
	pxor        mm0,mm6
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_one_zero:
	r_begin
	push        ecx
	shr         ecx,1
	jz          .noxloop
	.xloop:
	movq        mm0,[esi]
	add         esi,8
	movq        [edi],mm0
	add         edi,8
	dec         ecx
	jnz         .xloop
	.noxloop:
	pop         ecx
	and         ecx,1
	rep         movsd
	r_end


mmx_one_one:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	add         esi,4
	paddusb     mm0,mm1
	add         edx,4
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_one_srccolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	paddusb     mm0,mm2
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_one_srcinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pxor        mm0,mm6
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	paddusb     mm0,mm2
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_one_srcalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm0,mm0
	add         esi,4
	punpckhwd   mm0,mm0
	punpcklbw   mm1,mm7
	punpckhbw   mm0,mm7
	pmullw      mm0,mm4
	psrlw       mm0,8
	add         edx,4
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	paddusb     mm0,mm2
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_one_srcinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm0,mm0
	add         esi,4
	punpckhwd   mm0,mm0
	punpcklbw   mm1,mm7
	punpckhbw   mm0,mm7
	pmullw      mm0,mm4
	psrlw       mm0,8
	pxor        mm0,mm6
	add         edx,4
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	paddusb     mm0,mm2
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_one_destcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm1,mm7
	pmullw      mm1,mm1
	psrlw       mm1,8
	packuswb    mm1,mm7
	paddusb     mm0,mm1
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_one_destinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm1,mm7
	movq        mm2,mm1
	pxor        mm2,mm6
	pmullw      mm1,mm2
	psrlw       mm1,8
	packuswb    mm1,mm7
	paddusb     mm0,mm1
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_one_destalpha:
	r_begin
	.xloop:
	movd        mm0,[edx]
	movd        mm2,[esi]
	movq        mm1,mm0
	punpcklbw   mm0,mm0
	add         edx,4
	punpckhwd   mm0,mm0
	punpckhbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm0,mm5
	psrlw       mm0,8
	add         esi,4
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	paddusb     mm0,mm2
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_one_destinvalpha:
	r_begin
	.xloop:
	movd        mm0,[edx]
	movd        mm2,[esi]
	movq        mm1,mm0
	punpcklbw   mm0,mm0
	add         edx,4
	punpckhwd   mm0,mm0
	punpckhbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm0,mm5
	psrlw       mm0,8
	pxor        mm0,mm6
	add         esi,4
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	paddusb     mm0,mm2
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srccolor_zero:
	r_begin
	.xloop:
	movd        mm0,[esi]
	punpcklbw   mm0,mm7
	pmullw      mm0,mm0
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srccolor_one:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	pmullw      mm0,mm0
	psrlw       mm0,8
	packuswb    mm0,mm7
	paddusb     mm0,mm1
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srccolor_srccolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm1,mm0
	pmullw      mm0,mm0
	psrlw       mm1,8
	psrlw       mm0,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srccolor_srcinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	movq        mm2,mm0
	pxor        mm2,mm6
	pmullw      mm0,mm0
	pmullw      mm1,mm2
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srccolor_srcalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	pmullw      mm0,mm0
	punpckhbw   mm2,mm7
	add         esi,4
	pmullw      mm2,mm4
	add         edx,4
	psrlw       mm2,8
	pmullw      mm1,mm2
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srccolor_srcinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	pmullw      mm0,mm0
	punpckhbw   mm2,mm7
	add         esi,4
	pmullw      mm2,mm4
	add         edx,4
	psrlw       mm2,8
	pxor        mm2,mm6
	pmullw      mm1,mm2
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srccolor_destcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm0,mm0
	pmullw      mm1,mm1
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srccolor_destinvcolor:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	punpcklbw   mm1,mm7
	punpcklbw   mm0,mm7
	movq        mm2,mm1
	pmullw      mm0,mm0
	pxor        mm2,mm6
	pmullw      mm1,mm2
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srccolor_destalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhbw   mm2,mm7
	add         esi,4
	pmullw      mm2,mm5
	add         edx,4
	psrlw       mm2,8
	pmullw      mm0,mm0
	pmullw      mm1,mm2
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srccolor_destinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhbw   mm2,mm7
	add         esi,4
	pmullw      mm2,mm5
	add         edx,4
	psrlw       mm2,8
	pxor        mm2,mm6
	pmullw      mm0,mm0
	pmullw      mm1,mm2
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvcolor_zero:
	r_begin
	.xloop:
	movd        mm0,[esi]
	punpcklbw   mm0,mm7
	movq        mm1,mm0
	pxor        mm1,mm6
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvcolor_one:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm2,[edx]
	punpcklbw   mm0,mm7
	movq        mm1,mm0
	pxor        mm1,mm6
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	paddusb     mm0,mm2
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvcolor_srccolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	movq        mm2,mm0
	pxor        mm2,mm6
	pmullw      mm1,mm0
	pmullw      mm0,mm2
	psrlw       mm1,8
	psrlw       mm0,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvcolor_srcinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	movq        mm2,mm0
	pxor        mm2,mm6
	pmullw      mm1,mm2
	pmullw      mm0,mm2
	psrlw       mm1,8
	psrlw       mm0,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvcolor_srcalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm3,mm0
	movq        mm2,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm3,mm7
	punpckhbw   mm2,mm7
	pxor        mm3,mm6
	pmullw      mm2,mm4
	pmullw      mm0,mm3
	psrlw       mm2,8
	add         esi,4
	pmullw      mm1,mm2
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	add         edx,4
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvcolor_srcinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm3,mm0
	movq        mm2,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm3,mm7
	punpckhbw   mm2,mm7
	pxor        mm3,mm6
	pmullw      mm2,mm4
	pmullw      mm0,mm3
	psrlw       mm2,8
	pxor        mm2,mm6
	add         esi,4
	pmullw      mm1,mm2
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	add         edx,4
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvcolor_destcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	movq        mm2,mm0
	pxor        mm2,mm6
	pmullw      mm0,mm2
	pmullw      mm1,mm1
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvcolor_destinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	movq        mm2,mm0
	movq        mm3,mm1
	pxor        mm2,mm6
	pxor        mm3,mm6
	pmullw      mm0,mm2
	pmullw      mm1,mm3
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvcolor_destalpha:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	movq        mm2,mm1
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	movq        mm3,mm0
	punpckhbw   mm2,mm7
	add         esi,4
	pmullw      mm2,mm5
	add         edx,4
	pxor        mm3,mm6
	psrlw       mm2,8
	pmullw      mm0,mm3
	pmullw      mm1,mm2
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end

mmx_srcinvcolor_destinvalpha:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	movq        mm2,mm1
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	movq        mm3,mm0
	punpckhbw   mm2,mm7
	pxor        mm3,mm6
	pmullw      mm2,mm5
	pmullw      mm0,mm3
	psrlw       mm2,8
	add         esi,4
	pxor        mm2,mm6
	add         edx,4
	pmullw      mm1,mm2
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcalpha_zero:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movq        mm2,mm0
	punpcklbw   mm2,mm2
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	punpcklbw   mm0,mm7
	pmullw      mm2,mm4
	psrlw       mm2,8
	pmullw      mm0,mm2
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcalpha_one:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	add         esi,4
	punpcklbw   mm2,mm2
	add         edx,4
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	pmullw      mm2,mm4
	psrlw       mm2,8
	punpcklbw   mm0,mm7
	pmullw      mm0,mm2
	psrlw       mm0,8
	packuswb    mm0,mm7
	paddusb     mm0,mm1
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcalpha_srccolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm1,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	pmullw      mm1,mm0
	punpckhbw   mm2,mm7
	psrlw       mm1,8
	pmullw      mm2,mm4
	add         esi,4
	psrlw       mm2,8
	add         edx,4
	pmullw      mm0,mm2
	psrlw       mm0,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcalpha_srcinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm1,mm7
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	pxor        mm0,mm6
	punpckhwd   mm2,mm2
	pmullw      mm1,mm0
	punpckhbw   mm2,mm7
	psrlw       mm1,8
	pmullw      mm2,mm4
	pxor        mm0,mm6
	add         esi,4
	psrlw       mm2,8
	add         edx,4
	pmullw      mm0,mm2
	psrlw       mm0,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcalpha_srcalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	add         esi,4
	punpckhbw   mm2,mm7
	add         edx,4
	pmullw      mm2,mm4
	psrlw       mm2,8
	pmullw      mm0,mm2
	pmullw      mm1,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcalpha_srcinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	add         esi,4
	punpckhbw   mm2,mm7
	add         edx,4
	pmullw      mm2,mm4
	psrlw       mm2,8
	pmullw      mm0,mm2
	pxor        mm2,mm6
	pmullw      mm1,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcalpha_destcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	add         esi,4
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhbw   mm2,mm7
	add         edx,4
	pmullw      mm2,mm4
	psrlw       mm2,8
	pmullw      mm0,mm2
	pmullw      mm1,mm1
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcalpha_destinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	add         esi,4
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhbw   mm2,mm7
	add         edx,4
	pmullw      mm2,mm4
	movq        mm3,mm1
	psrlw       mm2,8
	pxor        mm3,mm6
	pmullw      mm0,mm2
	pmullw      mm1,mm3
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcalpha_destalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	movq        mm3,mm1
	punpcklbw   mm2,mm2
	punpcklbw   mm3,mm3
	punpckhwd   mm2,mm2
	punpckhwd   mm3,mm3
	punpckhbw   mm2,mm7
	punpckhbw   mm3,mm7
	pmullw      mm2,mm4
	pmullw      mm3,mm5
	psrlw       mm2,8
	psrlw       mm3,8
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm0,mm2
	pmullw      mm1,mm3
	add         esi,4
	paddusw     mm0,mm1
	add         edx,4
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcalpha_destinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	movq        mm3,mm1
	punpcklbw   mm2,mm2
	punpcklbw   mm3,mm3
	punpckhwd   mm2,mm2
	punpckhwd   mm3,mm3
	punpckhbw   mm2,mm7
	punpckhbw   mm3,mm7
	pmullw      mm2,mm4
	pmullw      mm3,mm5
	psrlw       mm2,8
	psrlw       mm3,8
	pxor        mm3,mm6
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm0,mm2
	pmullw      mm1,mm3
	add         esi,4
	paddusw     mm0,mm1
	add         edx,4
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvalpha_zero:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movq        mm2,mm0
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	add         esi,4
	punpckhbw   mm2,mm7
	pmullw      mm2,mm4
	psrlw       mm2,8
	pxor        mm2,mm6
	pmullw      mm0,mm2
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvalpha_one:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	add         esi,4
	punpckhbw   mm2,mm7
	add         edx,4
	pmullw      mm2,mm4
	psrlw       mm2,8
	pxor        mm2,mm6
	pmullw      mm0,mm2
	psrlw       mm0,8
	packuswb    mm0,mm7
	paddusb     mm0,mm1
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvalpha_srccolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	pmullw      mm1,mm0
	pmullw      mm2,mm4
	psrlw       mm2,8
	add         esi,4
	pxor        mm2,mm6
	add         edx,4
	pmullw      mm0,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvalpha_srcinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	pxor        mm0,mm6
	punpckhbw   mm2,mm7
	pmullw      mm1,mm0
	pmullw      mm2,mm4
	psrlw       mm2,8
	add         esi,4
	pxor        mm0,mm6
	pxor        mm2,mm6
	add         edx,4
	pmullw      mm0,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvalpha_srcalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm1,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	add         esi,4
	pmullw      mm2,mm4
	psrlw       mm2,8
	add         edx,4
	pmullw      mm1,mm2
	pxor        mm2,mm6
	pmullw      mm0,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvalpha_srcinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm1,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	add         esi,4
	pmullw      mm2,mm4
	psrlw       mm2,8
	add         edx,4
	pxor        mm2,mm6
	pmullw      mm1,mm2
	pmullw      mm0,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvalpha_destcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	add         esi,4
	pmullw      mm2,mm4
	add         edx,4
	psrlw       mm2,8
	pxor        mm2,mm6
	pmullw      mm0,mm2
	pmullw      mm1,mm1
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvalpha_destinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	movq        mm3,mm1
	punpckhbw   mm2,mm7
	add         esi,4
	pmullw      mm2,mm4
	add         edx,4
	psrlw       mm2,8
	pxor        mm3,mm6
	pxor        mm2,mm6
	pmullw      mm0,mm2
	pmullw      mm1,mm3
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvalpha_destalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	movq        mm3,mm1
	punpcklbw   mm2,mm2
	punpcklbw   mm3,mm3
	punpckhwd   mm2,mm2
	punpckhwd   mm3,mm3
	punpckhbw   mm2,mm7
	punpckhbw   mm3,mm7
	pmullw      mm2,mm4
	pmullw      mm3,mm5
	psrlw       mm2,8
	psrlw       mm3,8
	pxor        mm2,mm6
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm0,mm2
	pmullw      mm1,mm3
	paddusw     mm0,mm1
	psrlw       mm0,8
	add         esi,4
	add         edx,4
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_srcinvalpha_destinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	movq        mm3,mm1
	punpcklbw   mm2,mm2
	punpcklbw   mm3,mm3
	punpckhwd   mm2,mm2
	punpckhwd   mm3,mm3
	punpckhbw   mm2,mm7
	punpckhbw   mm3,mm7
	pmullw      mm2,mm4
	pmullw      mm3,mm5
	psrlw       mm2,8
	psrlw       mm3,8
	pxor        mm2,mm6
	pxor        mm3,mm6
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm0,mm2
	pmullw      mm1,mm3
	paddusw     mm0,mm1
	add         esi,4
	psrlw       mm0,8
	add         edx,4
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destcolor_zero:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destcolor_one:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm0,mm1
	psrlw       mm0,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destcolor_srccolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm0,mm1
	psrlw       mm0,8
	paddusw     mm0,mm0
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destcolor_srcinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	movq        mm2,mm0
	pmullw      mm0,mm1
	pxor        mm2,mm6
	pmullw      mm1,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destcolor_srcalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhbw   mm2,mm7
	pmullw      mm0,mm1
	pmullw      mm2,mm4
	add         esi,4
	psrlw       mm2,8
	add         edx,4
	pmullw      mm1,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destcolor_srcinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhbw   mm2,mm7
	pmullw      mm0,mm1
	pmullw      mm2,mm4
	add         esi,4
	psrlw       mm2,8
	add         edx,4
	pxor        mm2,mm6
	pmullw      mm1,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destcolor_destcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pmullw      mm0,mm1
	pmullw      mm1,mm1
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destcolor_destinvcolor:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	punpcklbw   mm1,mm7
	punpcklbw   mm0,mm7
	movq        mm2,mm1
	pmullw      mm0,mm1
	pxor        mm2,mm6
	pmullw      mm1,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destcolor_destalpha:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	movq        mm3,mm1
	punpcklbw   mm0,mm7
	punpcklbw   mm3,mm3
	punpcklbw   mm1,mm7
	punpckhwd   mm3,mm3
	pmullw      mm0,mm1
	punpckhbw   mm3,mm7
	add         esi,4
	pmullw      mm3,mm5
	psrlw       mm3,8
	add         edx,4
	pmullw      mm1,mm3
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destcolor_destinvalpha:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	movq        mm3,mm1
	punpcklbw   mm0,mm7
	punpcklbw   mm3,mm3
	punpcklbw   mm1,mm7
	punpckhwd   mm3,mm3
	punpckhbw   mm3,mm7
	pmullw      mm0,mm1
	pmullw      mm3,mm5
	psrlw       mm3,8
	pxor        mm3,mm6
	add         esi,4
	pmullw      mm1,mm3
	paddusw     mm0,mm1
	psrlw       mm0,8
	add         edx,4
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvcolor_zero:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	punpcklbw   mm1,mm7
	punpcklbw   mm0,mm7
	pxor        mm1,mm6
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvcolor_one:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	punpcklbw   mm1,mm7
	punpcklbw   mm0,mm7
	pxor        mm1,mm6
	pmullw      mm0,mm1
	pxor        mm1,mm6
	psrlw       mm0,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvcolor_srccolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pxor        mm1,mm6
	movq        mm2,mm0
	pmullw      mm0,mm1
	pxor        mm1,mm6
	psrlw       mm0,8
	pmullw      mm1,mm2
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvcolor_srcinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pxor        mm1,mm6
	movq        mm2,mm0
	pxor        mm2,mm6
	pmullw      mm0,mm1
	pxor        mm1,mm6
	psrlw       mm0,8
	pmullw      mm1,mm2
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvcolor_srcalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhbw   mm2,mm7
	pxor        mm1,mm6
	pmullw      mm0,mm1
	pxor        mm1,mm6
	pmullw      mm2,mm4
	add         esi,4
	psrlw       mm2,8
	add         edx,4
	pmullw      mm1,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvcolor_srcinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhbw   mm2,mm7
	pxor        mm1,mm6
	pmullw      mm0,mm1
	pxor        mm1,mm6
	pmullw      mm2,mm4
	add         esi,4
	psrlw       mm2,8
	add         edx,4
	pxor        mm2,mm6
	pmullw      mm1,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvcolor_destcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	pxor        mm1,mm6
	pmullw      mm0,mm1
	psrlw       mm0,8
	pxor        mm1,mm6
	pmullw      mm1,mm1
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvcolor_destinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	punpcklbw   mm0,mm7
	punpcklbw   mm1,mm7
	movq        mm2,mm1
	pxor        mm1,mm6
	pmullw      mm0,mm1
	psrlw       mm0,8
	pmullw      mm1,mm2
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         esi,4
	add         edi,4
	add         edx,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvcolor_destalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhbw   mm2,mm7
	pxor        mm1,mm6
	pmullw      mm0,mm1
	pxor        mm1,mm6
	pmullw      mm2,mm5
	add         esi,4
	psrlw       mm2,8
	add         edx,4
	pmullw      mm1,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvcolor_destinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhbw   mm2,mm7
	pxor        mm1,mm6
	pmullw      mm0,mm1
	pxor        mm1,mm6
	pmullw      mm2,mm5
	add         esi,4
	psrlw       mm2,8
	add         edx,4
	pxor        mm2,mm6
	pmullw      mm1,mm2
	psrlw       mm0,8
	psrlw       mm1,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destalpha_zero:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	punpcklbw   mm1,mm1
	punpcklbw   mm0,mm7
	punpckhwd   mm1,mm1
	punpckhbw   mm1,mm7
	add         esi,4
	pmullw      mm1,mm5
	psrlw       mm1,8
	add         edx,4
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destalpha_one:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	movq        mm2,mm1
	punpcklbw   mm1,mm1
	punpcklbw   mm0,mm7
	punpckhwd   mm1,mm1
	punpckhbw   mm1,mm7
	add         esi,4
	pmullw      mm1,mm5
	psrlw       mm1,8
	add         edx,4
	pmullw      mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	paddusb     mm0,mm2
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destalpha_srccolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	punpcklbw   mm1,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	pmullw      mm1,mm0
	punpckhbw   mm2,mm7
	psrlw       mm1,8
	pmullw      mm2,mm5
	add         esi,4
	psrlw       mm2,8
	add         edx,4
	pmullw      mm0,mm2
	psrlw       mm0,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destalpha_srcinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	punpcklbw   mm1,mm7
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	pxor        mm0,mm6
	punpckhwd   mm2,mm2
	pmullw      mm1,mm0
	punpckhbw   mm2,mm7
	psrlw       mm1,8
	pmullw      mm2,mm5
	pxor        mm0,mm6
	add         esi,4
	psrlw       mm2,8
	add         edx,4
	pmullw      mm0,mm2
	psrlw       mm0,8
	paddusw     mm0,mm1
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destalpha_srcalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	movq        mm3,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm3,mm3
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	punpckhwd   mm3,mm3
	add         esi,4
	punpckhbw   mm2,mm7
	punpckhbw   mm3,mm7
	add         edx,4
	pmullw      mm2,mm5
	psrlw       mm2,8
	pmullw      mm3,mm4
	psrlw       mm3,8
	pmullw      mm0,mm2
	pmullw      mm1,mm3
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destalpha_srcinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	movq        mm3,mm0
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm3,mm3
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	punpckhwd   mm3,mm3
	add         esi,4
	punpckhbw   mm2,mm7
	punpckhbw   mm3,mm7
	add         edx,4
	pmullw      mm2,mm5
	psrlw       mm2,8
	pmullw      mm3,mm4
	psrlw       mm3,8
	pmullw      mm0,mm2
	pxor        mm3,mm6
	pmullw      mm1,mm3
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destalpha_destcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	add         esi,4
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhbw   mm2,mm7
	add         edx,4
	pmullw      mm2,mm5
	psrlw       mm2,8
	pmullw      mm0,mm2
	pmullw      mm1,mm1
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destalpha_destinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	add         esi,4
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhbw   mm2,mm7
	add         edx,4
	pmullw      mm2,mm5
	movq        mm3,mm1
	psrlw       mm2,8
	pxor        mm3,mm6
	pmullw      mm0,mm2
	pmullw      mm1,mm3
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destalpha_destalpha:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	movq        mm2,mm1
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	add         esi,4
	pmullw      mm2,mm5
	psrlw       mm2,8
	add         edx,4
	pmullw      mm0,mm2
	pmullw      mm1,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destalpha_destinvalpha:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	movq        mm2,mm1
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	add         esi,4
	pmullw      mm2,mm5
	psrlw       mm2,8
	add         edx,4
	pmullw      mm0,mm2
	pxor        mm2,mm6
	pmullw      mm1,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvalpha_zero:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movq        mm2,[edx]
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	add         esi,4
	punpckhbw   mm2,mm7
	pmullw      mm2,mm5
	add         edx,4
	psrlw       mm2,8
	pxor        mm2,mm6
	pmullw      mm0,mm2
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvalpha_one:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	punpcklbw   mm2,mm2
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	add         esi,4
	punpckhbw   mm2,mm7
	add         edx,4
	pmullw      mm2,mm5
	psrlw       mm2,8
	pxor        mm2,mm6
	pmullw      mm0,mm2
	psrlw       mm0,8
	packuswb    mm0,mm7
	paddusb     mm0,mm1
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvalpha_srccolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	pmullw      mm1,mm0
	pmullw      mm2,mm5
	psrlw       mm2,8
	add         esi,4
	pxor        mm2,mm6
	add         edx,4
	pmullw      mm0,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvalpha_srcinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	pxor        mm0,mm6
	punpckhbw   mm2,mm7
	pmullw      mm1,mm0
	pmullw      mm2,mm5
	psrlw       mm2,8
	add         esi,4
	pxor        mm0,mm6
	pxor        mm2,mm6
	add         edx,4
	pmullw      mm0,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvalpha_srcalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	movq        mm3,mm1
	punpcklbw   mm1,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm3,mm3
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	punpckhwd   mm3,mm3
	punpckhbw   mm3,mm7
	add         esi,4
	pmullw      mm2,mm4
	psrlw       mm2,8
	pmullw      mm3,mm5
	psrlw       mm3,8
	add         edx,4
	pmullw      mm1,mm2
	pxor        mm3,mm6
	pmullw      mm0,mm3
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvalpha_srcinvalpha:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm0
	movq        mm3,mm1
	punpcklbw   mm1,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm3,mm3
	punpcklbw   mm0,mm7
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	punpckhwd   mm3,mm3
	punpckhbw   mm3,mm7
	add         esi,4
	pmullw      mm2,mm4
	psrlw       mm2,8
	pmullw      mm3,mm5
	psrlw       mm3,8
	add         edx,4
	pxor        mm2,mm6
	pxor        mm3,mm6
	pmullw      mm1,mm2
	pmullw      mm0,mm3
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvalpha_destcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	add         esi,4
	pmullw      mm2,mm5
	add         edx,4
	psrlw       mm2,8
	pxor        mm2,mm6
	pmullw      mm0,mm2
	pmullw      mm1,mm1
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvalpha_destinvcolor:
	r_begin
	.xloop:
	movd        mm0,[esi]
	movd        mm1,[edx]
	movq        mm2,mm1
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	movq        mm3,mm1
	punpckhbw   mm2,mm7
	add         esi,4
	pmullw      mm2,mm5
	add         edx,4
	psrlw       mm2,8
	pxor        mm3,mm6
	pxor        mm2,mm6
	pmullw      mm0,mm2
	pmullw      mm1,mm3
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvalpha_destalpha:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	movq        mm2,mm1
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	pmullw      mm2,mm5
	psrlw       mm2,8
	add         esi,4
	pmullw      mm1,mm2
	pxor        mm2,mm6
	add         edx,4
	pmullw      mm0,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end


mmx_destinvalpha_destinvalpha:
	r_begin
	.xloop:
	movd        mm1,[edx]
	movd        mm0,[esi]
	movq        mm2,mm1
	punpcklbw   mm0,mm7
	punpcklbw   mm2,mm2
	punpcklbw   mm1,mm7
	punpckhwd   mm2,mm2
	punpckhbw   mm2,mm7
	pmullw      mm2,mm5
	psrlw       mm2,8
	pxor        mm2,mm6
	add         esi,4
	pmullw      mm0,mm2
	pmullw      mm1,mm2
	paddusw     mm0,mm1
	psrlw       mm0,8
	packuswb    mm0,mm7
	add         edx,4
	movd        [edi],mm0
	add         edi,4
	dec         ecx
	jnz         .xloop
	r_end
