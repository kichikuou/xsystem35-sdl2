/*
 *	for new GOGO-no-coda (1999/09)
 *	Copyright (C) 1999 shigeo
 */

#ifndef _CHKCPU_H
#define _CHKCPU_H

#define tFPU	(1<<0)
#define tMMX	(1<<1)
#define t3DN	(1<<2)
#define tSSE	(1<<3)
#define tCMOV	(1<<4)
#define tE3DN	(1<<5)	/* Athlon用(Externd 3D Now!) */
#define tEMMX   (1<<6)  /* EMMX=E3DNow!_INT=SSE_INT  */
#define tINTEL	(1<<8)
#define tAMD	(1<<9)
#define tCYRIX	(1<<10)
#define tIDT	(1<<11)
#define tMULTI	(1<<12)	/* for Multi-threaded encoder. */
		/* Never set on UP or in the binary linked w/o multithread lib. */
#define tUNKNOWN	(1<<15)	/* ベンダー不明 */
#define tSPC1 (1<<16)	/* 特別なスイッチ */
#define tSPC2 (1<<17)	/* 用途は決まってない */

#define tFAMILY4	(1<<20)	/* 486 この時ベンダー判定は当てにならない */
#define tFAMILY5	(1<<21)	/* 586 (P5, P5-MMX, K6, K6-2, K6-III) */
#define tFAMILY6	(1<<22)	/* 686以降 P-Pro, P-II, P-III, Athlon */

/*
 *	搭載しているユニットに従って上の値の論理和を返す
 *	chkcpu.asmとの整合性注意
 */

int haveUNIT(void);

/*
 *	useUNITに従って関数の使用を変更する
 */

void setupUNIT(int useUNIT);

/*
 *	SSEを四捨五入モードにする
 */

void setPIII_round(void);

 /*
 *	使用関数の表示(デバッグ用)
 */

void SETUP_DSP(char *mes);

#ifdef WIN32
void maskFPU_exception( void );
#endif

#if 0
#define SETUP_DEBUG

#ifdef SETUP_DEBUG
#define SETUP_DSP(x) fprintf(stderr,x)
#else
#define SETUP_DSP(x)
#endif
#endif

#endif
