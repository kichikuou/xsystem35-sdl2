;;; 
;;;void ablend16_dpd(BYTE *write, int src, BYTE *dst, int alpha, int w, int h, int pitchw, int pitchd)
;;;  write:	write address     (Pointer)
;;;  src:	source pixel      (Data)
;;;  dst:	destination pixel (Pointer)
;;;  alpha:	alpha pixel       (Data)
;;;  w:		width
;;;  h:		height
;;;  pitchw:	scan line of write
;;;  pitchd:	scan line of dst

ablend16_dpd:
	push	ebp
	push	ebx
	push	ecx
	push	edx
	push	esi
	push	edi
	
%assign _P 4*6
%define write [esp + _P + 4]
%define src   [esp + _P + 8]
%define dst   [esp + _P + 12]
%define	alpha [esp + _P + 16]
%define width [esp + _P + 20]
%define height [esp + _P + 24]
%define pitchw [esp + _P + 28]
%define pitchd [esp + _P + 32]
	
	mov	esi, src			;esi=src
	mov	edi, dst			;edi=dst
	mov	ebp, write
	mov	eax, alpha			;eax=alpha
	mov	ebx, width		;ecx=w
	mov	ecx, height		;ecx=h

	mov	[alpharate], si			; copy to data segment 
	mov	[alpharate + 2], si		; 
	mov	[alpharate + 4], si		; 
	mov	[alpharate + 6], si		; 
		
	align	16
.primeloop:
	movd	mm1, eax	; mm1=00 00 00 00 a a a a
	pxor	mm2, mm2	; mm2=0
	
	movq	mm4, [alpharate]; g1: mm4=src0 src0 src0 src0
	punpcklbw mm1, mm2	; mm1=00a3 00a2 00a1 00a0
	
	align	16
.loopqword:
	mov	edx, eax
	test	ebx, 0xfffffffc	; check if only 3 pixel left
	jnz	.lp1
	jmp	.checkback	; 3 or less pixel left
.lp1:
	cmp	edx, 0xffffffff	; test for alpha value of 0
	jne	.lp2
	jmp	.copyback	; if 1's copy the source pixel to destination
.lp2
	
	test	edx, 0xffffffff	; test for alpha value of 1
	jnz	.lp3
	jmp	.leavefront	; if so go to the next 4 pixel
.lp3
	
; green
;	i+a*src+(63-a)*dst
;	i=(i+32)+((i+32)>>6>>6
; red/blue
;	i+a*src+(31-a)*dst
;	i=(i+32)+((i+32)>>5>>5
	
	movq	mm5, [edi]	; g2: mm5=dst3 dst2 dst1 dst0
	psrlw	mm1, 2		; mm1=a? >>2 nule out lower 2bit
	
	movq	mm7, [maskshiftg]; g3: mm7=1 bit shifted gree mask
	psrlw	mm4, 1		; g3a: move src green down by 1 so that we dont overflow 
	
	movq	mm0, mm1	; mm0=00a3 00a2 00a1 00a0
	psrlw	mm5, 1		; g3a: move dst green down by 1 so that we dont overflow 
	
	psrlw	mm1, 1		; mm1=a? >>1 nuke out lowe 1 bit
	pand	mm4, mm7	; g5: mm4=sg3 sg2 sg1 sg0
	
	movq	mm2, [sixones]	; g4 mm2 = 63
	pand	mm5, mm7	; g7: mm5=dg3 dg2 dg1 dg0
	
	movq	mm3, [alpharate]; b1: mm3=src0 src0 src0 src0
	psubsb	mm2, mm0	; g6: mm2=63-a3 63-a2 63-a1 63-a0
	
	movq	mm7, [maskb16]	; b2: mm7=blue mask
	pmullw	mm4, mm0	; g8: mm4=sg * a?
	
	movq	mm0, [edi]	; b3: mm0=dst3 dst2 dst1 dst0
	pmullw	mm5, mm2	; g9: mm5=dg? * (1-a?)

	movq	mm2, mm7	; b4: mm2=finevones
	pand	mm3, mm7	; b4: mm3=sb3 sb2 sb1 sb0
	
	pmullw	mm3, mm1	; b6: mm3=sb? * a?
	pand	mm0, mm7	; b5: mm0=db3 sb2 db1 db0
	
	movq	mm7, [alpharate]; r1: mm7=src0 src0 src0 src0
	paddw	mm4, mm5	; g10: mm4=sg? * a? + dg? * (1-a?)
	
	pand	mm7, [maskr16]	; r2: mm7=sr3 sr2 sr1 sr0
	psubsb	mm2, mm1	; b5a mm2=31-a3 31-a2 31-a1 31-a0
	
	paddw	mm4, [fivetwelve]; g11: mm4=(mm4+512) green
	pmullw	mm0, mm2	; b7: mm0=db? * (1-a?)
	
	movq	mm5, mm4	; g12: mm5=mm4 green
	psrlw	mm7, 11		; r4: shift src red down to position 0
	
	psrlw	mm4, 6		; g13: mm4=mm4 >> 6
	
	paddw	mm4, mm5	; g14: mm4=mm4+mm5 green
	
	paddw	mm0, mm3	; b8: mm0=sb? * a? + db? * (1-a?)
	
	movq	mm5, [edi]	; r3: mm5 = dst3 dst2 dst1 dst0
	
	paddw	mm0, [sixteen]	; b9: mm0=(mm0+16) blue
	
	pand	mm5, [maskr16]	; r5: mm5=dr3 dr2 dr1 dr0
	psrlw	mm4, 5		; g15: mm4=0?g0 0?g0 0?g0 0?g0 green
	
	movq	mm3, mm0	; b10: mm3=mm0 blue
	psrlw	mm0, 5		; b11: mm0=mm0 >> 5 blue
	
	psrlw	mm5, 11		; r6: shift dst red down to position 0
	paddw	mm0, mm3	; b12: mm0=mm3+mm0 blue
	
	psrlw	mm0, 5		; b13: mm0=000b 000b 000b 000b blue
	pmullw	mm7, mm1	; mm7=sr? * a?
	
	pand	mm4, [maskg16]	; g16: mm4=00g0 00g0 00g0 00g0 green
	pmullw	mm5, mm2	; r7: mm5=dr? * (31-a?) 
	
	por	mm0, mm4	; mm0=00gb 00gb 00gb 00gb
	add	edi, 8		; move to next 4 pixel in dst
	add	ebp, 8		; move to next 4 pixel in write
	
	movd	mm1, eax	; mm1=00 00 00 00 a a a a
	paddw	mm5, mm7	; r8: mm5=sr? * a? + dr? * (31-a?)

	paddw	mm5, [sixteen]	; r9: mm5=(mm5+16) red
	pxor	mm2, mm2	; mm2=0
	
	movq	mm7, mm5	; r10: mm7=mm5
	psrlw	mm5, 5		; r11: mm5=mm5>>5 red
	
	movq	mm4, [alpharate]; g1: mm4=src0 src0 src0 src0
	paddw	mm5, mm7	; r12: mm5=mm7+mm5
	
	punpcklbw mm1, mm2	; mm1=00a3 00a2 00a1 00a0
	psrlw	mm5, 5		; r13: mm5=mm5>>5 red
	
	psllw	mm5, 11		; r14: mm5=mm5<10 red

	por	mm0, mm5	; mm0=0rgb 0rgb 0rgb 0rgb
	
	sub	ebx, 4		; polished off 4 pixels
	movq	[ebp-8], mm0	; write = 0rgb 0rgb 0rgb 0rgb

	jmp	.loopqword	; go back to start
		
.copyback:
	movq	[ebp], mm4	; copy souce to write

.leavefront:
	add	edi, 8		; advance destination 4 pixels
	add	ebp, 8		; advance write       4 pixels
	sub	ebx, 4		; decrease pixel count by 4
	jmp	.primeloop
	
.checkback:
	test	ebx, 0xff	; check if 0 pixel left
 	jnz	.lp6
	jmp	.nextline
.lp6:	
	movq	mm5, [edi]	; g2: mm5=dst3 dst2 dst1 dst0
	psrlw	mm1, 2		; mm1=a? >>2 nule out lower 2bit
	
	movq	mm7, [maskshiftg]; g3: mm7=1 bit shifted gree mask
	psrlw	mm4, 1		; g3a: move src green down by 1 so that we dont overflow 
	
	movq	mm0,mm1		; mm0=00a3 00a2 00a1 00a0
	psrlw	mm5, 1		; g3a: move dst green down by 1 so that we dont overflow
	
	psrlw	mm1, 1		; mm1=a? >>1 nuke out lowe 1 bit
	pand	mm4, mm7	; g5: mm4=sg3 sg2 sg1 sg0
	
	movq	mm2, [sixones]	; g4 mm2 = 63
	pand	mm5, mm7	; g7: mm5=dg3 dg2 dg1 dg0
	
	movd	mm3, [alpharate]; b1: mm3=src3 src2 src1 src0
	psubsb	mm2, mm0	; g6: mm2=63-a3 63-a2 63-a1 63-a0
	
	movq	mm7, [maskb16]	; b2: mm7=blue mask
	pmullw	mm4, mm0	; g8: mm4=sg * a?
	
	movq	mm0, [edi]	; b3: mm0=dst3 dst2 dst1 dst0
	pmullw	mm5, mm2	; g9: mm5=dg? * (1-a?)
	
	movq	mm2, mm7	; b4: mm2=finevones
	pand	mm3, mm7	; b4: mm3=sb3 sb2 sb1 sb0
	
	pmullw	mm3, mm1	; b6: mm3=sb? * a?
	pand	mm0, mm7	; b5: mm0=db3 sb2 db1 db0	
	
	movq	mm7, [alpharate]; r1: mm7=src3 src2 src1 src0
	paddw	mm4, mm5	; g10: mm4=sg? * a? + dg? * (1-a?)
	
	pand	mm7, [maskr16]	; r2: mm7=sr3 sr2 sr1 sr0
	psubsb	mm2, mm1	; b5a mm2=31-a3 31-a2 31-a1 31-a0
	
	paddw	mm4, [fivetwelve]; g11: mm4=(mm4+512) green
	pmullw	mm0, mm2	; b7: mm0=db? * (1-a?)
	
	movq	mm5, mm4	; g12: mm5=mm4 green
	psrlw	mm7, 11		; r4: shift src red down to position 0
	
	psrlw	mm4, 6		; g13: mm4=mm4 >> 6
	
	paddw	mm4, mm5	; g14: mm4=mm4+mm5 green
	
	paddw	mm0, mm3	; b8: mm0=sb? * a? + db? * (1-a?)
	
	movq	mm5, [edi]	; r3: mm5 = dst3 dst2 dst1 dst0
	
	paddw	mm0, [sixteen]	; b9: mm0=(mm0+16) blue
	
	pand	mm5, [maskr16]	; r5: mm5=dr3 dr2 dr1 dr0
	psrlw	mm4, 5		; g15: mm4=0?g0 0?g0 0?g0 0?g0 green
	
	movq	mm3, mm0	; b10: mm3=mm0 blue
	psrlw	mm0, 5		; b11: mm0=mm0 >> 5 blue
	
	psrlw	mm5, 11		; r6: shift dst red down to position 0
	paddw	mm0, mm3	; b12: mm0=mm3+mm0 blue
	
	psrlw	mm0, 5		; b13: mm0=000b 000b 000b 000b blue
	pmullw	mm7, mm1	; mm7=sr? * a?
	
	pand	mm4, [maskg16]	; g16: mm4=00g0 00g0 00g0 00g0 green
	pmullw	mm5, mm2	; r7: mm5=dr? * (31-a?) 
	
	por	mm0, mm4	; mm0=00gb 00gb 00gb 00gb
	
	paddw	mm5, mm7	; r8: mm5=sr? * a? + dr? * (31-a?)
	
	paddw	mm5, [sixteen]	; r9: mm5=(mm5+16) red
	
	movq	mm7, mm5	; r10: mm7=mm5
	psrlw	mm5, 5		; r11: mm5=mm5>>5 red
	
	paddw	mm5, mm7	; r12: mm5=mm7+mm5
	
	psrlw	mm5, 5		; r13: mm5=mm5>>5 red
	
	psllw	mm5, 11		; r14: mm5=mm5<10 red
	
	por	mm0, mm5	; mm0=0rgb 0rgb 0rgb 0rgb
	test	ebx, 2		; check if there are 2 pixel
	
	jz	.oneendpixel	; goto one pixel if thats it
	movd	[ebp], mm0	; dst = 0000 000 0rgb 0rgb
	psrlq	mm0, 32		; mm0 >> 32
	
	add	edi, 4		; edi=edi+4
	add	ebp, 4		; ebp=ebp+4
	sub	ebx, 2		; save 2 pixels
	jz	.nextline	; all done goto next line
	
.oneendpixel:
	movd	edx, mm0	; edx=0rgb
	
	mov	[ebp], dx	; dst=0rgb
	
.nextline:
	dec	ecx		; nuke one line
	jz	.done		; all done
	
	mov	edi, dst	;edi=dst
	mov	ebp, write	;ebp=write

	add	edi, pitchd	;pitch dst
	add	ebp, pitchw	;pitch write
	
	mov	ebx, width
	mov	dst, edi
	mov	write, ebp
	
	jmp	.primeloop

.done:
	emms
	pop	edi
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	pop	ebp
	ret
