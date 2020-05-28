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

#include "client.h"

extern image_t *r_notexture;

void CL_InitImages () {

	i_conback = Draw_FindPic ("conback");
	i_inventory = Draw_FindPic ("inventory");
	i_net = Draw_FindPic ("net");
	i_pause = Draw_FindPic ("pause");
	i_loading = Draw_FindPic ("loading");

	i_backtile = Draw_FindPic ("backtile");
	i_turtle = Draw_FindPic ("turtle");
	i_nosaveshot = Draw_FindPic("nosaveshot");

	i_menuBackground	= Draw_FindPic("menuback");
	i_main_plaque[0]	= Draw_FindPic("m_main_plaque");
	i_main_plaque[1]	= Draw_FindPic("m_main_plaque_bump");
	i_main_logo[0]		= Draw_FindPic("m_main_logo");
	i_main_logo[1]		= Draw_FindPic("m_main_logo_bump");
	i_quit[0]			= Draw_FindPic("quit");
	i_quit[1]			= Draw_FindPic("quit_bump");

	i_banner_multiplayer[0] = Draw_FindPic("m_banner_multiplayer");
	i_banner_game[0]		= Draw_FindPic("m_banner_game");
	i_banner_load_game[0]	= Draw_FindPic("m_banner_load_game");
	i_banner_save_game[0]	= Draw_FindPic("m_banner_save_game");
	i_banner_join_server[0] = Draw_FindPic("m_banner_join_server");
	i_banner_addressbook[0]	= Draw_FindPic("m_banner_addressbook");
	i_banner_options[0]		= Draw_FindPic("m_banner_options");
	i_banner_video[0]		= Draw_FindPic("m_banner_video");

	i_banner_multiplayer[1] = Draw_FindPic("m_banner_multiplayer_bump");
	i_banner_game[1]		= Draw_FindPic("m_banner_game_bump");
	i_banner_load_game[1]	= Draw_FindPic("m_banner_load_game_bump");
	i_banner_save_game[1]	= Draw_FindPic("m_banner_save_game_bump");
	i_banner_join_server[1] = Draw_FindPic("m_banner_join_server_bump");
	i_banner_addressbook[1]	= Draw_FindPic("m_banner_addressbook_bump");
	i_banner_options[1]		= Draw_FindPic("m_banner_options_bump");
	i_banner_video[1]		= Draw_FindPic("m_banner_video_bump");

	char* names[] = {
	"m_main_game",
	"m_main_multiplayer",
	"m_main_options",
	"m_main_video",
	"m_main_quit"
	};

	char* namesBump[] = {
		"m_main_game_bump",
		"m_main_multiplayer_bump",
		"m_main_options_bump",
		"m_main_video_bump",
		"m_main_quit_bump"
	};
	
	char* names_sel[] = {
	"m_main_game_sel",
	"m_main_multiplayer_sel",
	"m_main_options_sel",
	"m_main_video_sel",
	"m_main_quit_sel"
	};

	char* namesBump_sel[] = {
		"m_main_game_bump_sel",
		"m_main_multiplayer_bump_sel",
		"m_main_options_bump_sel",
		"m_main_video_bump_sel",
		"m_main_quit_bump_sel"
	};

	for (int i = 0; i < 5; i++) {
		i_main_menu[i] = Draw_FindPic(names[i]);
		i_main_menu_bump[i] = Draw_FindPic(namesBump[i]);

		i_main_menu_sel[i] = Draw_FindPic(names_sel[i]);
		i_main_menu_bump_sel[i] = Draw_FindPic(namesBump_sel[i]);
	}
}

void CL_CacheMenuModels() {

	menumodel.menuQuad = R_RegisterModel("models/items/quaddama/tris.md2");
	menumodel.menuLogo = R_RegisterModel("models/menu/idlogo.md3");
	
}