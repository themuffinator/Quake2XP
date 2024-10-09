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

#include "../client/client.h"
#include "../client/qmenu.h"
#include "../renderer/r_local.h"

extern bool drawIDlogo;
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
static menuframework_s	s_opengl2_menu;
static menuframework_s *s_current_menu;

static menulist_s		s_mode_list;
static menulist_s		s_aniso_list;

static menuslider_s		s_brightness_slider;
static menuslider_s		s_contrast_slider;
static menuslider_s		s_saturation_slider;
static menuslider_s		s_gamma_slider;
static menuslider_s		s_vibrance_slider;
static menuslider_s		s_fixfov_slider;
//static menulist_s		s_lut_list;
static menuslider_s		s_hdrNits_slider;

static menuslider_s		s_flareIntens_slider;


static menulist_s  		s_fs_box;

static menuslider_s	    s_reliefScale_slider;
static menulist_s	    s_flare_box;
static menulist_s	    s_refresh_box;
static menulist_s	    s_parallax_box;
static menulist_s	    s_parallax_shadow;

static menulist_s	    s_samples_list;

static menulist_s	    s_bloom_box;
static menulist_s	    s_dof_box;

static menulist_s  		s_finish_box;
static menuaction_s		s_apply_action;
static menuaction_s		s_defaults_action;

static	menulist_s		s_autoBump_list;
static	menulist_s		s_radBlur_box;
static	menulist_s		s_ssao;
static	menulist_s		s_fxaa_box;
static	menulist_s		s_film_grain;
static	menulist_s		s_mb_box;
static	menuslider_s	s_ambientLevel_slider;

static	menuaction_s	s_menuAction_color;

static	menufield_s		s_menuColorTemp;

/////////////////////////////////////////////////////////
//
// MENU GENERIC FUNCTIONS
//
/////////////////////////////////////////////////////////

static void ambientLevelCallback (void *s) {
	float ambient = s_ambientLevel_slider.curValue / 20;
	Cvar_SetValue ("r_lightmapScale", ambient);
}

static void filmCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_filmicFx", box->curInteger * 1);
}

static void ParallaxCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_parallaxMapping", box->curInteger * 1);
}

static void reliefScaleCallback(void *s) {
	menuslider_s *slider = (menuslider_s *)s;
	Cvar_SetValue("r_parallaxScale", slider->curValue * 1);
}

static void reliefShadowCallback(void *s) {
	menulist_s *box = (menulist_s *)s;
	Cvar_SetValue("r_selfShadowingParallax", box->curInteger * 1);
}

static void FlareCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_hdrLensFlares", box->curInteger * 1);
}

static void AnisoCallback (void *s) {
	menulist_s *box  = (menulist_s*)s;

	Cvar_SetValue ("r_textureAnisotropy", box->curInteger * 1);
}

static void BrightnessCallback (void *s) {
	float brt;
	brt = s_brightness_slider.curValue / 10;

	Cvar_SetValue ("r_brightness", brt);
}

static void hdrNitsCallback(void *s) {
	float nits;
	nits = s_hdrNits_slider.curValue / 1;

	Cvar_SetValue("r_hdrUiNits", nits);
}

static void ContrastCallback(void *s) {
	float contr;
	contr = s_contrast_slider.curValue / 10;

	Cvar_SetValue("r_contrast", contr);
}

static void SaturationCallback(void *s) {
	float sat;
	sat = s_saturation_slider.curValue / 10;

	Cvar_SetValue("r_saturation", sat);
}

static void GammaCallback(void *s) {
	float gm;
	gm = s_gamma_slider.curValue / 10;

	Cvar_SetValue("r_gamma", gm);
}

static void VibranceCallback(void *s) {
	float vb;
	vb = s_vibrance_slider.curValue / 10;

	Cvar_SetValue("r_colorVibrance", vb);
}

static void FixFovCallback(void *s) {
	float vb;
	vb = s_fixfov_slider.curValue / 10;

	Cvar_SetValue("r_fixFovStrength", vb);
}


static void BloomCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_hdrBloom", box->curInteger * 1);
}

static void DofCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_dof", box->curInteger * 1);
}

static void RBCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_radialBlur", box->curInteger * 1);
}

static void ssaoCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_ssao", box->curInteger * 1);
}

static void fxaaCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_fxaa", box->curInteger * 1);
}

static void mbCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_motionBlur", box->curInteger * 1);
}

static void flareLevelCallback(void *s) {
	float intens = s_flareIntens_slider.curValue / 10;
	Cvar_SetValue("r_hdrLensFlaresIntens", intens);
}

static void ResetDefaults (void *unused) {
	VID_MenuInit ();
}

static void ApplyChanges (void *unused) {

	Cvar_SetValue ("r_textureAnisotropy", s_aniso_list.curInteger);
	Cvar_SetValue ("r_fullScreen", s_fs_box.curInteger);
	Cvar_SetValue ("r_drawFlares", s_flare_box.curInteger);
	Cvar_SetValue ("r_mode", s_mode_list.curInteger);
	Cvar_SetValue ("r_parallaxScale", s_reliefScale_slider.curValue);
	Cvar_SetValue ("r_parallaxMapping", s_parallax_box.curInteger);

	Cvar_SetValue("r_selfShadowingParallax", s_parallax_shadow.curInteger);

	Cvar_SetValue ("r_hdrGlare", s_bloom_box.curInteger);
	Cvar_SetValue ("r_dof", s_dof_box.curInteger);
	Cvar_SetValue ("r_radialBlur", s_radBlur_box.curInteger);
	Cvar_SetValue ("r_ssao", s_ssao.curInteger);
	Cvar_SetValue ("r_fxaa", s_fxaa_box.curInteger);
	Cvar_SetValue ("r_vsync", s_finish_box.curInteger);
	Cvar_SetValue ("r_filmicFx", s_film_grain.curInteger);
	Cvar_SetValue ("r_motionBlur", s_mb_box.curInteger);
	Cvar_SetValue("r_fixFovStrength", s_fixfov_slider.curValue);
	Cvar_SetValue("r_hdr_uiNits", s_hdrNits_slider.curValue);

	switch (s_aniso_list.curInteger)
	{
	case 0:
		Cvar_SetValue("r_textureAnisotropy", 1);
		break;
	case 1:
		Cvar_SetValue("r_textureAnisotropy", 2);
		break;
	case 2:
		Cvar_SetValue("r_textureAnisotropy", 4);
		break;
	case 3:
		Cvar_SetValue("r_textureAnisotropy", 8);
		break;
	case 4:
		Cvar_SetValue("r_textureAnisotropy", 16);
		break;
	
	default:
		Cvar_SetValue("r_textureAnisotropy", 1);
		break;
	}


	/*
	** update appropriate stuff if we're running OpenGL and gamma
	** has been modified
	*/
	
	if (r_selfShadowingParallax->modified)
		vid_ref->modified = true;

	if (r_brightness->modified)
		vid_ref->modified = true;
	
	if (r_contrast->modified)
		vid_ref->modified = true;
	
	if (r_saturation->modified)
		vid_ref->modified = true;

	if (r_gamma->modified)
		vid_ref->modified = true;

	if (r_textureAnisotropy->modified)
		vid_ref->modified = true;

	if (r_parallaxScale->modified)
		vid_ref->modified = true;

	if (r_hdrBloom->modified)
		vid_ref->modified = true;

	if (r_dof->modified)
		vid_ref->modified = true;

	if (r_displayRefresh->modified)
		vid_ref->modified = true;

	if (r_hdrLensFlares->modified)
		vid_ref->modified = true;

	if (r_parallaxMapping->modified)
		vid_ref->modified = true;

	if (r_vsync->modified)
		vid_ref->modified = true;

	if (r_dof->modified)
		vid_ref->modified = true;

	if (r_radialBlur->modified)
		vid_ref->modified = true;

	if (r_ssao->modified)
		vid_ref->modified = true;

	if (r_fxaa->modified)
		vid_ref->modified = true;

	if (r_lightmapScale->modified)
		vid_ref->modified = true;

	if (r_motionBlur->modified)
		vid_ref->modified = true;

	if (r_hdrLensFlaresIntens->modified)
		vid_ref->modified = true;

	if (r_hdrUiNits->modified)
		vid_ref->modified = true;

	if (r_fixFovStrength->modified)
		vid_ref->modified = true;
	
	M_ForceMenuOff ();

}

static void CancelChanges (void *unused) {
	extern void M_PopMenu (void);

	M_PopMenu ();
}


static void autoBumpCallBack (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_imageAutoBump", box->curInteger * 1);
}

static void vSyncCallBack (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_vsync", box->curInteger * 1);
}

static void lutCallBack(void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue("r_lutId", box->curInteger * 1);
}

void ColorTempFunc(void *unused)
{
	Cvar_Set("r_colorTempK", s_menuColorTemp.buffer);
	r_colorTempK->integer = ClampCvarInteger(999, 40000, r_colorTempK->integer);
}

void M_ColorInit() {

	if (!r_gamma)
		r_gamma = Cvar_Get("r_gamma", "1.5", CVAR_ARCHIVE);
	
	if (!r_colorTempK)
		r_colorTempK = Cvar_Get("r_colorTempK", "6500", CVAR_ARCHIVE);

	if (!r_brightness)
		r_brightness = Cvar_Get("r_brightness", "1", CVAR_ARCHIVE);

	if (!r_contrast)
		r_contrast = Cvar_Get("r_contrast", "1", CVAR_ARCHIVE);

	if (!r_saturation)
		r_saturation = Cvar_Get("r_saturation", "1", CVAR_ARCHIVE);

	if (!r_hdrLensFlaresIntens)
		r_hdrLensFlaresIntens = Cvar_Get("r_hdrLensFlaresIntens", "0.2", CVAR_ARCHIVE);

	if (!r_fixFovStrength)
		r_fixFovStrength = Cvar_Get("r_fixFovStrength", "0.0", CVAR_ARCHIVE);
	if(!r_hdrUiNits)
		r_hdrUiNits = Cvar_Get("r_hdr_uiNits", "100.0", CVAR_ARCHIVE);

	r_hdrUiNits->value = ClampCvar(100.0, gl_config.hdrMaxLuminance, r_hdrUiNits->value);
	r_gamma->value = ClampCvar(1.5, 2.2, r_gamma->value);
	r_brightness->value = ClampCvar(0.1, 2.0, r_brightness->value);
	r_contrast->value = ClampCvar(0.1, 2.0, r_contrast->value);
	r_saturation->value = ClampCvar(0.1, 2.0, r_saturation->value);
	r_colorVibrance->value = ClampCvar(-1.0, 1.0, r_colorVibrance->value);

	r_hdrLensFlaresIntens->value = ClampCvar(0.1, 1.0, r_hdrLensFlaresIntens->value);
	
	r_fixFovStrength->value = ClampCvar(0.0, 1.0, r_fixFovStrength->value);
	r_colorTempK->integer = ClampCvarInteger(1000, 40000, r_colorTempK->integer);

/*	static char* lut_table[8] = {0};
	
	for (int i = 0; i < lutCount; i++) {
		lut_table[i] = r_3dLut[i]->lutName;
	}
*/
	drawIDlogo = false;

	s_opengl2_menu.x = viddef.width * 0.50;
	s_opengl2_menu.nitems = 0;

	s_gamma_slider.generic.type = MTYPE_SLIDER;
	s_gamma_slider.generic.x = 0;
	s_gamma_slider.generic.y = 10 * ui_fontScale->value;
	s_gamma_slider.generic.name = "Gamma";
	s_gamma_slider.generic.callback = GammaCallback;
	s_gamma_slider.minValue = 15;
	s_gamma_slider.maxValue = 22;
	s_gamma_slider.curValue = r_gamma->value * 10;
	s_gamma_slider.divRange = 10;
	s_gamma_slider.generic.statusbar = "Screen Gamma";

	s_brightness_slider.generic.type = MTYPE_SLIDER;
	s_brightness_slider.generic.x = 0;
	s_brightness_slider.generic.y = 20 * ui_fontScale->value;
	s_brightness_slider.generic.name = "Brightness";
	s_brightness_slider.generic.callback = BrightnessCallback;
	s_brightness_slider.minValue = 1;
	s_brightness_slider.maxValue = 20;
	s_brightness_slider.curValue = r_brightness->value * 10;
	s_brightness_slider.divRange = 10;
	s_brightness_slider.generic.statusbar = "Screen Brightness";

	s_contrast_slider.generic.type = MTYPE_SLIDER;
	s_contrast_slider.generic.x = 0;
	s_contrast_slider.generic.y = 30 * ui_fontScale->value;
	s_contrast_slider.generic.name = "Contrast";
	s_contrast_slider.generic.callback = ContrastCallback;
	s_contrast_slider.minValue = 1;
	s_contrast_slider.maxValue = 20;
	s_contrast_slider.curValue = r_contrast->value * 10;
	s_contrast_slider.divRange = 10;
	s_contrast_slider.generic.statusbar = "Screen Contrast";

	s_saturation_slider.generic.type = MTYPE_SLIDER;
	s_saturation_slider.generic.x = 0;
	s_saturation_slider.generic.y = 40 * ui_fontScale->value;
	s_saturation_slider.generic.name = "Saturation";
	s_saturation_slider.generic.callback = SaturationCallback;
	s_saturation_slider.minValue = 1;
	s_saturation_slider.maxValue = 20;
	s_saturation_slider.curValue = r_saturation->value * 10;
	s_saturation_slider.divRange = 10;
	s_saturation_slider.generic.statusbar = "Screen Saturation";

	s_vibrance_slider.generic.type = MTYPE_SLIDER;
	s_vibrance_slider.generic.x = 0;
	s_vibrance_slider.generic.y = 50 * ui_fontScale->value;
	s_vibrance_slider.generic.name = "Vibrance";
	s_vibrance_slider.generic.callback = VibranceCallback;
	s_vibrance_slider.minValue = -10;
	s_vibrance_slider.maxValue = 10;
	s_vibrance_slider.curValue = r_colorVibrance->value * 10;
	s_vibrance_slider.divRange = 10;
	s_vibrance_slider.generic.statusbar = "Color Vibrance";

	s_hdrNits_slider.generic.type = MTYPE_SLIDER;
	s_hdrNits_slider.generic.x = 0;
	s_hdrNits_slider.generic.y = 70 * ui_fontScale->value;
	s_hdrNits_slider.generic.name = "HDR UI Brightness";
	s_hdrNits_slider.generic.callback = hdrNitsCallback;
	s_hdrNits_slider.minValue = 100;
	s_hdrNits_slider.maxValue = gl_config.hdrMaxLuminance;
	s_hdrNits_slider.curValue = r_hdrUiNits->value * 1;
	s_hdrNits_slider.divRange = 1;
	s_hdrNits_slider.name = "Nits";
	s_hdrNits_slider.generic.statusbar = "UI Brightness in HDR Mode";


	s_flareIntens_slider.generic.type = MTYPE_SLIDER;
	s_flareIntens_slider.generic.x = 0;
	s_flareIntens_slider.generic.y = 90 * ui_fontScale->value;
	s_flareIntens_slider.generic.name = "Lens Flares Intensity";
	s_flareIntens_slider.generic.callback = flareLevelCallback;
	s_flareIntens_slider.minValue = 1;
	s_flareIntens_slider.maxValue = 10;
	s_flareIntens_slider.curValue = r_hdrLensFlaresIntens->value * 10;
	s_flareIntens_slider.divRange = 10;
	s_flareIntens_slider.generic.statusbar = "Adjust Lens Flares Intensity In LDR or HDR Modes";

	s_fixfov_slider.generic.type = MTYPE_SLIDER;
	s_fixfov_slider.generic.x = 0;
	s_fixfov_slider.generic.y = 110 * ui_fontScale->value;
	s_fixfov_slider.generic.name = "Hi-FOV Corection";
	s_fixfov_slider.generic.callback = FixFovCallback;
	s_fixfov_slider.minValue = 0;
	s_fixfov_slider.maxValue = 10;
	s_fixfov_slider.curValue = r_fixFovStrength->value * 10;
	s_fixfov_slider.divRange = 10;
	s_fixfov_slider.generic.statusbar = "Reducing Field Of View Distortion";

/*	s_lut_list.generic.type = MTYPE_SPINCONTROL;
	s_lut_list.generic.name = "Color Grading";
	s_lut_list.generic.x = 0;
	s_lut_list.generic.y = 130 * ui_fontScale->value;
	s_lut_list.itemnames = lut_table;
	s_lut_list.curValue = r_lutId->integer;
	s_lut_list.generic.callback = lutCallBack;
	s_lut_list.generic.statusbar = "Add Color Filters";
	*/
	s_menuColorTemp.generic.type = MTYPE_FIELD;
	s_menuColorTemp.generic.name = "Color Temperature";
	s_menuColorTemp.generic.flags = QMF_NUMBERSONLY;
	s_menuColorTemp.generic.x = 0;
	s_menuColorTemp.generic.y = 130 * ui_fontScale->value;
	s_menuColorTemp.generic.statusbar = "Color Temperature in Kelvins 1000 - 40000";
	s_menuColorTemp.length = 9;
	s_menuColorTemp.visible_length = 9;
	s_menuColorTemp.generic.callback = ColorTempFunc;
	sprintf(s_menuColorTemp.buffer, "%d", r_colorTempK->integer);
	s_menuColorTemp.cursor = strlen(s_menuColorTemp.buffer);

	menuSize = 130;

	Menu_AddItem(&s_opengl2_menu, (void *)&s_gamma_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_brightness_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_contrast_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_saturation_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_vibrance_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_hdrNits_slider);

	Menu_AddItem(&s_opengl2_menu, (void *)&s_flareIntens_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_fixfov_slider);
//	Menu_AddItem(&s_opengl2_menu, (void *)&s_lut_list);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_menuColorTemp);


	Menu_Center(&s_opengl2_menu);
	s_opengl2_menu.x -= 8;
}

int Default_MenuKey(menuframework_s * m, int key);
void M_PushMenu(void(*draw) (void), int(*key) (int k));

void Draw_VideoBanner() {
	int w, h;

	w = i_banner_video[0]->width;
	h = i_banner_video[0]->height;

	if (ui_fontScale->value == 2) {
		Draw_ScaledPic((int)(viddef.width * 0.5 - (w * 0.5)), (int)(viddef.height * 0.5 - menuSize), ui_fontScale->value, ui_fontScale->value, PF_LIGHT, i_banner_video[0], i_banner_video[1]);
	}
	else if (ui_fontScale->value == 3) {
		Draw_ScaledPic((int)(viddef.width * 0.5 - (w * 0.75)), (int)(viddef.height * 0.5 - menuSize), ui_fontScale->value, ui_fontScale->value, PF_LIGHT, i_banner_video[0], i_banner_video[1]);
	}
}

void Color_MenuDraw(void)
{
	menuSize = 170 * ui_fontScale->value;
	
	Draw_VideoBanner();

	Menu_AdjustCursor(&s_opengl2_menu, 1);
	Menu_Draw(&s_opengl2_menu);
}

int Color_MenuKey(int key)
{
	return Default_MenuKey(&s_opengl2_menu, key);
}

void M_Menu_Color_f(void) {
	M_ColorInit();
	M_PushMenu(Color_MenuDraw, Color_MenuKey);
}


static void ColorSettingsFunc(void *unused) {
	M_Menu_Color_f();
}


/*
** VID_MenuInit
*/

void VID_MenuInit (void) {
#ifndef _WINDOWS
	static char *resolutions[] = {
		"[Native][Desktop Resolution]",
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
		"[1920 1080][1080p FULL HDTV]",
		"[1920 1200][16:10]",
		"[2560 1440][16:9 WQHD]",
		"[2560 1600][16:10]",
		"[3440 1440][21:9 QHD]",
		"[3840 1600][12:5 UW TV]",
		"[3840 2160][16:9 UHD TV 4K]",
		"[4096 1716][2.39:1 DCI 4K WIDE]",
		"[4096 2160][1.89:1 DCI 4K]",
		"[Custom]", 0 };
#endif
	static char* parallax_names[] = { "off", "Parallax Mapping", "Relief Mapping", 0 };
	static char	*yesno_names[] = { "off", "yes", 0 };
	static char	*adaptive_vc[] = { "off", "standart", "adaptive", 0 };
	static char* customScreenRes[] = { "Custom Window Resolution", 0 };
	static char	*aniso_items[] =
	{	"Off",	  // 1
		"Low",	  // 2
		"Medium", // 4
		"Hight",  // 8
		"Ultra",  // 16
		0 };

	if(!r_selfShadowingParallax)
		r_selfShadowingParallax = Cvar_Get("r_selfShadowingParallax", "0", CVAR_ARCHIVE);

	if (!r_mode)
		r_mode = Cvar_Get ("r_mode", "0", CVAR_ARCHIVE);

	if (!r_textureAnisotropy)
		r_textureAnisotropy = Cvar_Get ("r_textureAnisotropy", "1", CVAR_ARCHIVE);

	if (!r_hdrLensFlares)
		r_hdrLensFlares = Cvar_Get ("r_hdrLensFlares", "0", CVAR_ARCHIVE);


	if (!r_hdrBloom)
		r_hdrBloom = Cvar_Get ("r_hdrBloom", "0", CVAR_ARCHIVE);

	if (!r_parallaxMapping)
		r_parallaxMapping = Cvar_Get ("r_parallaxMapping", "0", CVAR_ARCHIVE);

	if (r_parallaxMapping->integer > 3)
		r_parallaxMapping = Cvar_Get("r_parallaxMapping", "1", CVAR_ARCHIVE);

	r_parallaxScale->value = ClampCvar(1.0, 10.0, r_parallaxScale->value);
	r_parallaxMapping->integer = ClampCvarInteger(0, 3, r_parallaxMapping->integer);

	if (!r_dof)
		r_dof = Cvar_Get ("r_dof", "0", CVAR_ARCHIVE);

	if (!r_radialBlur->integer)
		r_radialBlur = Cvar_Get ("r_radialBlur", "0", CVAR_ARCHIVE);

	if (!r_imageAutoBump)
		r_imageAutoBump = Cvar_Get ("r_imageAutoBump", "0", CVAR_ARCHIVE);

	if (!r_vsync)
		r_vsync = Cvar_Get ("r_vsync", "0", CVAR_ARCHIVE);

	if (!ui_fontScale)
		ui_fontScale = Cvar_Get ("ui_fontScale", "2", CVAR_ARCHIVE);

	if (!r_ssao->integer)
		r_ssao = Cvar_Get ("r_ssao", 0, CVAR_ARCHIVE);

	if (!r_fxaa->integer)
		r_fxaa = Cvar_Get ("r_fxaa", 0, CVAR_ARCHIVE);

	if (!r_lightmapScale->value)
		r_lightmapScale = Cvar_Get ("r_lightmapScale", "0", CVAR_ARCHIVE);

	if (!r_motionBlur->integer)
		r_motionBlur = Cvar_Get ("r_motionBlur", "0", CVAR_ARCHIVE);

	drawIDlogo = false;

	s_opengl_menu.x = viddef.width * 0.50;
	s_opengl_menu.nitems = 0;

	s_mode_list.generic.type = MTYPE_SPINCONTROL;
	s_mode_list.generic.name = "Screen Resolution";
	s_mode_list.generic.x = 0;
	s_mode_list.generic.y = 10 * ui_fontScale->value;
#ifndef _WINDOWS
	s_mode_list.itemnames = resolutions;
	s_mode_list.curInteger = r_mode->integer;
#else
	if (gl_state.fullscreen) {
		s_mode_list.itemnames = vid_winModes;
		s_mode_list.curInteger = r_mode->integer;
	}
	else {
		if (r_customWindowWidth->integer >= 1024 && r_customWindowHeight->integer >= 768) {
			s_mode_list.itemnames = customScreenRes;
			s_mode_list.curInteger = 0;
		}
		else {
			s_mode_list.itemnames = vid_winModes;
			s_mode_list.curInteger = r_mode->integer;
		}
	}
#endif
	
	s_mode_list.generic.statusbar = "Screen Resolution <Requires Restart Video Sub-System>";

	s_fs_box.generic.type = MTYPE_SPINCONTROL;
	s_fs_box.generic.x = 0;
	s_fs_box.generic.y = 20 * ui_fontScale->value;
	s_fs_box.generic.name = "Fullscreen";
	s_fs_box.itemnames = yesno_names;
	s_fs_box.curInteger = r_fullScreen->integer;
	s_fs_box.generic.statusbar = " Use Full Screen <Requires Restart Video Sub-System>";

	// -----------------------------------------------------------------------

	s_aniso_list.generic.type = MTYPE_SPINCONTROL;
	s_aniso_list.generic.name = "Anisotropy Filtering";
	s_aniso_list.generic.x = 0;
	s_aniso_list.generic.y = 40 * ui_fontScale->value;
	s_aniso_list.itemnames = aniso_items;
	s_aniso_list.generic.callback = AnisoCallback;
	s_aniso_list.generic.statusbar = "Texture Filtering Quality <Requires Restart Video Sub-System>";
		
	if (r_textureAnisotropy->value == 1.0)
		s_aniso_list.curInteger = 0;
	else
		if (r_textureAnisotropy->value == 2.0)
			s_aniso_list.curInteger = 1;
	else
		if (r_textureAnisotropy->value == 4.0)
			s_aniso_list.curInteger = 2;
	else
		if (r_textureAnisotropy->value == 8.0)
			s_aniso_list.curInteger = 3;
	else
		if (r_textureAnisotropy->value == 16.0)
			s_aniso_list.curInteger = 4;
		else
			s_aniso_list.curInteger = 0;

	// -----------------------------------------------------------------------

	s_autoBump_list.generic.type = MTYPE_SPINCONTROL;
	s_autoBump_list.generic.name = "Generate Normal Maps";
	s_autoBump_list.generic.x = 0;
	s_autoBump_list.generic.y = 50 * ui_fontScale->value;
	s_autoBump_list.itemnames = yesno_names;
	s_autoBump_list.curInteger = r_imageAutoBump->integer;
	s_autoBump_list.generic.callback = autoBumpCallBack;
	s_autoBump_list.generic.statusbar = "Realtime Normal Maps Generation For Old Textures";

	s_parallax_box.generic.type = MTYPE_SPINCONTROL;
	s_parallax_box.generic.x = 0;
	s_parallax_box.generic.y = 60 * ui_fontScale->value;
	s_parallax_box.generic.name = "Parallax Mapping";
	s_parallax_box.itemnames = parallax_names;
	s_parallax_box.curInteger = clamp(r_parallaxMapping->integer, 0, 2);
	s_parallax_box.generic.callback = ParallaxCallback;
	s_parallax_box.generic.statusbar = "Virtual Displacement Mapping";

	s_parallax_shadow.generic.type = MTYPE_SPINCONTROL;
	s_parallax_shadow.generic.x = 0;
	s_parallax_shadow.generic.y = 70 * ui_fontScale->value;
	s_parallax_shadow.generic.name = "Self Shadowing Parallax";
	s_parallax_shadow.itemnames = yesno_names;
	s_parallax_shadow.curInteger = r_selfShadowingParallax->integer;
	s_parallax_shadow.generic.callback = reliefShadowCallback;
	s_parallax_shadow.generic.statusbar = "Virtual Displacement Mapping Self Shadowing";

	s_reliefScale_slider.generic.type = MTYPE_SLIDER;
	s_reliefScale_slider.generic.x = 0;
	s_reliefScale_slider.generic.y = 80 * ui_fontScale->value;
	s_reliefScale_slider.generic.name = "Relief Scale";
	s_reliefScale_slider.minValue = 1;
	s_reliefScale_slider.maxValue = 6;
	s_reliefScale_slider.curValue = r_parallaxScale->value;
	s_reliefScale_slider.generic.callback = reliefScaleCallback;
	s_reliefScale_slider.divRange = 1;
	s_reliefScale_slider.generic.statusbar = "Virtual Displacement Depth";


	s_ambientLevel_slider.generic.type = MTYPE_SLIDER;
	s_ambientLevel_slider.generic.x = 0;
	s_ambientLevel_slider.generic.y = 90 * ui_fontScale->value;
	s_ambientLevel_slider.generic.name = "Lightmap Brightness";
	s_ambientLevel_slider.generic.callback = ambientLevelCallback;
	s_ambientLevel_slider.minValue = 0;
	s_ambientLevel_slider.maxValue = 20;
	s_ambientLevel_slider.curValue = r_lightmapScale->value * 20;
	s_ambientLevel_slider.divRange = 20;
	s_ambientLevel_slider.percent = true;
	s_ambientLevel_slider.generic.statusbar = "Ambient Lighting Level";

	s_flare_box.generic.type = MTYPE_SPINCONTROL;
	s_flare_box.generic.x = 0;
	s_flare_box.generic.y = 110 * ui_fontScale->value;
	s_flare_box.generic.name = "Lens Flares";
	s_flare_box.itemnames = yesno_names;
	s_flare_box.curInteger = r_hdrLensFlares->integer;
	s_flare_box.generic.callback = FlareCallback;
	s_flare_box.generic.statusbar = "Pseudo Lens Flares";

	s_bloom_box.generic.type = MTYPE_SPINCONTROL;
	s_bloom_box.generic.x = 0;
	s_bloom_box.generic.y = 120 * ui_fontScale->value;
	s_bloom_box.generic.name = "Bloom";
	s_bloom_box.itemnames = yesno_names;
	s_bloom_box.curInteger = r_hdrBloom->integer;
	s_bloom_box.generic.callback = BloomCallback;
	s_bloom_box.generic.statusbar = "Draw Hdr Bloom Effect";

	s_dof_box.generic.type = MTYPE_SPINCONTROL;
	s_dof_box.generic.x = 0;
	s_dof_box.generic.y = 130 * ui_fontScale->value;
	s_dof_box.generic.name = "Depth of Field";
	s_dof_box.itemnames = yesno_names;
	s_dof_box.curInteger = r_dof->integer;
	s_dof_box.generic.callback = DofCallback;
	s_dof_box.generic.statusbar = "Draw Depth of Field Effect";

	s_radBlur_box.generic.type = MTYPE_SPINCONTROL;
	s_radBlur_box.generic.x = 0;
	s_radBlur_box.generic.y = 140 * ui_fontScale->value;
	s_radBlur_box.generic.name = "Radial Blur";
	s_radBlur_box.itemnames = yesno_names;
	s_radBlur_box.curInteger = r_radialBlur->integer;
	s_radBlur_box.generic.callback = RBCallback;
	s_radBlur_box.generic.statusbar = "Draw Radial Blur Effect";

	s_mb_box.generic.type = MTYPE_SPINCONTROL;
	s_mb_box.generic.x = 0;
	s_mb_box.generic.y = 150 * ui_fontScale->value;
	s_mb_box.generic.name = "Motion Blur";
	s_mb_box.itemnames = yesno_names;
	s_mb_box.curInteger = r_motionBlur->integer;
	s_mb_box.generic.callback = mbCallback;
	s_mb_box.generic.statusbar = "Draw Motion Blur Effect";

	s_ssao.generic.type = MTYPE_SPINCONTROL;
	s_ssao.generic.x = 0;
	s_ssao.generic.y = 160 * ui_fontScale->value;
	s_ssao.generic.name = "SSAO";
	s_ssao.itemnames = yesno_names;
	s_ssao.curInteger = r_ssao->integer;
	s_ssao.generic.callback = ssaoCallback;
	s_ssao.generic.statusbar = "Draw Screen Space Ambient Occlusion Effect";

	s_film_grain.generic.type = MTYPE_SPINCONTROL;
	s_film_grain.generic.x = 0;
	s_film_grain.generic.y = 170 * ui_fontScale->value;
	s_film_grain.generic.name = "Filmic FX";
	s_film_grain.itemnames = yesno_names;
	s_film_grain.curInteger = r_filmicFx->integer;
	s_film_grain.generic.callback = filmCallback;
	s_film_grain.generic.statusbar = "Chromatic Abberation, Lens Distortion Add Vignet Filters";

	s_fxaa_box.generic.type = MTYPE_SPINCONTROL;
	s_fxaa_box.generic.x = 0;
	s_fxaa_box.generic.y = 180 * ui_fontScale->value;
	s_fxaa_box.generic.name = "FXAA";
	s_fxaa_box.itemnames = yesno_names;
	s_fxaa_box.curInteger = r_fxaa->integer;
	s_fxaa_box.generic.callback = fxaaCallback;
	s_fxaa_box.generic.statusbar = "Use Post-Process Anti-Aliasing";

	s_finish_box.generic.type = MTYPE_SPINCONTROL;
	s_finish_box.generic.x = 0;
	s_finish_box.generic.y = 190 * ui_fontScale->value;
	s_finish_box.generic.name = "Vertical Sync";
	s_finish_box.generic.callback = vSyncCallBack;
	s_finish_box.curInteger = r_vsync->integer;
if (r_vsync->integer >= 3)
	Cvar_SetValue ("r_vsync", 2);
	s_finish_box.itemnames = adaptive_vc;
	s_finish_box.generic.statusbar = "Standart Or Adaptive";


	s_menuAction_color.generic.type = MTYPE_ACTION;
	s_menuAction_color.generic.x = 0;
	s_menuAction_color.generic.y = 210 * ui_fontScale->value;
	s_menuAction_color.generic.name = "Post-Process Settings...";
	s_menuAction_color.generic.callback = ColorSettingsFunc;
	s_menuAction_color.generic.statusbar = "Color Balance and Bloom Settings";

	s_defaults_action.generic.type = MTYPE_ACTION;
	s_defaults_action.generic.name = "reset to defaults";
	s_defaults_action.generic.x = 0;
	s_defaults_action.generic.y = 230 * ui_fontScale->value;
	s_defaults_action.generic.callback = ResetDefaults;
	s_defaults_action.generic.statusbar = "Reset And Restart";

	s_apply_action.generic.type = MTYPE_ACTION;
	s_apply_action.generic.name = "Apply Changes";
	s_apply_action.generic.x = 0;
	s_apply_action.generic.y = 240 * ui_fontScale->value;
	s_apply_action.generic.callback = ApplyChanges;
	s_apply_action.generic.statusbar = "Save Settings And Restart";
	menuSize = 240;

	Menu_AddItem (&s_opengl_menu, (void *)&s_mode_list);
	Menu_AddItem (&s_opengl_menu, (void *)&s_fs_box);

	Menu_AddItem (&s_opengl_menu, (void *)&s_aniso_list);

	Menu_AddItem (&s_opengl_menu, (void *)&s_autoBump_list);
	Menu_AddItem (&s_opengl_menu, (void *)&s_parallax_box);
	Menu_AddItem(&s_opengl_menu, (void *)&s_parallax_shadow);

	Menu_AddItem (&s_opengl_menu, (void *)&s_reliefScale_slider);
	Menu_AddItem (&s_opengl_menu, (void *)&s_ambientLevel_slider);
	Menu_AddItem (&s_opengl_menu, (void *)&s_flare_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_bloom_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_dof_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_radBlur_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_mb_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_ssao);
	Menu_AddItem (&s_opengl_menu, (void *)&s_film_grain);
	Menu_AddItem (&s_opengl_menu, (void *)&s_fxaa_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_finish_box);

	Menu_AddItem (&s_opengl_menu, (void *)&s_menuAction_color);
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

	s_current_menu = &s_opengl_menu;
	menuSize = 170 * ui_fontScale->value;

	Draw_VideoBanner();

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
		case K_XPAD_B:
		case K_XPAD_BACK:
		case K_ESCAPE:
			CancelChanges (NULL);
			return 0;

		case K_MWHEELUP:
		case K_KP_UPARROW:
		case K_XPAD_DPAD_UP:
		case K_UPARROW:
			m->cursor--;
			Menu_AdjustCursor (m, -1);
			break;

		case K_MWHEELDOWN:
		case K_KP_DOWNARROW:
		case K_XPAD_DPAD_DOWN:
		case K_DOWNARROW:
			m->cursor++;
			Menu_AdjustCursor (m, 1);
			break;
		case K_KP_LEFTARROW:
		case K_XPAD_DPAD_LEFT:
		case K_LEFTARROW:
			Menu_SlideItem (m, -1);
			break;
		case K_KP_RIGHTARROW:
		case K_XPAD_DPAD_RIGHT:
		case K_RIGHTARROW:
			Menu_SlideItem (m, 1);
			break;
		case K_KP_ENTER:
		case K_XPAD_A:
		case K_ENTER:
			if (!Menu_SelectItem (m))
				ApplyChanges (NULL);
			break;
	}

	return menu_in_sound;
}


