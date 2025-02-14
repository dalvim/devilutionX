#include "diablo.h"

DEVILUTION_BEGIN_NAMESPACE

WORD level_frame_types[MAXTILES];
int themeCount;
char nTransTable[2049];
//int dword_52D204;
int dMonster[MAXDUNX][MAXDUNY];
BYTE dungeon[40][40];
char dObject[MAXDUNX][MAXDUNY];
BYTE *pSpeedCels;
int nlevel_frames; // weak
char pdungeon[40][40];
char dDead[MAXDUNX][MAXDUNY];
MICROS dpiece_defs_map_1[MAXDUNX * MAXDUNY];
char dPreLight[MAXDUNX][MAXDUNY];
char TransVal; // weak
int MicroTileLen;
char dflags[40][40];
int dPiece[MAXDUNX][MAXDUNY];
char dLight[MAXDUNX][MAXDUNY];
int setloadflag_2; // weak
int tile_defs[MAXTILES];
BYTE *pMegaTiles;
BYTE *pLevelPieces;
int gnDifficulty; // idb
char block_lvid[2049];
//char byte_5B78EB;
char dTransVal[MAXDUNX][MAXDUNY];
BOOLEAN nTrapTable[2049];
BYTE leveltype;
unsigned char currlevel; // idb
char TransList[256];
BOOLEAN nSolidTable[2049];
int level_frame_count[MAXTILES];
ScrollStruct ScrollInfo;
BYTE *pDungeonCels;
int SpeedFrameTbl[128][16];
THEME_LOC themeLoc[MAXTHEMES];
char dPlayer[MAXDUNX][MAXDUNY];
int dword_5C2FF8;   // weak
int dword_5C2FFC;   // weak
int scr_pix_width;  // weak
int scr_pix_height; // weak
char dArch[MAXDUNX][MAXDUNY];
char nBlockTable[2049];
BYTE *pSpecialCels;
char dFlags[MAXDUNX][MAXDUNY];
char dItem[MAXDUNX][MAXDUNY];
BYTE setlvlnum;
int level_frame_sizes[MAXTILES];
char nMissileTable[2049];
char *pSetPiece_2;
char setlvltype; // weak
BOOLEAN setlevel;
int LvlViewY;    // weak
int LvlViewX;    // weak
int dmaxx;       // weak
int dmaxy;       // weak
int setpc_h;     // weak
int setpc_w;     // weak
int setpc_x;     // idb
int ViewX;       // idb
int ViewY;       // idb
int setpc_y;     // idb
char dMissile[MAXDUNX][MAXDUNY];
int dminx; // weak
int dminy; // weak
MICROS dpiece_defs_map_2[MAXDUNX][MAXDUNY];

void FillSolidBlockTbls()
{
	unsigned char bv;
	unsigned int dwTiles;
	unsigned char *pSBFile, *pTmp;
	int i;

	memset(nBlockTable, 0, sizeof(nBlockTable));
	memset(nSolidTable, 0, sizeof(nSolidTable));
	memset(nTransTable, 0, sizeof(nTransTable));
	memset(nMissileTable, 0, sizeof(nMissileTable));
	memset(nTrapTable, 0, sizeof(nTrapTable));

	switch (leveltype) {
	case DTYPE_TOWN:
		pSBFile = LoadFileInMem("Levels\\TownData\\Town.SOL", (int *)&dwTiles);
		break;
	case DTYPE_CATHEDRAL:
		pSBFile = LoadFileInMem("Levels\\L1Data\\L1.SOL", (int *)&dwTiles);
		break;
	case DTYPE_CATACOMBS:
		pSBFile = LoadFileInMem("Levels\\L2Data\\L2.SOL", (int *)&dwTiles);
		break;
	case DTYPE_CAVES:
		pSBFile = LoadFileInMem("Levels\\L3Data\\L3.SOL", (int *)&dwTiles);
		break;
	case DTYPE_HELL:
		pSBFile = LoadFileInMem("Levels\\L4Data\\L4.SOL", (int *)&dwTiles);
		break;
	default:
		app_fatal("FillSolidBlockTbls");
	}

	pTmp = pSBFile;

	for (i = 1; i <= dwTiles; i++) {
		bv = *pTmp++;
		if (bv & 1)
			nSolidTable[i] = 1;
		if (bv & 2)
			nBlockTable[i] = 1;
		if (bv & 4)
			nMissileTable[i] = 1;
		if (bv & 8)
			nTransTable[i] = 1;
		if (bv & 0x80)
			nTrapTable[i] = 1;
		block_lvid[i] = (bv & 0x70) >> 4; /* beta: (bv >> 4) & 7 */
	}

	mem_free_dbg(pSBFile);
}

void MakeSpeedCels()
{
	int i, j, x, y;
	int total_frames, blocks, total_size, frameidx, lfs_adder, blk_cnt, currtile, nDataSize;
	WORD m;
	BOOL blood_flag;
	DWORD *pFrameTable;
	MICROS *pMap;
#ifndef USE_ASM
	int k, l;
	BYTE width, pix;
	BYTE *src, *dst, *tbl;
#endif

	for (i = 0; i < MAXTILES; i++) {
		tile_defs[i] = i;
		level_frame_count[i] = 0;
		level_frame_types[i] = 0;
	}

	if (leveltype != DTYPE_HELL)
		blocks = 10;
	else
		blocks = 12;

	for (y = 0; y < MAXDUNY; y++) {
		for (x = 0; x < MAXDUNX; x++) {
			for (i = 0; i < blocks; i++) {
				pMap = &dpiece_defs_map_2[x][y];
				if (pMap->mt[i]) {
					level_frame_count[pMap->mt[i] & 0xFFF]++;
					level_frame_types[pMap->mt[i] & 0xFFF] = pMap->mt[i] & 0x7000;
				}
			}
		}
	}

	pFrameTable = (DWORD *)pDungeonCels;
	nlevel_frames = pFrameTable[0] & 0xFFFF;

	for (i = 1; i < nlevel_frames; i++) {
#ifdef USE_ASM
		__asm {
			mov		ebx, pDungeonCels
			mov		eax, i
			shl		eax, 2
			add		ebx, eax
			mov		eax, [ebx+4]
			sub		eax, [ebx]
			mov		nDataSize, eax
		}
#else
		nDataSize = pFrameTable[i + 1] - pFrameTable[i];
#endif
		level_frame_sizes[i] = nDataSize & 0xFFFF;
	}

	level_frame_sizes[0] = 0;

	if (leveltype == DTYPE_HELL) {
		for (i = 0; i < nlevel_frames; i++) {
			if (!i)
				level_frame_count[0] = 0;
			blood_flag = TRUE;
			if (level_frame_count[i]) {
				if (level_frame_types[i] != 0x1000) {
#ifdef USE_ASM
					j = level_frame_sizes[i];
					__asm {
						mov		ebx, pDungeonCels
						mov		eax, i
						shl		eax, 2
						add		ebx, eax
						mov		esi, pDungeonCels
						add		esi, [ebx]
						xor		ebx, ebx
						mov		ecx, j
						jecxz	l1_label3
					l1_label1:
						lodsb
						cmp		al, 0
						jz		l1_label2
						cmp		al, 32
						jnb		l1_label2
						mov		blood_flag, ebx
					l1_label2:
						loop	l1_label1
					l1_label3:
						nop
					}
#else
					src = &pDungeonCels[pFrameTable[i]];
					for (j = level_frame_sizes[i]; j; j--) {
						pix = *src++;
						if (pix && pix < 32)
							blood_flag = FALSE;
					}
#endif
				} else {
#ifdef USE_ASM
					__asm {
						mov		ebx, pDungeonCels
						mov		eax, i
						shl		eax, 2
						add		ebx, eax
						mov		esi, pDungeonCels
						add		esi, [ebx]
						xor		ebx, ebx
						mov		ecx, 32
					l2_label1:
						push	ecx
						mov		edx, 32
					l2_label2:
						xor		eax, eax
						lodsb
						or		al, al
						js		l2_label5
						sub		edx, eax
						mov		ecx, eax
					l2_label3:
						lodsb
						cmp		al, 0
						jz		l2_label4
						cmp		al, 32
						jnb		l2_label4
						mov		blood_flag, ebx
					l2_label4:
						loop	l2_label3
						or		edx, edx
						jz		l2_label6
						jmp		l2_label2
					l2_label5:
						neg		al
						sub		edx, eax
						jnz		l2_label2
					l2_label6:
						pop		ecx
						loop	l2_label1
					}
#else
					src = &pDungeonCels[pFrameTable[i]];
					for (k = 32; k; k--) {
						for (l = 32; l;) {
							width = *src++;
							if (!(width & 0x80)) {
								l -= width;
								while (width) {
									pix = *src++;
									if (pix && pix < 32)
										blood_flag = FALSE;
									width--;
								}
							} else {
								width = -(char)width;
								l -= width;
							}
						}
					}
#endif
				}
				if (!blood_flag)
					level_frame_count[i] = 0;
			}
		}
	}

	SortTiles(MAXTILES - 1);
	total_size = 0;
	total_frames = 0;

	if (light4flag) {
		while (total_size < 0x100000) {
			total_size += level_frame_sizes[total_frames] << 1;
			total_frames++;
		}
	} else {
		while (total_size < 0x100000) {
			total_size += (level_frame_sizes[total_frames] << 4) - (level_frame_sizes[total_frames] << 1);
			total_frames++;
		}
	}

	total_frames--;
	if (total_frames > 128)
		total_frames = 128;

	frameidx = 0; /* move into loop ? */

	if (light4flag)
		blk_cnt = 3;
	else
		blk_cnt = 15;

	for (i = 0; i < total_frames; i++) {
		currtile = tile_defs[i];
		SpeedFrameTbl[i][0] = currtile;
		if (level_frame_types[i] != 0x1000) {
			lfs_adder = level_frame_sizes[i];
			for (j = 1; j < blk_cnt; j++) {
				SpeedFrameTbl[i][j] = frameidx;
#ifdef USE_ASM
				__asm {
					mov		ebx, pDungeonCels
					mov		eax, currtile
					shl		eax, 2
					add		ebx, eax
					mov		esi, pDungeonCels
					add		esi, [ebx]
					mov		edi, pSpeedCels
					add		edi, frameidx
					mov		ebx, j
					shl		ebx, 8
					add		ebx, pLightTbl
					mov		ecx, lfs_adder
					jecxz	l3_label2
				l3_label1:
					lodsb
					xlat
					stosb
					loop	l3_label1
				l3_label2:
					nop
				}
#else
				src = &pDungeonCels[pFrameTable[currtile]];
				dst = &pSpeedCels[frameidx];
				tbl = &pLightTbl[256 * j];
				for (k = lfs_adder; k; k--) {
					*dst++ = tbl[*src++];
				}
#endif
				frameidx += lfs_adder;
			}
		} else {
			for (j = 1; j < blk_cnt; j++) {
				SpeedFrameTbl[i][j] = frameidx;
#ifdef USE_ASM
				__asm {
					mov		ebx, pDungeonCels
					mov		eax, currtile
					shl		eax, 2
					add		ebx, eax
					mov		esi, pDungeonCels
					add		esi, [ebx]
					mov		edi, pSpeedCels
					add		edi, frameidx
					mov		ebx, j
					shl		ebx, 8
					add		ebx, pLightTbl
					mov		ecx, 32
				l4_label1:
					push	ecx
					mov		edx, 32
				l4_label2:
					xor		eax, eax
					lodsb
					stosb
					or		al, al
					js		l4_label4
					sub		edx, eax
					mov		ecx, eax
				l4_label3:
					lodsb
					xlat
					stosb
					loop	l4_label3
					or		edx, edx
					jz		l4_label5
					jmp		l4_label2
				l4_label4:
					neg		al
					sub		edx, eax
					jnz		l4_label2
				l4_label5:
					pop		ecx
					loop	l4_label1
				}
#else
				src = &pDungeonCels[pFrameTable[currtile]];
				dst = &pSpeedCels[frameidx];
				tbl = &pLightTbl[256 * j];
				for (k = 32; k; k--) {
					for (l = 32; l;) {
						width = *src++;
						*dst++ = width;
						if (!(width & 0x80)) {
							l -= width;
							while (width) {
								*dst++ = tbl[*src++];
								width--;
							}
						} else {
							width = -(char)width;
							l -= width;
						}
					}
				}
#endif
				frameidx += level_frame_sizes[i];
			}
		}
	}

	for (y = 0; y < MAXDUNY; y++) {
		for (x = 0; x < MAXDUNX; x++) {
			if (dPiece[x][y]) {
				pMap = &dpiece_defs_map_2[x][y];
				for (i = 0; i < blocks; i++) {
					if (pMap->mt[i]) {
						for (m = 0; m < total_frames; m++) {
							if ((pMap->mt[i] & 0xFFF) == tile_defs[m]) {
								pMap->mt[i] = m + level_frame_types[m] + 0x8000;
								m = total_frames;
							}
						}
					}
				}
			}
		}
	}
}
// 525728: using guessed type int light4flag;
// 53CD4C: using guessed type int nlevel_frames;

void SortTiles(int frames)
{
	int i;
	BOOL doneflag;

	doneflag = FALSE;
	while (frames > 0 && !doneflag) {
		doneflag = TRUE;
		for (i = 0; i < frames; i++) {
			if (level_frame_count[i] < level_frame_count[i + 1]) {
				SwapTile(i, i + 1);
				doneflag = FALSE;
			}
		}
		frames--;
	}
}

void SwapTile(int f1, int f2)
{
	int swap;

	swap = level_frame_count[f1];
	level_frame_count[f1] = level_frame_count[f2];
	level_frame_count[f2] = swap;
	swap = tile_defs[f1];
	tile_defs[f1] = tile_defs[f2];
	tile_defs[f2] = swap;
	swap = level_frame_types[f1];
	level_frame_types[f1] = level_frame_types[f2];
	level_frame_types[f2] = swap;
	swap = level_frame_sizes[f1];
	level_frame_sizes[f1] = level_frame_sizes[f2];
	level_frame_sizes[f2] = swap;
}

int IsometricCoord(int x, int y)
{
	if (x < MAXDUNY - y)
		return (y + y * y + x * (x + 2 * y + 3)) / 2;

	x = MAXDUNX - x - 1;
	y = MAXDUNY - y - 1;
	return MAXDUNX * MAXDUNY - ((y + y * y + x * (x + 2 * y + 3)) / 2) - 1;
}

void SetSpeedCels()
{
	int x, y;

	for (x = 0; x < MAXDUNX; x++) {
		for (y = 0; y < MAXDUNY; y++) {
			dpiece_defs_map_1[IsometricCoord(x, y)] = dpiece_defs_map_2[x][y];
		}
	}
}

void SetDungeonMicros()
{
	int i, x, y, lv, blocks;
	WORD *pPiece;
	MICROS *pMap;

	if (leveltype != DTYPE_HELL) {
		MicroTileLen = 10;
		blocks = 10;
	} else {
		MicroTileLen = 12;
		blocks = 16;
	}

	for (y = 0; y < MAXDUNY; y++) {
		for (x = 0; x < MAXDUNX; x++) {
			lv = dPiece[x][y];
			pMap = &dpiece_defs_map_2[x][y];
			if (lv) {
				lv--;
				if (leveltype != DTYPE_HELL)
					pPiece = (WORD *)&pLevelPieces[20 * lv];
				else
					pPiece = (WORD *)&pLevelPieces[32 * lv];
				for (i = 0; i < blocks; i++)
					pMap->mt[i] = pPiece[(i & 1) + blocks - 2 - (i & 0xE)];
			} else {
				for (i = 0; i < blocks; i++)
					pMap->mt[i] = 0;
			}
		}
	}

	MakeSpeedCels();
	SetSpeedCels();

	if (zoomflag) {
		scr_pix_width = 640;
		scr_pix_height = VIEWPORT_HEIGHT;
		dword_5C2FF8 = 10;
		dword_5C2FFC = 11;
	} else {
		scr_pix_width = 384;
		scr_pix_height = 224;
		dword_5C2FF8 = 6;
		dword_5C2FFC = 7;
	}
}
// 52569C: using guessed type int zoomflag;
// 5C2FF8: using guessed type int dword_5C2FF8;
// 5C2FFC: using guessed type int dword_5C2FFC;
// 5C3000: using guessed type int scr_pix_width;
// 5C3004: using guessed type int scr_pix_height;

void DRLG_InitTrans()
{
	memset(dTransVal, 0, sizeof(dTransVal));
	memset(TransList, 0, sizeof(TransList));
	TransVal = 1;
}
// 5A5590: using guessed type char TransVal;

void DRLG_MRectTrans(int x1, int y1, int x2, int y2)
{
	int i, j;

	x1 = 2 * x1 + 17;
	y1 = 2 * y1 + 17;
	x2 = 2 * x2 + 16;
	y2 = 2 * y2 + 16;

	for(j = y1; j <= y2; j++) {
		for(i = x1; i <= x2; i++) {
			dTransVal[i][j] = TransVal;
		}
	}

	TransVal++;
}
// 5A5590: using guessed type char TransVal;

void DRLG_RectTrans(int x1, int y1, int x2, int y2)
{
	int i, j;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			dTransVal[i][j] = TransVal;
		}
	}
	TransVal++;
}
// 5A5590: using guessed type char TransVal;

void DRLG_CopyTrans(int sx, int sy, int dx, int dy)
{
	dTransVal[dx][dy] = dTransVal[sx][sy];
}

void DRLG_ListTrans(int num, unsigned char *List)
{
	int i;
	unsigned char x1, x2, y1, y2;

	for (i = 0; i < num; i++) {
		x1 = *List++;
		y1 = *List++;
		x2 = *List++;
		y2 = *List++;
		DRLG_RectTrans(x1, y1, x2, y2);
	}
}

void DRLG_AreaTrans(int num, unsigned char *List)
{
	int i;
	unsigned char x1, x2, y1, y2;

	for (i = 0; i < num; i++) {
		x1 = *List++;
		y1 = *List++;
		x2 = *List++;
		y2 = *List++;
		DRLG_RectTrans(x1, y1, x2, y2);
		--TransVal;
	}
	++TransVal;
}
// 5A5590: using guessed type char TransVal;

void DRLG_InitSetPC()
{
	setpc_x = 0;
	setpc_y = 0;
	setpc_w = 0;
	setpc_h = 0;
}
// 5CF330: using guessed type int setpc_h;
// 5CF334: using guessed type int setpc_w;

void DRLG_SetPC()
{
	int i, j, x, y, w, h;

	w = 2 * setpc_w;
	h = 2 * setpc_h;
	x = 2 * setpc_x + 16;
	y = 2 * setpc_y + 16;

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			dFlags[i + x][j + y] |= 8;
		}
	}
}
// 5CF330: using guessed type int setpc_h;
// 5CF334: using guessed type int setpc_w;

void Make_SetPC(int x, int y, int w, int h)
{
	int i, j, dx, dy, dh, dw;

	dw = 2 * w;
	dh = 2 * h;
	dx = 2 * x + 16;
	dy = 2 * y + 16;

	for (j = 0; j < dh; j++) {
		for (i = 0; i < dw; i++) {
			dFlags[i + dx][j + dy] |= 8;
		}
	}
}

BOOL DRLG_WillThemeRoomFit(int floor, int x, int y, int minSize, int maxSize, int *width, int *height)
{
	int ii, xx, yy;
	int xSmallest, ySmallest;
	int xArray[20], yArray[20];
	int xCount, yCount;
	BOOL yFlag, xFlag;

	yFlag = TRUE;
	xFlag = TRUE;
	xCount = 0;
	yCount = 0;

	if (x > DMAXX - maxSize && y > DMAXY - maxSize) {
		return FALSE;
	}
	if (!SkipThemeRoom(x, y)) {
		return FALSE;
	}

	memset(xArray, 0, sizeof(xArray));
	memset(yArray, 0, sizeof(yArray));

	for (ii = 0; ii < maxSize; ii++) {
		if (xFlag) {
			for (xx = x; xx < x + maxSize; xx++) {
				if (dungeon[xx][y + ii] != floor) {
					if (xx >= minSize) {
						break;
					}
					xFlag = FALSE;
				} else {
					xCount++;
				}
			}
			if (xFlag) {
				xArray[ii] = xCount;
				xCount = 0;
			}
		}
		if (yFlag) {
			for (yy = y; yy < y + maxSize; yy++) {
				if (dungeon[x + ii][yy] != floor) {
					if (yy >= minSize) {
						break;
					}
					yFlag = FALSE;
				} else {
					yCount++;
				}
			}
			if (yFlag) {
				yArray[ii] = yCount;
				yCount = 0;
			}
		}
	}

	for (ii = 0; ii < minSize; ii++) {
		if (xArray[ii] < minSize || yArray[ii] < minSize) {
			return FALSE;
		}
	}

	xSmallest = xArray[0];
	ySmallest = yArray[0];

	for (ii = 0; ii < maxSize; ii++) {
		if (xArray[ii] < minSize || yArray[ii] < minSize) {
			break;
		}
		if (xArray[ii] < xSmallest) {
			xSmallest = xArray[ii];
		}
		if (yArray[ii] < ySmallest) {
			ySmallest = yArray[ii];
		}
	}

	*width = xSmallest - 2;
	*height = ySmallest - 2;
	return TRUE;
}
// 41965B: using guessed type int var_6C[20];
// 41965B: using guessed type int var_BC[20];

void DRLG_CreateThemeRoom(int themeIndex)
{
	int xx, yy;

	for(yy = themeLoc[themeIndex].y; yy < themeLoc[themeIndex].y + themeLoc[themeIndex].height; yy++) {
		for(xx = themeLoc[themeIndex].x; xx < themeLoc[themeIndex].x + themeLoc[themeIndex].width; xx++) {
			if(leveltype == DTYPE_CATACOMBS) {
				if(yy == themeLoc[themeIndex].y
				&& xx >= themeLoc[themeIndex].x
				&& xx <= themeLoc[themeIndex].x + themeLoc[themeIndex].width
				|| yy == themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1
				&& xx >= themeLoc[themeIndex].x
				&& xx <= themeLoc[themeIndex].x + themeLoc[themeIndex].width) {
					dungeon[xx][yy] = 2;
				} else if(xx == themeLoc[themeIndex].x
				&& yy >= themeLoc[themeIndex].y
				&& yy <= themeLoc[themeIndex].y + themeLoc[themeIndex].height
				|| xx == themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1
				&& yy >= themeLoc[themeIndex].y
				&& yy <= themeLoc[themeIndex].y + themeLoc[themeIndex].height) {
					dungeon[xx][yy] = 1;
				} else {
					dungeon[xx][yy] = 3;
				}
			}
			if(leveltype == DTYPE_CAVES) {
				if(yy == themeLoc[themeIndex].y
				&& xx >= themeLoc[themeIndex].x
				&& xx <= themeLoc[themeIndex].x + themeLoc[themeIndex].width
				|| yy == themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1
				&& xx >= themeLoc[themeIndex].x
				&& xx <= themeLoc[themeIndex].x + themeLoc[themeIndex].width) {
					dungeon[xx][yy] = 134;
				} else if(xx == themeLoc[themeIndex].x
				&& yy >= themeLoc[themeIndex].y
				&& yy <= themeLoc[themeIndex].y + themeLoc[themeIndex].height
				|| xx == themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1
				&& yy >= themeLoc[themeIndex].y
				&& yy <= themeLoc[themeIndex].y + themeLoc[themeIndex].height) {
					dungeon[xx][yy] = 137;
				} else {
					dungeon[xx][yy] = 7;
				}
			}
			if(leveltype == DTYPE_HELL) {
				if(yy == themeLoc[themeIndex].y
				&& xx >= themeLoc[themeIndex].x
				&& xx <= themeLoc[themeIndex].x + themeLoc[themeIndex].width
				|| yy == themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1
				&& xx >= themeLoc[themeIndex].x
				&& xx <= themeLoc[themeIndex].x + themeLoc[themeIndex].width) {
					dungeon[xx][yy] = 2;
				} else if(xx == themeLoc[themeIndex].x
				&& yy >= themeLoc[themeIndex].y
				&& yy <= themeLoc[themeIndex].y + themeLoc[themeIndex].height
				|| xx == themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1
				&& yy >= themeLoc[themeIndex].y
				&& yy <= themeLoc[themeIndex].y + themeLoc[themeIndex].height) {
					dungeon[xx][yy] = 1;
				} else {
					dungeon[xx][yy] = 6;
				}
			}
		}
	}

	if(leveltype == DTYPE_CATACOMBS) {
		dungeon[themeLoc[themeIndex].x][themeLoc[themeIndex].y] = 8;
		dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1][themeLoc[themeIndex].y] = 7;
		dungeon[themeLoc[themeIndex].x][themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1] = 9;
		dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1][themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1] = 6;
	}
	if(leveltype == DTYPE_CAVES) {
		dungeon[themeLoc[themeIndex].x][themeLoc[themeIndex].y] = 150;
		dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1][themeLoc[themeIndex].y] = 151;
		dungeon[themeLoc[themeIndex].x][themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1] = 152;
		dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1][themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1] = 138;
	}
	if(leveltype == DTYPE_HELL) {
		dungeon[themeLoc[themeIndex].x][themeLoc[themeIndex].y] = 9;
		dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1][themeLoc[themeIndex].y] = 16;
		dungeon[themeLoc[themeIndex].x][themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1] = 15;
		dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1][themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1] = 12;
	}

	if(leveltype == DTYPE_CATACOMBS) {
		switch(random(0, 2)) {
		case 0:
			dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1][themeLoc[themeIndex].y + themeLoc[themeIndex].height / 2] = 4;
			break;
		case 1:
			dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width / 2][themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1] = 5;
			break;
		}
	}
	if(leveltype == DTYPE_CAVES) {
		switch(random(0, 2)) {
		case 0:
			dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1][themeLoc[themeIndex].y + themeLoc[themeIndex].height / 2] = 147;
			break;
		case 1:
			dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width / 2][themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1] = 146;
			break;
		}
	}
	if(leveltype == DTYPE_HELL) {
		switch(random(0, 2)) {
		case 0:
			dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1][themeLoc[themeIndex].y + themeLoc[themeIndex].height / 2 - 1] = 53;
			dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1][themeLoc[themeIndex].y + themeLoc[themeIndex].height / 2] = 6;
			dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width - 1][themeLoc[themeIndex].y + themeLoc[themeIndex].height / 2 + 1] = 52;
			dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width - 2][themeLoc[themeIndex].y + themeLoc[themeIndex].height / 2 - 1] = 54;
			break;
		case 1:
			dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width / 2 - 1][themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1] = 57;
			dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width / 2][themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1] = 6;
			dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width / 2 + 1][themeLoc[themeIndex].y + themeLoc[themeIndex].height - 1] = 56;
			dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width / 2][themeLoc[themeIndex].y + themeLoc[themeIndex].height - 2] = 59;
			dungeon[themeLoc[themeIndex].x + themeLoc[themeIndex].width / 2 - 1][themeLoc[themeIndex].y + themeLoc[themeIndex].height - 2] = 58;
			break;
		}
	}
}

void DRLG_PlaceThemeRooms(int minSize, int maxSize, int floor, int freq, int rndSize)
{
	int v5; // ebx
	//int v7; // eax
	int v8;             // esi
	int v9;             // edi
	int v10;            // eax
	int v12;            // eax
	int v14;            // eax
	int v16;            // eax
	int v17;            // edi
	int v18;            // esi
	int v19;            // ecx
	int v20;            // ecx
	int v21;            // eax
	int minSize2;       // [esp+10h] [ebp-1Ch]
	int maxSize2;       // [esp+14h] [ebp-18h]
	unsigned char *v24; // [esp+18h] [ebp-14h]
	signed int x_start; // [esp+1Ch] [ebp-10h]
	int x;              // [esp+20h] [ebp-Ch]
	int width;          // [esp+24h] [ebp-8h]
	int height;         // [esp+28h] [ebp-4h]

	v5 = 0;
	maxSize2 = maxSize;
	minSize2 = minSize;
	themeCount = 0;
	memset(themeLoc, 0, sizeof(*themeLoc));
	do {
		x = 0;
		x_start = 20;
		v24 = (unsigned char *)dungeon + v5;
		do {
			if (*v24 == floor) {
				if (!random(0, freq)) {
					//_LOBYTE(v7) = DRLG_WillThemeRoomFit(floor, x, v5, minSize2, maxSize2, &width, &height);
					if (DRLG_WillThemeRoomFit(floor, x, v5, minSize2, maxSize2, &width, &height)) {
						if (rndSize) {
							v8 = minSize2 - 2;
							v9 = maxSize2 - 2;
							v10 = random(0, width - (minSize2 - 2) + 1);
							v12 = minSize2 - 2 + random(0, v10);
							if (v12 < minSize2 - 2 || (width = v12, v12 > v9))
								width = minSize2 - 2;
							v14 = random(0, height - v8 + 1);
							v16 = v8 + random(0, v14);
							if (v16 < v8 || v16 > v9)
								v16 = minSize2 - 2;
							height = v16;
						} else {
							v16 = height;
						}
						v17 = themeCount;
						v18 = themeCount;
						themeLoc[v18].x = x + 1;
						themeLoc[v18].y = v5 + 1;
						v19 = width;
						themeLoc[v18].width = width;
						themeLoc[v18].height = v16;
						v20 = x + v19;
						v21 = v5 + v16;
						if (leveltype == DTYPE_CAVES)
							DRLG_RectTrans(x_start, 2 * v5 + 20, 2 * v20 + 15, 2 * v21 + 15);
						else
							DRLG_MRectTrans(x + 1, v5 + 1, v20, v21);
						themeLoc[v18].ttval = TransVal - 1;
						DRLG_CreateThemeRoom(v17);
						++themeCount;
					}
				}
			}
			x_start += 2;
			++x;
			v24 += 40;
		} while (x_start < 100);
		++v5;
	} while (v5 < 40);
}
// 5A5590: using guessed type char TransVal;

void DRLG_HoldThemeRooms()
{
	int i, x, y, xx, yy;

	for(i = 0; i < themeCount; i++) {
		for(y = themeLoc[i].y; y < themeLoc[i].y + themeLoc[i].height - 1; y++) {
			for(x = themeLoc[i].x; x < themeLoc[i].x + themeLoc[i].width - 1; x++) {
				xx = 2 * x + 16;
				yy = 2 * y + 16;
				dFlags[xx][yy] |= BFLAG_POPULATED;
				dFlags[xx + 1][yy] |= BFLAG_POPULATED;
				dFlags[xx][yy + 1] |= BFLAG_POPULATED;
				dFlags[xx + 1][yy + 1] |= BFLAG_POPULATED;
			}
		}
	}
}

BOOL SkipThemeRoom(int x, int y)
{
	int i;

	for (i = 0; i < themeCount; i++) {
		if (x >= themeLoc[i].x - 2 && x <= themeLoc[i].x + themeLoc[i].width + 2
		    && y >= themeLoc[i].y - 2 && y <= themeLoc[i].y + themeLoc[i].height + 2)
			return 0;
	}

	return 1;
}

void InitLevels()
{
	if (!leveldebug) {
		currlevel = 0;
		leveltype = 0;
		setlevel = 0;
	}
}
// 52572C: using guessed type int leveldebug;
// 5CF31D: using guessed type char setlevel;

DEVILUTION_END_NAMESPACE
