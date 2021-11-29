#include "config.h"

#include <stdio.h>
#include <zlib.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "filecheck.h"
#include "modules.h"
#include "nact.h"
#include "ngraph.h"

#define NUM_MAPS 16
#define NUM_LAYERS 3
#define MAPDATASIZE (128 * 128 * sizeof(WORD) * NUM_LAYERS)

typedef struct {
	DWORD size;
	DWORD map;
} RecordHeader;

static void *mapdata[NUM_MAPS];
static int setChipParam_index;
static MyRectangle chip_params[NUM_LAYERS];
static int window_width;
static int window_height;
static int map_width;

static void MakeMapSetParam() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();

	if (p1 != NUM_LAYERS) {
		WARNING("Unexpected number of layers: %d\n", p1);
		return;
	}
	setChipParam_index = 0;
	window_width = p2;
	window_height = p3;
	map_width = p4;

	DEBUG_COMMAND("oujimisc.MakeMapSetParam %d,%d,%d,%d:\n", p1,p2,p3,p4);
}

static void MakeMapSetChipParam() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();

	if (setChipParam_index >= NUM_LAYERS) {
		WARNING("unexpected MakeMapSeChipParam call\n");
		return;
	}
	MyRectangle *r = &chip_params[setChipParam_index++];
	r->x = p1;
	r->y = p2;
	r->w = p3;
	r->h = p4;

	DEBUG_COMMAND("oujimisc.MakeMapSeChipParam %d,%d,%d,%d,%d:\n", p1,p2,p3,p4,p5);
}

static void MakeMapDraw() {
	int p1 = getCaliValue(); /* ISys3xDIB */
	int dstX = getCaliValue();
	int dstY = getCaliValue();
	int posX = getCaliValue();
	int posY = getCaliValue();
	int *a1 = getCaliVariable();
	int *a2 = getCaliVariable();
	int *a3 = getCaliVariable();

	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			int index = (y + posY) * map_width + (x + posX);
			int c1 = a1[index];
			int c2 = a2[index];
			int c3 = a3[index];
			MyRectangle *r1 = &chip_params[0];
			MyRectangle *r2 = &chip_params[1];
			MyRectangle *r3 = &chip_params[2];
			surface_t *dib = ags_getDIB();
			if (c3) {
				ags_copyArea(r3->x + c3 * r3->w, r3->y, r3->w, r3->h, dstX + x * r3->w, dstY + y * r3->h);
			} else {
				ags_copyArea(r1->x + c1 * r1->w, r1->y, r1->w, r1->h, dstX + x * r1->w, dstY + y * r1->h);
				gr_blend_alpha_map(dib, dstX + x * r2->w, dstY + y * r2->h, dib, r2->x + c2 * r2->w, r2->y, r2->w, r2->h);
			}
		}
	}

	DEBUG_COMMAND("oujimisc.MakeMapDraw %d,%d,%d,%d,%d,%p,%p,%p:\n", p1,dstX,dstY,posX,posY,a1,a2,a3);
}

static void MakeMapInit() {
	int p1 = getCaliValue(); /* ISys3x */
	
	DEBUG_COMMAND_YET("oujimisc.MakeMapInit %d:\n", p1);
}	

static void DrawNumber() {
	int p1 = getCaliValue(); /* ISys3xDIB */
	int srcX = getCaliValue();
	int srcY = getCaliValue();
	int width = getCaliValue();
	int height = getCaliValue();
	int dstX = getCaliValue();
	int dstY = getCaliValue();
	int num = getCaliValue();

	char buf[6];
	sprintf(buf, "%5d", num);

	surface_t *dib = ags_getDIB();
	for (char *c = buf; *c; c++) {
		if (*c != ' ') {
			int sx = srcX + (*c - '0') * width;
			gr_blend_alpha_map(dib, dstX, dstY, dib, sx, srcY, width, height);
		}
		dstX += width;
	}

	DEBUG_COMMAND("oujimisc.DrawNumber %d,%d,%d,%d,%d,%d,%d,%d:\n", p1,srcX,srcY,width,height,dstX,dstY,num);
}

static void TempMapCreateShadow() {
	for (int i = 0; i < NUM_MAPS; i++) {
		dridata *dfile = ald_getdata(DRIFILE_DATA, i);
		if (!dfile)
			continue;
		if (dfile->size != MAPDATASIZE)
			WARNING("unexpected map data size %d", dfile->size);
		free(mapdata[i]);
		mapdata[i] = malloc(dfile->size);
		memcpy(mapdata[i], dfile->data, dfile->size);
		ald_freedata(dfile);
	}

	DEBUG_COMMAND("oujimisc.TempMapCreateShadow:\n");
}

static void TempMapInit() {
	int p1 = getCaliValue(); /* ISys3x */
	int p2 = getCaliValue();
	
	DEBUG_COMMAND_YET("oujimisc.TempMapInit %d,%d:\n", p1,p2);
}

static void TempMapLoadToShadow() {
	int map = getCaliValue();
	int *a1 = getCaliVariable();
	int a1page = preVarPage;
	int *a2 = getCaliVariable();
	int a2page = preVarPage;
	int *a3 = getCaliVariable();
	int a3page = preVarPage;
	int size = getCaliValue();

	if (!mapdata[map]) {
		WARNING("No map %d\n", map);
		return;
	}
	if (a1page == 0 || a2page == 0 || a3page == 0) {
		WARNING("non-array destination variable\n");
		return;
	}
	WORD *p = mapdata[map];
	for (int i = 0; i < size; i++)
		*a1++ = SDL_SwapLE16(*p++);
	for (int i = 0; i < size; i++)
		*a2++ = SDL_SwapLE16(*p++);
	for (int i = 0; i < size; i++)
		*a3++ = SDL_SwapLE16(*p++);

	DEBUG_COMMAND("oujimisc.TempMapLoadToShadow %d,%p,%p,%p,%d:\n", map,a1,a2,a3,size);
}

static void TempMapSaveToShadow() {
	int map = getCaliValue();
	int *a1 = getCaliVariable();
	int a1page = preVarPage;
	int *a2 = getCaliVariable();
	int a2page = preVarPage;
	int *a3 = getCaliVariable();
	int a3page = preVarPage;
	int size = getCaliValue();

	if (!mapdata[map]) {
		WARNING("No map %d\n", map);
		return;
	}
	if (a1page == 0 || a2page == 0 || a3page == 0) {
		WARNING("non-array source variable\n");
		return;
	}
	WORD *p = mapdata[map];
	for (int i = 0; i < size; i++)
		*p++ = SDL_SwapLE16(*a1++);
	for (int i = 0; i < size; i++)
		*p++ = SDL_SwapLE16(*a2++);
	for (int i = 0; i < size; i++)
		*p++ = SDL_SwapLE16(*a3++);

	DEBUG_COMMAND("oujimisc.TempMapSaveToShadow %d,%p,%p,%p,%d:\n", map,a1,a2,a3,size);
}

static void TempMapFileSave() {
	int p1 = getCaliValue();

	char fname[32];
	sprintf(fname, "王子_M%d.asd", p1);
	FILE *fp = fc_open(fname, 'w');
	if (!fp) {
		WARNING("Cannot open %s\n", fname);
		return;
	}

	for (int i = 0; i < NUM_MAPS; i++) {
		if (!mapdata[i])
			continue;
		unsigned long size = compressBound(MAPDATASIZE);
		void *compressed = malloc(size);
		int r = compress2(compressed, &size, mapdata[i], MAPDATASIZE, Z_BEST_COMPRESSION);
		if (r != Z_OK) {
			WARNING("compress2() failed: %d\n", r);
			free(compressed);
			continue;
		}
		RecordHeader hdr = { size, i };
		fwrite(&hdr, sizeof(hdr), 1, fp);
		fwrite(compressed, size, 1, fp);
		free(compressed);
	}
	fclose(fp);

	DEBUG_COMMAND("oujimisc.TempMapFileSave %d:\n", p1);
}

static void TempMapFileLoad() {
	int p1 = getCaliValue();

	char fname[32];
	sprintf(fname, "王子_M%d.asd", p1);
	FILE *fp = fc_open(fname, 'r');
	if (!fp) {
		WARNING("Cannot open %s\n", fname);
		return;
	}

	for (int i = 0; i < NUM_MAPS; i++) {
		RecordHeader hdr;
		if (fread(&hdr, sizeof(hdr), 1, fp) != 1) {
			WARNING("fread error\n");
			break;
		}
		if (hdr.map != i) {
			WARNING("unexpected map no. %d != %d\n", hdr.map, i);
			break;
		}
		if (!mapdata[i]) {
			WARNING("map %d is not allocated\n", i);
			break;
		}
		void *compressed = malloc(hdr.size);
		if (fread(compressed, hdr.size, 1, fp) != 1) {
			WARNING("fread error\n");
			free(compressed);
			break;
		}
		unsigned long raw_size = MAPDATASIZE;
		int r = uncompress(mapdata[i], &raw_size, compressed, hdr.size);
		if (r != Z_OK)
			WARNING("uncompress() failed: %d\n", r);
		free(compressed);
	}
	fclose(fp);

	DEBUG_COMMAND("oujimisc.TempMapFileLoad %d:\n", p1);
}

static void CalcMoveDiffer() {
	int *dx = getCaliVariable();
	int *dy = getCaliVariable();
	int moveL = getCaliValue();
	int moveU = getCaliValue();
	int moveR = getCaliValue();
	int moveD = getCaliValue();
	int *pt = getCaliVariable();
	int duration = getCaliValue();

	int t = min(*pt, duration);
	if (moveL) {
		*dx = moveL - moveL * t / duration;
		*dy = 0;
	} else if (moveU) {
		*dx = 0;
		*dy = moveU - moveU * t / duration;
	} else if (moveR) {
		*dx = moveR * t / duration;
		*dy = 0;
	} else if (moveD) {
		*dx = 0;
		*dy = moveD * t / duration;
	}

	*pt = t >= duration ? 1 : 0;

	DEBUG_COMMAND("oujimisc.CalcMoveDiffer %p,%p,%d,%d,%d,%d,%p,%d:\n", dx,dy,moveL,moveU,moveR,moveD,pt,duration);
}

static const ModuleFunc functions[] = {
	{"CalcMoveDiffer", CalcMoveDiffer},
	{"DrawNumber", DrawNumber},
	{"MakeMapDraw", MakeMapDraw},
	{"MakeMapInit", MakeMapInit},
	{"MakeMapSetChipParam", MakeMapSetChipParam},
	{"MakeMapSetParam", MakeMapSetParam},
	{"TempMapCreateShadow", TempMapCreateShadow},
	{"TempMapFileLoad", TempMapFileLoad},
	{"TempMapFileSave", TempMapFileSave},
	{"TempMapInit", TempMapInit},
	{"TempMapLoadToShadow", TempMapLoadToShadow},
	{"TempMapSaveToShadow", TempMapSaveToShadow},
};

const Module module_oujimisc = {"oujimisc", functions, sizeof(functions) / sizeof(ModuleFunc)};
