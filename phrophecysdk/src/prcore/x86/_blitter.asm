; ============================================================
; module: mmx blitter
;
; Copyright (C) 1999-2003 Twilight 3D Finland Oy Ltd.
; http://www.twilight3d.com
; ============================================================

		segment .data

fracmask64 dd   00ff00ffh, 00ff00ffh
vfrac64 dd      0,0
ufracstep64 dd  0,0

vfrac   dd      0
ustep   dd      0

		segment .text

		global  _inner_stretch_rgb888_bilinear_x86_mmx
		global  inner_stretch_rgb888_bilinear_x86_mmx_
		global  inner_stretch_rgb888_bilinear_x86_mmx

_inner_stretch_rgb888_bilinear_x86_mmx:
inner_stretch_rgb888_bilinear_x86_mmx_:
inner_stretch_rgb888_bilinear_x86_mmx:

		pushad
		mov     esi,[esp+8*4+8]
		mov     edi,[esi]
		mov     eax,[esi+1*4]
		mov     ebx,[esi+2*4]
		mov     edx,[esi+3*4]
		movd    mm0,[esi+6*4]
		punpcklwd mm0,mm0
		punpckldq mm0,mm0
		psrlw   mm0,8
		movq    [vfrac64],mm0
		movd    mm0,[esi+5*4]
		punpcklwd mm0,mm0
		punpckldq mm0,mm0
		movq    [ufracstep64],mm0
		movd    mm4,[esi+4*4]
		dec     edx
		punpcklwd mm4,mm4
		mov     ebp,edx
		add     edx,edx
		punpckldq mm4,mm4
		add     edx,ebp

		mov     ebp,[esi+5*4]
		mov     esi,[esi+4*4]

		add     edi,edx
		push    esi
		neg     edx
		shr     esi,16
		movq    mm6,[fracmask64]
		mov     ecx,esi
		add     esi,esi
		add     esi,ecx
		pxor    mm7,mm7
		movd    mm1,[eax+esi+2]
		movd    mm2,[ebx+esi]
		psrld   mm1,8
		movd    mm3,[ebx+esi+2]
		psrld   mm3,8

		test    edx,edx
		jz      near .singlepixel
.pixel:

		punpcklbw mm1,mm7
		movq    mm5,mm4
		movd    mm0,[eax+esi]
		punpcklbw mm3,mm7
		psrlw   mm5,8
		pop     esi

		pmullw  mm1,mm5
		add     esi,ebp
		punpcklbw mm0,mm7
		push    esi
		pmullw  mm3,mm5
		shr     esi,16
		pxor    mm5,mm6
		mov     ecx,esi
		punpcklbw mm2,mm7
		add     esi,esi
		pmullw  mm0,mm5
		add     esi,ecx
		pmullw  mm2,mm5
		paddw   mm2,mm3
		movd    mm3,[ebx+esi+2]
		paddw   mm0,mm1
		psrld   mm3,8
		movq    mm5,[vfrac64]
		psrlw   mm2,8
		movd    mm1,[eax+esi+2]
		psrlw   mm0,8

		pmullw  mm2,mm5
		pxor    mm5,mm6
		psrld   mm1,8
		pmullw  mm0,mm5
		paddw   mm0,mm2
		movd    mm2,[ebx+esi]
		psrlw   mm0,8

		packuswb mm0,mm7
		paddw   mm4,[ufracstep64]
		movd    [edi+edx],mm0

		add     edx,3
		js      near .pixel
.singlepixel:

		punpcklbw mm1,mm7
		movq    mm5,mm4
		punpcklbw mm3,mm7
		movd    mm0,[eax+esi]
		psrlw   mm5,8
		pop     esi

		pmullw  mm1,mm5
		punpcklbw mm0,mm7
		pmullw  mm3,mm5
		pxor    mm5,mm6
		punpcklbw mm2,mm7
		pmullw  mm0,mm5
		pmullw  mm2,mm5
		paddw   mm2,mm3
		paddw   mm0,mm1
		movq    mm5,[vfrac64]
		psrlw   mm2,8
		psrlw   mm0,8

		pmullw  mm2,mm5
		pxor    mm5,mm6
		pmullw  mm0,mm5
		paddw   mm0,mm2
		psrlw   mm0,8

		packuswb mm0,mm7
		movd    eax,mm0
		mov     [edi],al
		mov     [edi+1],ah
		shr     eax,16
		mov     [edi+2],al

		emms
		popad
		ret

		global  _inner_stretch_argb8888_bilinear_x86_mmx
		global  inner_stretch_argb8888_bilinear_x86_mmx_
		global  inner_stretch_argb8888_bilinear_x86_mmx

_inner_stretch_argb8888_bilinear_x86_mmx:
inner_stretch_argb8888_bilinear_x86_mmx_:
inner_stretch_argb8888_bilinear_x86_mmx:

		pushad
		mov     esi,[esp+8*4+8]
		mov     ecx,[esi]
		mov     eax,[esi+1*4]
		mov     ebx,[esi+2*4]
		mov     edx,[esi+3*4]
		movd    mm0,[esi+6*4]
		punpcklwd mm0,mm0
		punpckldq mm0,mm0
		psrlw   mm0,8
		movq    [vfrac64],mm0
		movd    mm0,[esi+5*4]
		punpcklwd mm0,mm0
		punpckldq mm0,mm0
		movq    [ufracstep64],mm0
		movd    mm4,[esi+4*4]
		punpcklwd mm4,mm4
		punpckldq mm4,mm4

		mov     ebp,[esi+5*4]
		mov     esi,[esi+4*4]

		lea     ecx,[ecx+edx*4]
		mov     edi,esi
		neg     edx
		and     edi,0ffff0000h
		movq    mm6,[fracmask64]
		shr     edi,14
		pxor    mm7,mm7
		movd    mm1,[eax+edi+4]
		movd    mm2,[ebx+edi]
		movd    mm3,[ebx+edi+4]

		cmp     edx,-1
		je      .singlepixel
.pixel:

		punpcklbw mm1,mm7
		movq    mm5,mm4
		punpcklbw mm3,mm7
		movd    mm0,[eax+edi]
		psrlw   mm5,8

		pmullw  mm1,mm5
		punpcklbw mm0,mm7
		pmullw  mm3,mm5
		add     esi,ebp
		pxor    mm5,mm6
		mov     edi,esi
		punpcklbw mm2,mm7
		and     edi,0ffff0000h
		pmullw  mm0,mm5
		shr     edi,14
		pmullw  mm2,mm5
		paddw   mm2,mm3
		movd    mm3,[ebx+edi+4]
		paddw   mm0,mm1
		movq    mm5,[vfrac64]
		psrlw   mm2,8
		movd    mm1,[eax+edi+4]
		psrlw   mm0,8

		pmullw  mm2,mm5
		pxor    mm5,mm6
		pmullw  mm0,mm5
		paddw   mm0,mm2
		movd    mm2,[ebx+edi]
		psrlw   mm0,8

		packuswb mm0,mm7
		paddw   mm4,[ufracstep64]
		movd    [ecx+edx*4],mm0

		inc     edx
		js      .pixel
.singlepixel:

		punpcklbw mm1,mm7
		movq    mm5,mm4
		punpcklbw mm3,mm7
		movd    mm0,[eax+edi]
		psrlw   mm5,8

		pmullw  mm1,mm5
		punpcklbw mm0,mm7
		pmullw  mm3,mm5
		pxor    mm5,mm6
		punpcklbw mm2,mm7
		pmullw  mm0,mm5
		pmullw  mm2,mm5
		paddw   mm2,mm3
		paddw   mm0,mm1
		movq    mm5,[vfrac64]
		psrlw   mm2,8
		psrlw   mm0,8

		pmullw  mm2,mm5
		pxor    mm5,mm6
		pmullw  mm0,mm5
		paddw   mm0,mm2
		psrlw   mm0,8

		packuswb mm0,mm7
		movd    [ecx-4],mm0

		emms
		popad
		ret

		global  _inner_stretch_smc_start
		global  inner_stretch_smc_start_
		global  inner_stretch_smc_start

_inner_stretch_smc_start:
inner_stretch_smc_start_:
inner_stretch_smc_start:

		global  _inner_stretch_rgb565_bilinear_x86
		global  inner_stretch_rgb565_bilinear_x86_
		global  inner_stretch_rgb565_bilinear_x86

_inner_stretch_rgb565_bilinear_x86:
inner_stretch_rgb565_bilinear_x86_:
inner_stretch_rgb565_bilinear_x86:

		pushad
		mov     edx,[esp+8*4+8]
		mov     eax,[edx]
		mov     ebx,[edx+1*4]
		mov     ecx,[edx+2*4]
		mov     edi,[edx+3*4]
		add     eax,edi
		add     eax,edi
		mov     [.smc_dest-4],eax
		mov     [.smc_src1-4],ebx
		mov     [.smc_src2-4],ecx
		mov     ebp,[edx+4*4]
		mov     eax,[edx+5*4]
		mov     ebx,[edx+6*4]
		neg     edi
		mov     [ustep],eax
		shr     ebx,11
		mov     [vfrac],ebx
.pixel:
		mov     esi,ebp
		shr     esi,16
		mov     eax,[01234567h+esi*2]
.smc_src1:
		mov     ebx,[01234567h+esi*2]
.smc_src2:
		mov     ecx,eax
		mov     edx,ebx
		and     eax,0f81f07e0h
		and     ebx,0f81f07e0h
		xor     ecx,eax
		shr     eax,5
		xor     edx,ebx
		shr     ebx,5

		mov     esi,[vfrac]
		imul    ebx,esi
		imul    edx,esi
		xor     esi,1fh
		imul    eax,esi
		imul    ecx,esi
		add     eax,ebx
		add     ecx,edx
		shr     ecx,5
		and     eax,0f81f07e0h
		and     ecx,07e0f81fh

		rol     ecx,16
		mov     ebx,eax
		mov     esi,ebp
		xor     ebx,ecx
		shr     esi,11
		and     ebx,0ffffh
		and     esi,1fh
		xor     eax,ebx
		shr     eax,5
		xor     ecx,ebx
		shr     ecx,5
		imul    eax,esi
		xor     esi,1fh
		imul    ecx,esi
		add     eax,ecx
		and     eax,0f81f07e0h
		mov     ebx,eax
		rol     ebx,16
		or      eax,ebx

		mov     [01234567h+edi*2],ax
.smc_dest:
		add     ebp,[ustep]
		inc     edi
		jnz     near .pixel

		popad
		ret

		global  _inner_stretch_argb1555_bilinear_x86
		global  inner_stretch_argb1555_bilinear_x86_
		global  inner_stretch_argb1555_bilinear_x86

_inner_stretch_argb1555_bilinear_x86:
inner_stretch_argb1555_bilinear_x86_:
inner_stretch_argb1555_bilinear_x86:

		pushad
		mov     edx,[esp+8*4+8]
		mov     eax,[edx]
		mov     ebx,[edx+1*4]
		mov     ecx,[edx+2*4]
		mov     edi,[edx+3*4]
		add     eax,edi
		add     eax,edi
		mov     [.smc_dest-4],eax
		mov     [.smc_src1-4],ebx
		mov     [.smc_src2-4],ecx
		mov     ebp,[edx+4*4]
		mov     eax,[edx+5*4]
		mov     ebx,[edx+6*4]
		neg     edi
		mov     [ustep],eax
		shr     ebx,11
		mov     [vfrac],ebx
.pixel:
		mov     esi,ebp
		shr     esi,16
		mov     eax,[01234567h+esi*2]
.smc_src1:
		mov     ebx,[01234567h+esi*2]
.smc_src2:
		push    eax

		mov     ecx,eax
		mov     edx,ebx
		and     eax,7c1f03e0h
		and     ebx,7c1f03e0h
		and     ecx,03e07c1fh
		shr     eax,5
		and     edx,03e07c1fh
		shr     ebx,5

		mov     esi,[vfrac]
		imul    ebx,esi
		imul    edx,esi
		xor     esi,1fh
		imul    eax,esi
		imul    ecx,esi
		add     eax,ebx
		add     ecx,edx
		shr     ecx,5
		and     eax,7c1f03e0h
		and     ecx,03e07c1fh

		rol     ecx,16
		mov     ebx,eax
		mov     esi,ebp
		xor     ebx,ecx
		shr     esi,11
		and     ebx,0ffffh
		and     esi,1fh
		xor     eax,ebx
		shr     eax,5
		xor     ecx,ebx
		shr     ecx,5
		imul    eax,esi
		xor     esi,1fh
		imul    ecx,esi
		add     eax,ecx
		and     eax,7c1f03e0h
		mov     ebx,eax
		pop     edx
		rol     ebx,16
		and     edx,8000h
		or      eax,ebx
		or      eax,edx

		mov     [01234567h+edi*2],ax
.smc_dest:
		add     ebp,[ustep]
		inc     edi
		jnz     near .pixel

		popad
		ret

		global  _inner_stretch_argb4444_bilinear_x86
		global  inner_stretch_argb4444_bilinear_x86_
		global  inner_stretch_argb4444_bilinear_x86

_inner_stretch_argb4444_bilinear_x86:
inner_stretch_argb4444_bilinear_x86_:
inner_stretch_argb4444_bilinear_x86:

		pushad
		mov     edx,[esp+8*4+8]
		mov     eax,[edx]
		mov     ebx,[edx+1*4]
		mov     ecx,[edx+2*4]
		mov     edi,[edx+3*4]
		add     eax,edi
		add     eax,edi
		mov     [.smc_dest-4],eax
		mov     [.smc_src1-4],ebx
		mov     [.smc_src2-4],ecx
		mov     ebp,[edx+4*4]
		mov     eax,[edx+5*4]
		mov     ebx,[edx+6*4]
		neg     edi
		mov     [ustep],eax
		shr     ebx,12
		mov     [vfrac],ebx
.pixel:
		mov     esi,ebp
		shr     esi,16
		mov     eax,[01234567h+esi*2]
.smc_src1:
		mov     ebx,[01234567h+esi*2]
.smc_src2:
		mov     ecx,eax
		mov     edx,ebx
		and     eax,0f0f0f0f0h
		and     ebx,0f0f0f0f0h
		xor     ecx,eax
		shr     eax,4
		xor     edx,ebx
		shr     ebx,4

		mov     esi,[vfrac]
		imul    ebx,esi
		imul    edx,esi
		xor     esi,0fh
		imul    eax,esi
		imul    ecx,esi
		add     eax,ebx
		add     ecx,edx
		and     eax,0f0f0f0f0h
		and     ecx,0f0f0f0f0h

		ror     ecx,16
		mov     ebx,eax
		mov     esi,ebp
		xor     ebx,ecx
		shr     esi,12
		and     ebx,0ffffh
		and     esi,0fh
		xor     eax,ebx
		shr     eax,4
		xor     ecx,ebx
		ror     ecx,16
		shr     ecx,4
		imul    eax,esi
		xor     esi,0fh
		imul    ecx,esi
		add     eax,ecx
		mov     ebx,eax
		and     eax,0f0f0h
		shr     eax,4
		and     ebx,0f0f00000h
		rol     ebx,16
		or      eax,ebx

		mov     [01234567h+edi*2],ax
.smc_dest:
		add     ebp,[ustep]
		inc     edi
		jnz     near .pixel

		popad
		ret

		global  _inner_stretch_smc_size
		global  inner_stretch_smc_size_
		global  inner_stretch_smc_size

_inner_stretch_smc_size:
inner_stretch_smc_size_:
inner_stretch_smc_size:
		dd      $-inner_stretch_smc_start
