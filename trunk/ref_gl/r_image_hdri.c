/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "r_local.h"

typedef unsigned char RGBE[4];
#define R			0
#define G			1
#define B			2
#define E			3

#define  MINELEN	8				// minimum scanline length for encoding
#define  MAXELEN	0x7fff			// maximum scanline length for encoding

float convertComponent(int expo, int val)
{
	float v = val / 256.0f;
	float d = (float)pow(2, expo);
	return v * d;
}

void workOnRGBE(RGBE* scan, int len, float* cols)
{
	while (len-- > 0) {
		int expo = scan[0][E] - 128;
		cols[0] = convertComponent(expo, scan[0][R]);
		cols[1] = convertComponent(expo, scan[0][G]);
		cols[2] = convertComponent(expo, scan[0][B]);
		cols += 3;
		scan++;
	}
}

qboolean oldDecrunch(RGBE* scanline, int len, FILE* file)
{
	int i;
	int rshift = 0;

	while (len > 0) {
		scanline[0][R] = fgetc(file);
		scanline[0][G] = fgetc(file);
		scanline[0][B] = fgetc(file);
		scanline[0][E] = fgetc(file);
		if (feof(file))
			return qfalse;

		if (scanline[0][R] == 1 &&
			scanline[0][G] == 1 &&
			scanline[0][B] == 1) {
			for (i = scanline[0][E] << rshift; i > 0; i--) {
				memcpy(&scanline[0][0], &scanline[-1][0], 4);
				scanline++;
				len--;
			}
			rshift += 8;
		}
		else {
			scanline++;
			len--;
			rshift = 0;
		}
	}
	return qtrue;
}

qboolean decrunch(RGBE* scanline, int len, FILE* file)
{
	int  i, j;

	if (len < MINELEN || len > MAXELEN)
		return oldDecrunch(scanline, len, file);

	i = fgetc(file);
	if (i != 2) {
		fseek(file, -1, SEEK_CUR);
		return oldDecrunch(scanline, len, file);
	}

	scanline[0][G] = fgetc(file);
	scanline[0][B] = fgetc(file);
	i = fgetc(file);

	if (scanline[0][G] != 2 || scanline[0][B] & 128) {
		scanline[0][R] = 2;
		scanline[0][E] = i;
		return oldDecrunch(scanline + 1, len - 1, file);
	}

	// read each component
	for (i = 0; i < 4; i++) {
		for (j = 0; j < len; ) {
			unsigned char code = fgetc(file);
			if (code > 128) { // run
				code &= 127;
				unsigned char val = fgetc(file);
				while (code--)
					scanline[j++][i] = val;
			}
			else {	// non-run
				while (code--)
					scanline[j++][i] = fgetc(file);
			}
		}
	}

	return feof(file) ? qfalse : qtrue;
}

qboolean R_LoadHdri(const char* fileName, hdri_t* res)
{
	int i;
	char str[200];
	FILE* file;

	file = fopen(fileName, "rb");
	if (!file)
		return qfalse;

	fread(str, 10, 1, file);
	if (memcmp(str, "#?RADIANCE", 10)) {
		fclose(file);
		return qfalse;
	}

	fseek(file, 1, SEEK_CUR);

	char cmd[200];
	i = 0;
	char c = 0, oldc;
	while (qtrue) {
		oldc = c;
		c = fgetc(file);
		if (c == 0xa && oldc == 0xa)
			break;
		cmd[i++] = c;
	}

	char reso[200];
	i = 0;
	while (qtrue) {
		c = fgetc(file);
		reso[i++] = c;
		if (c == 0xa)
			break;
	}

	int w, h;
	if (!sscanf(reso, "-Y %ld +X %ld", &h, &w)) {
		fclose(file);
		return qfalse;
	}

	res->width = w;
	res->height = h;

	float* cols = malloc(w * h * 3 * sizeof(float));
	res->data = cols;

	RGBE* scanline = malloc(w * sizeof(RGBE));
	if (!scanline) {
		fclose(file);
		return qfalse;
	}

	// convert image 
	for (int y = h - 1; y >= 0; y--) {
		if (decrunch(scanline, w, file) == qfalse)
			break;
		workOnRGBE(scanline, w, cols);
		cols += w * 3;
	}

	free(scanline);
	fclose(file);

	return qtrue;
}
