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

#include "../client/client.h"
#include "../client/qmenu.h"
#include "../ref_gl/r_local.h"

extern cvar_t *vid_ref;

extern void M_ForceMenuOff (void);

int refresh = 0;

int menuSize = 0;

/*
====================================================================

MENU INTERACTION

====================================================================
*/
static menuframework_s	s_opengl_menu;
static menuframework_s *s_current_menu;

static menulist_s		s_mode_list;
static menuslider_s		s_aniso_slider;
static menuslider_s		s_brightness_slider;
static menulist_s  		s_fs_box;

static menulist_s	    s_shadow_box;
static menulist_s	    s_flare_box;
static menulist_s	    s_tc_box;
static menulist_s	    s_refresh_box;
static menulist_s	    s_parallax_box;
static menulist_s	    s_samples_list;

static menulist_s	    s_bloom_box;
static menulist_s	    s_dof_box;

static menulist_s  		s_finish_box;
static menuaction_s		s_apply_action;
static menuaction_s		s_defaults_action;

static	menulist_s			a_pplWorld_list;
static	menulist_s			s_radBlur_box;
static	menulist_s			s_ssao;
static	menulist_s			s_fxaa_box;
static	menulist_s			s_film_grain;
static	menulist_s			s_mb_box;
static menuslider_s			s_ambientLevel_slider;


/////////////////////////////////////////////////////////
//
// MENU GENERIC FUNCTIONS
//
/////////////////////////////////////////////////////////

static float ClampCvar (float min, float max, float value) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}


static void ambientLevelCallback (void *s) {
	float ambient = s_ambientLevel_slider.curvalue / 20;
	Cvar_SetValue ("r_lightmapScale", ambient);
}

static void RadarCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_filmGrain", box->curvalue * 1);
}

static void ShadowsCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_shadows", box->curvalue * 1);
}

static void ParallaxCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_reliefMapping", box->curvalue * 1);
}


static void FlareCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_drawFlares", box->curvalue * 1);
}

static void AnisoCallback (void *s) {
	menuslider_s *slider = (menuslider_s *)s;

	Cvar_SetValue ("r_anisotropic", slider->curvalue * 1);
}

static void BrightnessCallback (void *s) {
	float gamma;
	gamma = s_brightness_slider.curvalue / 20;

	Cvar_SetValue ("r_brightness", gamma);
}


static void BloomCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_bloom", box->curvalue * 1);
}

static void DofCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_dof", box->curvalue * 1);
}

static void RBCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_radialBlur", box->curvalue * 1);
}

static void ssaoCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_ssao", box->curvalue * 1);
}

static void fxaaCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_fxaa", box->curvalue * 1);
}

static void mbCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_motionBlur", box->curvalue * 1);
}

static void ResetDefaults (void *unused) {
	VID_MenuInit ();
}

static void ApplyChanges (void *unused) {

	Cvar_SetValue ("r_anisotropic", s_aniso_slider.curvalue);
	Cvar_SetValue ("r_fullScreen", s_fs_box.curvalue);
	Cvar_SetValue ("r_drawFlares", s_flare_box.curvalue);
	Cvar_SetValue ("r_textureCompression", s_tc_box.curvalue);
	Cvar_SetValue ("r_mode", s_mode_list.curvalue);
	Cvar_SetValue ("r_shadows", s_shadow_box.curvalue);
	Cvar_SetValue ("r_reliefMapping", s_parallax_box.curvalue);
	Cvar_SetValue ("r_bloom", s_bloom_box.curvalue);
	Cvar_SetValue ("r_dof", s_dof_box.curvalue);
	Cvar_SetValue ("r_radialBlur", s_radBlur_box.curvalue);
	Cvar_SetValue ("r_ssao", s_ssao.curvalue);
	Cvar_SetValue ("r_fxaa", s_fxaa_box.curvalue);
	Cvar_SetValue ("r_vsync", s_finish_box.curvalue);
	Cvar_SetValue ("r_filmGrain", s_film_grain.curvalue);
	Cvar_SetValue ("r_motionBlur", s_mb_box.curvalue);

	/*
	** update appropriate stuff if we're running OpenGL and gamma
	** has been modified
	*/
	if (r_brightness->modified)
		vid_ref->modified = qtrue;

	if (r_anisotropic->modified)
		vid_ref->modified = qtrue;

	if (r_shadows->modified)
		vid_ref->modified = qtrue;

	if (r_bloom->modified)
		vid_ref->modified = qtrue;

	if (r_dof->modified)
		vid_ref->modified = qtrue;

	if (r_displayRefresh->modified)
		vid_ref->modified = qtrue;

	if (r_drawFlares->modified)
		vid_ref->modified = qtrue;

	if (r_reliefMapping->modified)
		vid_ref->modified = qtrue;

	if (r_textureCompression->modified)
		vid_ref->modified = qtrue;

	if (r_vsync->modified)
		vid_ref->modified = qtrue;

	if (r_dof->modified)
		vid_ref->modified = qtrue;

	if (r_radialBlur->modified)
		vid_ref->modified = qtrue;

	if (r_ssao->modified)
		vid_ref->modified = qtrue;

	if (r_fxaa->modified)
		vid_ref->modified = qtrue;

	if (r_lightmapScale->modified)
		vid_ref->modified = qtrue;

	if (r_motionBlur->modified)
		vid_ref->modified = qtrue;

	M_ForceMenuOff ();

}

static void CancelChanges (void *unused) {
	extern void M_PopMenu (void);

	M_PopMenu ();
}


static void pplWorldCallBack (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_skipStaticLights", box->curvalue * 1);
}

static void vSyncCallBack (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_vsync", box->curvalue * 1);
}

/*
** VID_MenuInit
*/
void VID_MenuInit (void) {
	static char *resolutions[] = {
#ifdef _WIN32
		"[Desktop]",
#endif
		"[1024 768][4:3]",
		"[1152 864][4:3]",
		"[1280 1024][5:4]",
		"[1600 1200][4:3]",
		"[2048 1536][4:3]",

		"[1280 720][720p HDTV]",
		"[1280 800][16:10]",
		"[1366 768][16:9 Plasma and LCD TV]",
		"[1440 900][16:10]",
		"[1600 900][16:9 LCD]",
		"[1680 1050][16:10]",
		"[1920 1080][1080p full HDTV]",
		"[1920 1200][16:10]",
		"[2560 1600][16:10]",
		"[Custom]", 0 };

	static char	*yesno_names[] = { "no", "yes", 0 };
	static char	*adaptive_vc[] = { "no", "default", "adaptive", 0 };

#ifndef _WIN32
	static char	*samples[] = { "[off]", "[2x]", "[4x]", 0 }; // sdl bug work only 2 and 4 samples per pixel
#else
	static char	*samples[] = { "[off]", "[2x]", "[4x]", "[8x]", "[16x]", 0 };
#endif

	static char	*radar[] = { "off", "map only", "map and entities", "move detector", 0 };

	if (!r_mode)
		r_mode = Cvar_Get ("r_mode", "0", 0);

	if (!r_anisotropic)
		r_anisotropic = Cvar_Get ("r_anisotropic", "1", CVAR_ARCHIVE);

	if (!r_textureCompression)
		r_textureCompression = Cvar_Get ("r_textureCompression", "0", CVAR_ARCHIVE);

	if (!r_drawFlares)
		r_drawFlares = Cvar_Get ("r_drawFlares", "0", CVAR_ARCHIVE);


	if (!r_bloom)
		r_bloom = Cvar_Get ("r_bloom", "0", CVAR_ARCHIVE);

	if (!r_shadows)
		r_shadows = Cvar_Get ("r_shadows", "0", CVAR_ARCHIVE);

	if (!r_reliefMapping)
		r_reliefMapping = Cvar_Get ("r_reliefMapping", "0", CVAR_ARCHIVE);

	if (r_reliefMapping->value > 1)
		r_reliefMapping = Cvar_Get("r_reliefMapping", "1", CVAR_ARCHIVE);

	if (!r_dof)
		r_dof = Cvar_Get ("r_dof", "0", CVAR_ARCHIVE);

	if (!r_radialBlur->value)
		r_radialBlur = Cvar_Get ("r_radialBlur", "0", CVAR_ARCHIVE);

	if (!r_skipStaticLights)
		r_skipStaticLights = Cvar_Get ("r_skipStaticLights", "0", CVAR_ARCHIVE);

	if (!r_vsync)
		r_vsync = Cvar_Get ("r_vsync", "0", CVAR_ARCHIVE);

	if (!cl_fontScale)
		cl_fontScale = Cvar_Get ("cl_fontScale", "1", CVAR_ARCHIVE);

	if (!r_ssao->value)
		r_ssao = Cvar_Get ("r_ssao", 0, CVAR_ARCHIVE);

	if (!r_fxaa->value)
		r_fxaa = Cvar_Get ("r_fxaa", 0, CVAR_ARCHIVE);

	if (!r_lightmapScale->value)
		r_lightmapScale = Cvar_Get ("r_lightmapScale", "0", CVAR_ARCHIVE);

	if (!r_motionBlur->value)
		r_motionBlur = Cvar_Get ("r_motionBlur", "0", CVAR_ARCHIVE);

	s_opengl_menu.x = viddef.width * 0.50;
	s_opengl_menu.nitems = 0;

	s_mode_list.generic.type = MTYPE_SPINCONTROL;
	s_mode_list.generic.name = "Screen Resolution";
	s_mode_list.generic.x = 0;
	s_mode_list.generic.y = 10 * cl_fontScale->value;
	s_mode_list.itemnames = resolutions;
	s_mode_list.curvalue = r_mode->value;
	s_mode_list.generic.statusbar = "Requires Restart Video Sub-System";

	s_fs_box.generic.type = MTYPE_SPINCONTROL;
	s_fs_box.generic.x = 0;
	s_fs_box.generic.y = 20 * cl_fontScale->value;
	s_fs_box.generic.name = "Fullscreen";
	s_fs_box.itemnames = yesno_names;
	s_fs_box.curvalue = r_fullScreen->value;
	s_fs_box.generic.statusbar = "Requires Restart Video Sub-System";


	s_brightness_slider.generic.type = MTYPE_SLIDER;
	s_brightness_slider.generic.x = 0;
	s_brightness_slider.generic.y = 30 * cl_fontScale->value;
	s_brightness_slider.generic.name = "Brightness";
	s_brightness_slider.generic.callback = BrightnessCallback;
	s_brightness_slider.minvalue = 20;
	s_brightness_slider.maxvalue = 40;
	s_brightness_slider.curvalue = r_brightness->value * 20;

	// -----------------------------------------------------------------------

	s_aniso_slider.generic.type = MTYPE_SLIDER;
	s_aniso_slider.generic.x = 0;
	s_aniso_slider.generic.y = 50 * cl_fontScale->value;
	s_aniso_slider.generic.name = "Texture Anisotropy Level";
	s_aniso_slider.minvalue = 1;
	s_aniso_slider.maxvalue = 16;
	s_aniso_slider.curvalue = r_anisotropic->value;
	s_aniso_slider.generic.callback = AnisoCallback;

	s_tc_box.generic.type = MTYPE_SPINCONTROL;
	s_tc_box.generic.x = 0;
	s_tc_box.generic.y = 60 * cl_fontScale->value;
	s_tc_box.generic.name = "Texture Compression";
	s_tc_box.itemnames = yesno_names;
	s_tc_box.curvalue = r_textureCompression->value;
	s_tc_box.generic.statusbar = "Requires Restart Video Sub-System";

	// -----------------------------------------------------------------------

	a_pplWorld_list.generic.type = MTYPE_SPINCONTROL;
	a_pplWorld_list.generic.name = "Skip Static Lights";
	a_pplWorld_list.generic.x = 0;
	a_pplWorld_list.generic.y = 80 * cl_fontScale->value;
	a_pplWorld_list.itemnames = yesno_names;
	a_pplWorld_list.curvalue = r_skipStaticLights->value;
	a_pplWorld_list.generic.callback = pplWorldCallBack;

	s_shadow_box.generic.type = MTYPE_SPINCONTROL;
	s_shadow_box.generic.x = 0;
	s_shadow_box.generic.y = 90 * cl_fontScale->value;
	s_shadow_box.generic.name = "Shadows";
	s_shadow_box.itemnames = yesno_names;
	s_shadow_box.curvalue = r_shadows->value;
	s_shadow_box.generic.callback = ShadowsCallback;

	s_parallax_box.generic.type = MTYPE_SPINCONTROL;
	s_parallax_box.generic.x = 0;
	s_parallax_box.generic.y = 100 * cl_fontScale->value;
	s_parallax_box.generic.name = "Relief Mapping";
	s_parallax_box.itemnames = yesno_names;
	s_parallax_box.curvalue = r_reliefMapping->value;
	s_parallax_box.generic.callback = ParallaxCallback;
	s_parallax_box.generic.statusbar = "Relief Parallax Mapping";

	s_ambientLevel_slider.generic.type = MTYPE_SLIDER;
	s_ambientLevel_slider.generic.x = 0;
	s_ambientLevel_slider.generic.y = 110 * cl_fontScale->value;
	s_ambientLevel_slider.generic.name = "Lightmap Scale";
	s_ambientLevel_slider.generic.callback = ambientLevelCallback;
	s_ambientLevel_slider.minvalue = 0;
	s_ambientLevel_slider.maxvalue = 20;
	s_ambientLevel_slider.curvalue = r_lightmapScale->value * 20;
	s_ambientLevel_slider.generic.statusbar = "Precomputed Lighting Level";

	s_flare_box.generic.type = MTYPE_SPINCONTROL;
	s_flare_box.generic.x = 0;
	s_flare_box.generic.y = 130 * cl_fontScale->value;
	s_flare_box.generic.name = "Flares";
	s_flare_box.itemnames = yesno_names;
	s_flare_box.curvalue = r_drawFlares->value;
	s_flare_box.generic.callback = FlareCallback;

	s_bloom_box.generic.type = MTYPE_SPINCONTROL;
	s_bloom_box.generic.x = 0;
	s_bloom_box.generic.y = 140 * cl_fontScale->value;
	s_bloom_box.generic.name = "Light Blooms";
	s_bloom_box.itemnames = yesno_names;
	s_bloom_box.curvalue = r_bloom->value;
	s_bloom_box.generic.callback = BloomCallback;

	s_dof_box.generic.type = MTYPE_SPINCONTROL;
	s_dof_box.generic.x = 0;
	s_dof_box.generic.y = 150 * cl_fontScale->value;
	s_dof_box.generic.name = "Depth of Field";
	s_dof_box.itemnames = yesno_names;
	s_dof_box.curvalue = r_dof->value;
	s_dof_box.generic.callback = DofCallback;

	s_radBlur_box.generic.type = MTYPE_SPINCONTROL;
	s_radBlur_box.generic.x = 0;
	s_radBlur_box.generic.y = 160 * cl_fontScale->value;
	s_radBlur_box.generic.name = "Radial Blur";
	s_radBlur_box.itemnames = yesno_names;
	s_radBlur_box.curvalue = r_radialBlur->value;
	s_radBlur_box.generic.callback = RBCallback;

	s_mb_box.generic.type = MTYPE_SPINCONTROL;
	s_mb_box.generic.x = 0;
	s_mb_box.generic.y = 170 * cl_fontScale->value;
	s_mb_box.generic.name = "Motion Blur";
	s_mb_box.itemnames = yesno_names;
	s_mb_box.curvalue = r_motionBlur->value;
	s_mb_box.generic.callback = mbCallback;

	s_ssao.generic.type = MTYPE_SPINCONTROL;
	s_ssao.generic.x = 0;
	s_ssao.generic.y = 180 * cl_fontScale->value;
	s_ssao.generic.name = "SSAO";
	s_ssao.itemnames = yesno_names;
	s_ssao.curvalue = r_ssao->value;
	s_ssao.generic.callback = ssaoCallback;
	s_ssao.generic.statusbar = "Screen Space Ambient Occlusion";

	s_fxaa_box.generic.type = MTYPE_SPINCONTROL;
	s_fxaa_box.generic.x = 0;
	s_fxaa_box.generic.y = 190 * cl_fontScale->value;
	s_fxaa_box.generic.name = "FXAA";
	s_fxaa_box.itemnames = yesno_names;
	s_fxaa_box.curvalue = r_fxaa->value;
	s_fxaa_box.generic.callback = fxaaCallback;
	s_fxaa_box.generic.statusbar = "Post-Process Anti-Aliasing";

	s_film_grain.generic.type = MTYPE_SPINCONTROL;
	s_film_grain.generic.x = 0;
	s_film_grain.generic.y = 200 * cl_fontScale->value;
	s_film_grain.generic.name = "Film Grain";
	s_film_grain.itemnames = yesno_names;
	s_film_grain.curvalue = r_filmGrain->value;
	s_film_grain.generic.callback = RadarCallback;

	s_finish_box.generic.type = MTYPE_SPINCONTROL;
	s_finish_box.generic.x = 0;
	s_finish_box.generic.y = 210 * cl_fontScale->value;
	s_finish_box.generic.name = "Vertical Sync";
	s_finish_box.generic.callback = vSyncCallBack;
	s_finish_box.curvalue = r_vsync->value;
	if (gl_state.wgl_swap_control_tear) {
		if (r_vsync->value >= 3)
			Cvar_SetValue ("r_vsync", 2);
		s_finish_box.itemnames = adaptive_vc;
		s_finish_box.generic.statusbar = "Off - On - Adaptive";
	}
	else {
		if (r_vsync->value >= 2)
			Cvar_SetValue ("r_vsync", 1);
		s_finish_box.itemnames = yesno_names;
		s_finish_box.generic.statusbar = "Off - On";
	}

	s_defaults_action.generic.type = MTYPE_ACTION;
	s_defaults_action.generic.name = "reset to defaults";
	s_defaults_action.generic.x = 0;
	s_defaults_action.generic.y = 230 * cl_fontScale->value;
	s_defaults_action.generic.callback = ResetDefaults;

	s_apply_action.generic.type = MTYPE_ACTION;
	s_apply_action.generic.name = "Apply Changes";
	s_apply_action.generic.x = 0;
	s_apply_action.generic.y = 240 * cl_fontScale->value;
	s_apply_action.generic.callback = ApplyChanges;

	menuSize = 250;

	Menu_AddItem (&s_opengl_menu, (void *)&s_mode_list);
	Menu_AddItem (&s_opengl_menu, (void *)&s_fs_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_brightness_slider);

	Menu_AddItem (&s_opengl_menu, (void *)&s_aniso_slider);

	Menu_AddItem (&s_opengl_menu, (void *)&s_tc_box);

	Menu_AddItem (&s_opengl_menu, (void *)&a_pplWorld_list);
	Menu_AddItem (&s_opengl_menu, (void *)&s_shadow_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_parallax_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_ambientLevel_slider);
	Menu_AddItem (&s_opengl_menu, (void *)&s_flare_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_bloom_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_dof_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_radBlur_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_mb_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_ssao);
	Menu_AddItem (&s_opengl_menu, (void *)&s_fxaa_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_film_grain);
	Menu_AddItem (&s_opengl_menu, (void *)&s_finish_box);

	Menu_AddItem (&s_opengl_menu, (void *)&s_defaults_action);
	Menu_AddItem (&s_opengl_menu, (void *)&s_apply_action);

	Menu_Center (&s_opengl_menu);
	s_opengl_menu.x -= 8;

}

/*
================
VID_MenuDraw
================
*/
void VID_MenuDraw (void) {
	int w, h;

	s_current_menu = &s_opengl_menu;

	menuSize = 190 * cl_fontScale->value;

	// draw the banner
	Draw_GetPicSize (&w, &h, "m_banner_video");
	Draw_PicScaled ((int)(viddef.width *0.5 - (w *0.5)), (int)((viddef.height *0.5 - menuSize)), cl_fontScale->value, cl_fontScale->value, "m_banner_video");
	Draw_PicBumpScaled((int)(viddef.width *0.5 - (w *0.5)), (int)((viddef.height *0.5 - menuSize)), cl_fontScale->value, cl_fontScale->value, "m_banner_video", "m_banner_video_bump");

	// move cursor to a reasonable starting position
	Menu_AdjustCursor (s_current_menu, 1);

	// draw menu
	Menu_Draw (s_current_menu);
}

/*
================
VID_MenuKey
================
*/
int VID_MenuKey (int key) {
	menuframework_s *m = s_current_menu;

	switch (key) {
		case K_MOUSE2:
		case K_ESCAPE:
			CancelChanges (NULL);
			return 0;

		case K_MWHEELUP:
		case K_KP_UPARROW:
		case K_UPARROW:
			m->cursor--;
			Menu_AdjustCursor (m, -1);
			break;

		case K_MWHEELDOWN:
		case K_KP_DOWNARROW:
		case K_DOWNARROW:
			m->cursor++;
			Menu_AdjustCursor (m, 1);
			break;
		case K_KP_LEFTARROW:
		case K_LEFTARROW:
			Menu_SlideItem (m, -1);
			break;
		case K_KP_RIGHTARROW:
		case K_RIGHTARROW:
			Menu_SlideItem (m, 1);
			break;
		case K_KP_ENTER:
		case K_ENTER:
			if (!Menu_SelectItem (m))
				ApplyChanges (NULL);
			break;
	}

	return menu_in_sound;
}


