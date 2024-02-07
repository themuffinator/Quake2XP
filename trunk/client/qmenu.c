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
#include <string.h>
#include <ctype.h>

#include "client.h"
#include "qmenu.h"

static void Action_DoEnter (menuaction_s * a);
static void Action_Draw (menuaction_s * a);
static void Menu_DrawStatusBar (const char *string);
static void Menulist_DoEnter (menulist_s * l);
static void MenuList_Draw (menulist_s * l);
static void Separator_Draw (menuseparator_s * s);
static void Slider_DoSlide (menuslider_s * s, int dir);
static void Slider_Draw (menuslider_s * s);
static void SpinControl_DoEnter (menulist_s * s);
static void SpinControl_Draw (menulist_s * s);
static void SpinControl_DoSlide (menulist_s * s, int dir);

#define RCOLUMN_OFFSET  32
#define LCOLUMN_OFFSET -16

extern viddef_t viddef;

#define VID_WIDTH viddef.width
#define VID_HEIGHT viddef.height



#define Draw_Fill Draw_Fill

void Action_DoEnter (menuaction_s * a) {
	if (a->generic.callback)
		a->generic.callback (a);
}

//action items and sub menus
void Action_Draw (menuaction_s * a) {
	int	fontscale = ui_fontScale->integer;

	if (a->generic.flags & QMF_LEFT_JUSTIFY) {
		if (a->generic.flags & QMF_GRAYED)
			Menu_DrawStringDark (a->generic.x + a->generic.parent->x +
			LCOLUMN_OFFSET,
			a->generic.y + a->generic.parent->y,
			a->generic.name);
		else
			CL_AddString(a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET*fontscale,
			a->generic.y + a->generic.parent->y,
			fontscale,
			(char*)a->generic.name, menuFont);
	}
	else {
		if (a->generic.flags & QMF_GRAYED)
			Menu_DrawStringR2LDark (a->generic.x + a->generic.parent->x +
			LCOLUMN_OFFSET,
			a->generic.y + a->generic.parent->y,
			a->generic.name);
		else
			Menu_DrawStringR2L (a->generic.x + a->generic.parent->x +
			LCOLUMN_OFFSET,
			a->generic.y + a->generic.parent->y,
			a->generic.name);
	}
	if (a->generic.ownerdraw)
		a->generic.ownerdraw (a);
}

qboolean Field_DoEnter (menufield_s * f) {
	if (f->generic.callback) {
		f->generic.callback (f);
		return qtrue;
	}
	return qfalse;
}
// mp menu  values in field
void Field_Draw (menufield_s * f) {
	int i;
	char tempbuffer[128] = "";
	int fontscale = ui_fontScale->integer;

	if (f->generic.name)
		Menu_DrawStringR2LDark (f->generic.x + f->generic.parent->x +
		LCOLUMN_OFFSET,
		f->generic.y + f->generic.parent->y,
		f->generic.name);

	strncpy (tempbuffer, f->buffer + f->visible_offset, f->visible_length);

	// top left part of text box
	R_AddCharsToList(f->generic.x + f->generic.parent->x + 16 / fontscale,
		f->generic.y + f->generic.parent->y - 4,
		fontscale,
		18, menuFont);
	// buttom left
	R_AddCharsToList(f->generic.x + f->generic.parent->x + 16 / fontscale,
		f->generic.y + f->generic.parent->y + 4,
		fontscale,
		24, menuFont);

	//right top
	R_AddCharsToList(f->generic.x + f->generic.parent->x + 24 + f->visible_length * 8 * fontscale,
		f->generic.y + f->generic.parent->y - 4,
		fontscale,
		20, menuFont);
	//right buttom
	R_AddCharsToList(f->generic.x + f->generic.parent->x + 24 + f->visible_length * 8 * fontscale,
		f->generic.y + f->generic.parent->y + 4,
		fontscale,
		26, menuFont);

	for (i = 0; i < f->visible_length; i++) {

		R_AddCharsToList(f->generic.x + f->generic.parent->x + 24 + i * 8 * fontscale,
			f->generic.y + f->generic.parent->y - 4,
			fontscale,
			19, menuFont);

		R_AddCharsToList(f->generic.x + f->generic.parent->x + 24 + i * 8 * fontscale,
			f->generic.y + f->generic.parent->y + 4,
			fontscale,
			25, menuFont);
	}

		CL_AddString (f->generic.x + f->generic.parent->x + 24,
		f->generic.y + f->generic.parent->y,
		fontscale,
		tempbuffer, menuFont);

	if (Menu_ItemAtCursor (f->generic.parent) == f) {
		int offset;

		if (f->visible_offset)
			offset = f->visible_length * fontscale * CURSOR_INTERVAL;
		else
			offset = f->cursor * fontscale * CURSOR_INTERVAL;

		if (((int)(Sys_Milliseconds () / 250)) & 1) {
			R_AddCharsToList(f->generic.x + f->generic.parent->x + (offset + 2) * 8 + 8,
				f->generic.y + f->generic.parent->y,
				fontscale,
				11, menuFont);
		}
		else {
			R_AddCharsToList(f->generic.x + f->generic.parent->x + (offset + 2) * 8 + 8,
				f->generic.y + f->generic.parent->y,
				fontscale,
				' ', menuFont);
		}
	}
}

qboolean Field_Key (menufield_s * f, int key) {
	extern int keydown[];

	switch (key) {
		case K_KP_SLASH:
			key = '/';
			break;
		case K_KP_MINUS:
			key = '-';
			break;
		case K_KP_PLUS:
			key = '+';
			break;
		case K_KP_HOME:
			key = '7';
			break;
		case K_KP_UPARROW:
			key = '8';
			break;
		case K_KP_PGUP:
			key = '9';
			break;
		case K_KP_LEFTARROW:
			key = '4';
			break;
		case K_KP_5:
			key = '5';
			break;
		case K_KP_RIGHTARROW:
			key = '6';
			break;
		case K_KP_END:
			key = '1';
			break;
		case K_KP_DOWNARROW:
			key = '2';
			break;
		case K_KP_PGDN:
			key = '3';
			break;
		case K_KP_INS:
			key = '0';
			break;
		case K_KP_DEL:
			key = '.';
			break;
	}

	if (key > 127) {
		switch (key) {
			case K_DEL:
			default:
				return qfalse;
		}
	}

	/*
	 ** support pasting from the clipboard
	 */
	if ((toupper (key) == 'V' && keydown[K_CTRL]) ||
		(((key == K_INS) || (key == K_KP_INS)) && keydown[K_SHIFT])) {
		char *cbd;

		if ((cbd = Sys_GetClipboardData ()) != 0) {
			strtok (cbd, "\n\r\b");

			strncpy (f->buffer, cbd, f->length - 1);
			f->cursor = strlen (f->buffer);
			f->visible_offset = f->cursor - f->visible_length;
			if (f->visible_offset < 0)
				f->visible_offset = 0;

			free (cbd);
		}
		return qtrue;
	}

	switch (key) {
		case K_KP_LEFTARROW:
		case K_LEFTARROW:
		case K_BACKSPACE:
			if (f->cursor > 0) {
				memmove (&f->buffer[f->cursor - 1], &f->buffer[f->cursor],
					strlen (&f->buffer[f->cursor]) + 1);
				f->cursor--;

				if (f->visible_offset) {
					f->visible_offset--;
				}
			}
			break;

		case K_KP_DEL:
		case K_DEL:
			memmove (&f->buffer[f->cursor], &f->buffer[f->cursor + 1],
				strlen (&f->buffer[f->cursor + 1]) + 1);
			break;

		case K_KP_ENTER:
		case K_ENTER:
		case K_ESCAPE:
		case K_TAB:
			return qfalse;

		case K_SPACE:
		default:
			if (!isdigit (key) && (f->generic.flags & QMF_NUMBERSONLY))
				return qfalse;

			if (f->cursor < f->length) {
				f->buffer[f->cursor++] = key;
				f->buffer[f->cursor] = 0;

				if (f->cursor > f->visible_length) {
					f->visible_offset++;
				}
			}
	}

	return qtrue;
}

void Menu_AddItem (menuframework_s * menu, void *item) {
	if (menu->nitems == 0)
		menu->nslots = 0;

	if (menu->nitems < MAXMENUITEMS) {
		menu->items[menu->nitems] = item;
		((menucommon_s *)menu->items[menu->nitems])->parent = menu;
		menu->nitems++;
	}

	menu->nslots = Menu_TallySlots (menu);
}

/*
** Menu_AdjustCursor
**
** This function takes the given menu, the direction, and attempts
** to adjust the menu's cursor so that it's at the next available
** slot.
*/
void Menu_AdjustCursor (menuframework_s * m, int dir) {
	menucommon_s *citem;

	/*
	 ** see if it's in a valid spot
	 */
	if (m->cursor >= 0 && m->cursor < m->nitems) {
		if ((citem = Menu_ItemAtCursor (m)) != 0) {
			if (citem->type != MTYPE_SEPARATOR)
				return;
		}
	}

	/*
	 ** it's not in a valid spot, so crawl in the direction indicated until we
	 ** find a valid spot
	 */
	if (dir == 1) {
		while (1) {
			citem = Menu_ItemAtCursor (m);
			if (citem)
			if (citem->type != MTYPE_SEPARATOR)
				break;
			m->cursor += dir;
			if (m->cursor >= m->nitems)
				m->cursor = 0;
		}
	}
	else {
		while (1) {
			citem = Menu_ItemAtCursor (m);
			if (citem)
			if (citem->type != MTYPE_SEPARATOR)
				break;
			m->cursor += dir;
			if (m->cursor < 0)
				m->cursor = m->nitems - 1;
		}
	}
}

void Menu_Center (menuframework_s * menu) {
	int height;

	height = ((menucommon_s *)menu->items[menu->nitems - 1])->y;
	height += 10;

	menu->y = (VID_HEIGHT - height) / 2;
}

void Menu_Draw (menuframework_s * menu) {
	int i;
	menucommon_s *item;

	/*
	 ** draw contents
	 */
	for (i = 0; i < menu->nitems; i++) {
		switch (((menucommon_s *)menu->items[i])->type) {
			case MTYPE_FIELD:
				Field_Draw ((menufield_s *)menu->items[i]);
				break;
			case MTYPE_SLIDER:
				Slider_Draw ((menuslider_s *)menu->items[i]);
				break;
			case MTYPE_LIST:
				MenuList_Draw ((menulist_s *)menu->items[i]);
				break;
			case MTYPE_SPINCONTROL:
				SpinControl_Draw ((menulist_s *)menu->items[i]);
				break;
			case MTYPE_ACTION:
				Action_Draw ((menuaction_s *)menu->items[i]);
				break;
			case MTYPE_SEPARATOR:
				Separator_Draw ((menuseparator_s *)menu->items[i]);
				break;
		}
	}

	item = Menu_ItemAtCursor (menu);


	if (item && item->cursordraw) {
		item->cursordraw (item);
	}
	else if (menu->cursordraw) {
		menu->cursordraw (menu);
	}
	else if (item && item->type != MTYPE_FIELD) {
		if (item->flags & QMF_LEFT_JUSTIFY) {
			R_AddCharsToList(menu->x + item->x - 24 * ui_fontScale->integer + item->cursor_offset+8,
				menu->y + item->y,
				ui_fontScale->integer, 
				12 + ((int)(Sys_Milliseconds () * 0.004) & 1), menuFont);
		}
		else {
			R_AddCharsToList(menu->x + item->cursor_offset*ui_fontScale->integer+8,
				menu->y + item->y,
				ui_fontScale->integer, 
				12 + ((int)(Sys_Milliseconds () * 0.004) & 1), menuFont);
		}
	}

	if (item) {
		if (item->statusbarfunc) {
			item->statusbarfunc((void*)item);
			Menu_DrawStatusBar(item->statusbar);// status bar+status func
		}
		else if (item->statusbar)
			Menu_DrawStatusBar (item->statusbar);
		else
			Menu_DrawStatusBar (menu->statusbar);

	}
	else {
		Menu_DrawStatusBar (menu->statusbar);
	}
}

void Menu_DrawStatusBar (const char *string) {
	int	fontscale = (int)ui_fontScale->integer;
	int	upOffset = 5;

	if (string) {
		int center = ((int)strlen(string) * fontscale * 5) * 0.5;

		Draw_Fill (0, VID_HEIGHT - (12 * fontscale + upOffset), VID_WIDTH, 12 * fontscale, 0.0, 0.35, 0.0, 0.88, qfalse);
		CL_AddString ((VID_WIDTH * 0.5) - center, VID_HEIGHT - (10 * fontscale + upOffset), fontscale, (char*)string, menuFont);
	}
	else {
		Draw_Fill(0, VID_HEIGHT - (12 * fontscale + upOffset), VID_WIDTH, 12 * fontscale, 0.0, 0.0, 0.0, 0.0, qfalse);
	}
}

void Menu_DrawStringDark (int x, int y, const char *string) {
	uint	i;
	int		fontscale = ui_fontScale->integer;

	for (i = 0; i < strlen (string); i++) {
		R_AddCharsToList ((x + i * 8 * fontscale), y, fontscale, string[i] + 128, menuFont);
	}
}

void Menu_DrawStringR2L (int x, int y, const char *string) {
	unsigned	i;
	int		fontscale = ui_fontScale->integer;

	for (i = 0; i < strlen (string); i++) {
		R_AddCharsToList((x - i * 8 * fontscale), y, fontscale, string[strlen (string) - i - 1], menuFont);
	}
}

void Menu_DrawStringR2LDark (int x, int y, const char *string) {
	unsigned	i;
	int		fontscale = ui_fontScale->integer;

	for (i = 0; i < strlen (string); i++) {
		R_AddCharsToList ((x - i * 8 * fontscale), y, fontscale, string[strlen (string) - i - 1] + 128, menuFont);
	}
}

void *Menu_ItemAtCursor (menuframework_s * m) {
	if (m->cursor < 0 || m->cursor >= m->nitems)
		return 0;

	return m->items[m->cursor];
}

qboolean Menu_SelectItem (menuframework_s * s) {
	menucommon_s *item = (menucommon_s *)Menu_ItemAtCursor (s);

	if (item) {
		switch (item->type) {
			case MTYPE_FIELD:
				return Field_DoEnter ((menufield_s *)item);
			case MTYPE_ACTION:
				Action_DoEnter ((menuaction_s *)item);
				return qtrue;
			case MTYPE_LIST:
				//          Menulist_DoEnter( ( menulist_s * ) item );
				return qfalse;
			case MTYPE_SPINCONTROL:
				//          SpinControl_DoEnter( ( menulist_s * ) item );
				return qfalse;
		}
	}
	return qfalse;
}

void Menu_SetStatusBar (menuframework_s * m, const char *string) {
	m->statusbar = string;
}

void Menu_SlideItem (menuframework_s * s, int dir) {
	menucommon_s *item = (menucommon_s *)Menu_ItemAtCursor (s);

	if (item) {
		switch (item->type) {
			case MTYPE_SLIDER:
				Slider_DoSlide ((menuslider_s *)item, dir);
				break;
			case MTYPE_SPINCONTROL:
				SpinControl_DoSlide ((menulist_s *)item, dir);
				break;
		}
	}

}

int Menu_TallySlots (menuframework_s * menu) {
	int i;
	int total = 0;

	for (i = 0; i < menu->nitems; i++) {
		if (((menucommon_s *)menu->items[i])->type == MTYPE_LIST) {
			int nitems = 0;
			char **n = ((menulist_s *)menu->items[i])->itemnames;

			while (*n)
				nitems++, n++;

			total += nitems;
		}
		else {
			total++;
		}
	}

	return total;
}

void Menulist_DoEnter (menulist_s * l) {
	int start;

	start = l->generic.y / 10 + 1;

	l->curInteger = l->generic.parent->cursor - start;

	if (l->generic.callback)
		l->generic.callback (l);
}

void MenuList_Draw (menulist_s * l) {
	char **n;
	int y = 0;

	Menu_DrawStringR2LDark (l->generic.x + l->generic.parent->x +
		LCOLUMN_OFFSET,
		l->generic.y + l->generic.parent->y,
		l->generic.name);
	
	n = l->itemnames;

	Draw_Fill (l->generic.x - 112 + l->generic.parent->x,
		l->generic.parent->y + l->generic.y + l->curInteger * 10 + 10,
		128, 10, 1.0, 0.0, 1.0, 1.0, qfalse);
	
	while (*n) {
		Menu_DrawStringR2LDark (l->generic.x + l->generic.parent->x +
			LCOLUMN_OFFSET,
			l->generic.y + l->generic.parent->y + y +
			10, *n);

		n++;
		y += 10;
	}

}

void Separator_Draw (menuseparator_s * s) {

	if (s->generic.name)
		Menu_DrawStringR2LDark (s->generic.x + s->generic.parent->x,
		s->generic.y + s->generic.parent->y,
		s->generic.name);
}

void Slider_DoSlide (menuslider_s * s, int dir) {
	s->curValue += dir;

	if (s->curValue > s->maxValue)
		s->curValue = s->maxValue;
	else if (s->curValue < s->minValue)
		s->curValue = s->minValue;

	if (s->generic.callback)
		s->generic.callback (s);
}

#define SLIDER_RANGE 10

void Slider_Draw (menuslider_s * s) {
	int		i, shift;
	int	fontscale = ui_fontScale->integer;

	shift = (ui_fontScale->integer - 1) * 8;

	Menu_DrawStringR2LDark (s->generic.x + s->generic.parent->x +
		LCOLUMN_OFFSET,
		s->generic.y + s->generic.parent->y,
		s->generic.name);

	s->range = (s->curValue - s->minValue) / (float)(s->maxValue - s->minValue);

	if (s->range < 0)
		s->range = 0;
	if (s->range > 1)
		s->range = 1;

	// slider line
	for (i = 0; i < SLIDER_RANGE; i++)
		R_AddCharsToList (RCOLUMN_OFFSET + s->generic.x + i * 8 * fontscale + s->generic.parent->x,
		s->generic.y + s->generic.parent->y, fontscale, 129, menuFont);

	//slider	
	R_AddCharsToList((int)(RCOLUMN_OFFSET + s->generic.parent->x + s->generic.x + (SLIDER_RANGE - 1) * 8 * s->range * fontscale),
					s->generic.y + s->generic.parent->y, fontscale, 131, menuFont);

	char data[8];
	if (s->percent) {
		Com_sprintf(data, sizeof(data), " %.0f%%", (s->curValue / s->divRange) * 100);
	} else
		Com_sprintf(data, sizeof(data), " %.1f", s->curValue / s->divRange);
	CL_AddString(RCOLUMN_OFFSET + s->generic.x + i * 8 * fontscale + s->generic.parent->x + 8, s->generic.y + s->generic.parent->y, fontscale-1, data, consFont);
}

void SpinControl_DoEnter (menulist_s * s) {
	s->curInteger++;
	if (s->itemnames[s->curInteger] == 0)
		s->curInteger = 0;

	if (s->generic.callback)
		s->generic.callback (s);
}

void SpinControl_DoSlide (menulist_s * s, int dir) {
	s->curInteger += dir;

	if (s->curInteger < 0)
		s->curInteger = 0;
	else if (s->itemnames[s->curInteger] == 0)
		s->curInteger--;

	if (s->generic.callback)
		s->generic.callback (s);
}

void SpinControl_Draw (menulist_s * s) {
	char	buffer[100];
	int	fontscale = ui_fontScale->integer;

	if (s->generic.name) {
		Menu_DrawStringR2LDark (s->generic.x + s->generic.parent->x +
			LCOLUMN_OFFSET,
			s->generic.y + s->generic.parent->y,
			s->generic.name);
	}
	if (!strchr (s->itemnames[s->curInteger], '\n')) {
		CL_AddString (RCOLUMN_OFFSET + s->generic.x + s->generic.parent->x,
			s->generic.y + s->generic.parent->y,
			fontscale, 
			s->itemnames[s->curInteger], menuFont);
	}
	else {
		strcpy (buffer, s->itemnames[s->curInteger]);
		*strchr (buffer, '\n') = 0;
		CL_AddString(RCOLUMN_OFFSET + s->generic.x + s->generic.parent->x,
			s->generic.y + s->generic.parent->y,
			fontscale, 
			buffer, menuFont);

		strcpy (buffer, strchr (s->itemnames[s->curInteger], '\n') + 1);
		CL_AddString(RCOLUMN_OFFSET + s->generic.x + s->generic.parent->x,
			s->generic.y + s->generic.parent->y + 10 * ui_fontScale->integer,
			fontscale, 
			buffer, menuFont);
	}

}
