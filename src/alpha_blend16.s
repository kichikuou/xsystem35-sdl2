g;;; from Intel application notes
	
%include "nasm.h"
	global	ablend16_dpd
	global	ablend16_ppd
	global	ablend16_ppp

	segment_data
	
	align	16

maskb16		dd	0x001f001f, 0x001f001f
maskg16 	dd	0x07e007e0, 0x07e007e0
maskr16 	dd	0xf800f800, 0xf800f800
maskshiftg 	dd	0x03f003f0, 0x03f003f0
sixteen 	dd	0x00100010, 0x00100010
fivetwelve	dd	0x02000200, 0x02000200
sixones 	dd	0x003f003f, 0x003f003f
alpharate	dd	0, 0
alphamask	dd	0x00ff00ff, 0x00ff00ff
	
	segment_code

%include "ablend16_dpd.s"
%include "ablend16_ppd.s"
%include "ablend16_ppp.s"		
