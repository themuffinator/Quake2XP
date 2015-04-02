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

#include "client.h"

image_t *i_conback;
image_t *i_inventory;
image_t *i_net;
image_t *i_pause;
image_t *i_loading;
image_t *i_backtile;
image_t *i_turtle;
extern image_t *r_notexture;
image_t *im_main_menu[5];


void CL_InitImages () {
	int i;

	i_conback = Draw_FindPic ("conback");
	i_inventory = Draw_FindPic ("inventory");
	i_net = Draw_FindPic ("net");
	i_pause = Draw_FindPic ("pause");
	i_loading = Draw_FindPic ("loading");
	i_backtile = Draw_FindPic ("backtile");
	i_turtle = Draw_FindPic ("turtle");

	im_main_menu[0] = Draw_FindPic ("m_main_game");
	im_main_menu[1] = Draw_FindPic ("m_main_multiplayer");
	im_main_menu[2] = Draw_FindPic ("m_main_options");
	im_main_menu[3] = Draw_FindPic ("m_main_video");
	im_main_menu[4] = Draw_FindPic ("m_main_quit");

	for (i = 0; i < 5; i++)
	if (!im_main_menu[i])
		im_main_menu[i] = r_notexture;

	for (i = 0; i < 5; i++) {
		im_main_menu[i]->name[strlen (im_main_menu[i]->name) - 4] = 0;
	}

}
