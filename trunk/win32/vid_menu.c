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

extern void M_ForceMenuOff( void );

int refresh=0;

int menuSize  = 0;

/*
====================================================================

MENU INTERACTION

====================================================================
*/
static menuframework_s	s_opengl_menu;
static menuframework_s *s_current_menu;
static int				s_current_menu_index;

static menulist_s		s_mode_list;
static menuslider_s		s_aniso_slider;
static menuslider_s		s_screensize_slider;
static menuslider_s		s_brightness_slider;
static menulist_s  		s_fs_box;

static menulist_s	    s_shadow_box;
static menulist_s	    s_flare_box;
static menulist_s	    s_tc_box;
static menulist_s	    s_refresh_box;
static menulist_s	    s_parallax_box;    
static menulist_s	    s_samples_list; 
static menuslider_s		s_texturelod_slider;

static menulist_s	    s_bloom_box;
static menulist_s	    s_dof_box;

static menulist_s  		s_finish_box;
static menuaction_s		s_apply_action;
static menuaction_s		s_defaults_action;

static	menulist_s			s_ab_list;
static	menulist_s			s_wb_list;
static	menulist_s			s_radBlur_box;
static	menulist_s			s_softParticles;
static	menulist_s			s_fxaa_box;
static	menulist_s			s_minimap;



/////////////////////////////////////////////////////////
//
// MENU GENERIC FUNCTIONS
//
/////////////////////////////////////////////////////////

static float ClampCvar(float min, float max, float value)
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

static void ScreenSizeCallback(void *s)
{
	menuslider_s *slider = (menuslider_s*) s;

	Cvar_SetValue("viewsize", slider->curvalue * 10);
}


static void RadarCallback( void *s )
{
	menulist_s *box = ( menulist_s * ) s;

	Cvar_SetValue( "r_radar", box->curvalue * 1 );
}

static void ShadowsCallback( void *s )
{
	menulist_s *box = ( menulist_s * ) s;

	Cvar_SetValue( "r_shadows", box->curvalue * 1 );
}

static void ParallaxCallback( void *s )
{
	menulist_s *box = ( menulist_s * ) s;

	Cvar_SetValue( "r_parallax", box->curvalue * 1 );
}


static void FlareCallback( void *s )
{
	menulist_s *box = ( menulist_s * ) s;

	Cvar_SetValue( "r_drawFlares", box->curvalue * 1 );
}

static void AnisoCallback( void *s )
{
	menuslider_s *slider = ( menuslider_s * ) s;

	Cvar_SetValue("r_anisotropic", slider->curvalue * 1 );
}

static void BrightnessCallback( void *s )
{
	float gamma;
	gamma = ( 0.8 - ( s_brightness_slider.curvalue/10.0 - 0.5 ) ) + 0.5;

	Cvar_SetValue( "r_gamma", gamma );
}


static void BloomCallback( void *s )
{
	menulist_s *box = ( menulist_s * ) s;

	Cvar_SetValue( "r_bloom", box->curvalue * 1 );
}

static void DofCallback( void *s )
{
	menulist_s *box = ( menulist_s * ) s;

	Cvar_SetValue( "r_dof", box->curvalue * 1 );
}

static void RBCallback( void *s )
{
	menulist_s *box = ( menulist_s * ) s;

	Cvar_SetValue( "r_radialBlur", box->curvalue * 1 );
}

static void softPartCallback( void *s )
{
	menulist_s *box = ( menulist_s * ) s;

	Cvar_SetValue( "r_softParticles", box->curvalue * 1 );
}

static void fxaaCallback( void *s )
{
	menulist_s *box = ( menulist_s * ) s;

	Cvar_SetValue( "r_fxaa", box->curvalue * 1 );
}

static void ResetDefaults( void *unused )
{
	VID_MenuInit();
}

static void ApplyChanges( void *unused )
{
	float gamma;


	/*
	** invert sense so greater = brighter, and scale to a range of 0.5 to 1.3
	*/
	gamma = ( 0.8 - ( s_brightness_slider.curvalue/10.0 - 0.5 ) ) + 0.5;

	Cvar_SetValue( "r_gamma", gamma );
	

	Cvar_SetValue( "r_anisotropic",			s_aniso_slider.curvalue);
	Cvar_SetValue( "r_fullScreen",			s_fs_box.curvalue );
	Cvar_SetValue( "r_drawFlares",			s_flare_box.curvalue );
    Cvar_SetValue( "r_textureCompression",	s_tc_box.curvalue );
	Cvar_SetValue( "r_mode",				s_mode_list.curvalue );
	Cvar_SetValue( "r_shadows",				s_shadow_box.curvalue );
    Cvar_SetValue( "r_parallax",			s_parallax_box.curvalue );
    Cvar_SetValue( "r_bloom",				s_bloom_box.curvalue );  
	Cvar_SetValue( "r_dof",					s_dof_box.curvalue );
	Cvar_SetValue( "r_radialBlur",			s_radBlur_box.curvalue );
	Cvar_SetValue( "r_softParticles",		s_softParticles.curvalue);
	Cvar_SetValue( "r_fxaa",				s_fxaa_box.curvalue);
	Cvar_SetValue( "r_vsync",				s_finish_box.curvalue);
	Cvar_SetValue( "r_radar",				s_minimap.curvalue);


	
/*	
Nvidia Coverange AA
	
Samples						# of Color/Z/Stencil	# of Coverage Samples
8x	                                4						8
8xQ (Quality)						8						8
16x									4						16
16xQ (Quality)						8						16
{"[off]", "[8x]", "[8xQ]", "[16x]", "[16xQ]", 0};
*/

	if(gl_state.wgl_nv_multisample_coverage_aviable){
	
		if (s_samples_list.curvalue == 1){//8x
		Cvar_SetValue("r_arbSamples", 4);
		Cvar_SetValue("r_nvSamplesCoverange", 8);
		}else
		if (s_samples_list.curvalue == 2){//8xQ
		Cvar_SetValue("r_arbSamples", 8);
		Cvar_SetValue("r_nvSamplesCoverange", 8);
		}else
		if (s_samples_list.curvalue == 3){//16x
		Cvar_SetValue("r_arbSamples", 4);
		Cvar_SetValue("r_nvSamplesCoverange", 16);
		}else
		if (s_samples_list.curvalue == 4){//16xQ
		Cvar_SetValue("r_arbSamples", 8);
		Cvar_SetValue("r_nvSamplesCoverange", 16);
		}else
		if (s_samples_list.curvalue == 0){ //off
			Cvar_SetValue("r_arbSamples", 0);
			Cvar_SetValue("r_nvSamplesCoverange", 8);
		}
	}else if(!gl_state.wgl_nv_multisample_coverage){
	// Multisampling
	if (s_samples_list.curvalue == 1)
		Cvar_SetValue("r_arbSamples", 2);
 	else 
		if (s_samples_list.curvalue == 2)
		Cvar_SetValue("r_arbSamples", 4);
	else 
		if (s_samples_list.curvalue == 3)
		Cvar_SetValue("r_arbSamples", 8);
	else
		if (s_samples_list.curvalue == 4)
		Cvar_SetValue("r_arbSamples", 16);
	else
		if (s_samples_list.curvalue == 0)
		Cvar_SetValue("r_arbSamples", 0);
	}
	// displayrefresh
	if (s_refresh_box.curvalue == 1)
	{
		if (r_displayRefresh->value < 60 || r_displayRefresh->value > 75)
			Cvar_SetValue("r_displayRefresh", 60);
	}
    else if (s_refresh_box.curvalue == 2)
	{
		if (r_displayRefresh->value < 70 || r_displayRefresh->value > 85)
			Cvar_SetValue("r_displayRefresh", 75);
	}
	else if (s_refresh_box.curvalue == 3)
	{
		if (r_displayRefresh->value < 85 || r_displayRefresh->value > 100)
			Cvar_SetValue("r_displayRefresh", 85);
	}
	else if (s_refresh_box.curvalue == 4)
	{
		if (r_displayRefresh->value < 100 || r_displayRefresh->value > 120)
			Cvar_SetValue("r_displayRefresh", 100);
	}
	else if (s_refresh_box.curvalue == 5)
	{
		if (r_displayRefresh->value < 120)
			Cvar_SetValue("r_displayRefresh", 120);
	}
	else
		Cvar_SetValue("r_displayRefresh", 0);


	/*
	** update appropriate stuff if we're running OpenGL and gamma
	** has been modified
	*/
		if ( r_gamma->modified )
			vid_ref->modified = true;

		if ( r_anisotropic->modified )
			vid_ref->modified = true;

		        
		if ( r_shadows->modified )
			vid_ref->modified = true;
				
		if ( r_bloom->modified )
			vid_ref->modified = true;

		if ( r_dof->modified )
			vid_ref->modified = true;

		if ( r_displayRefresh->modified )
			vid_ref->modified = true;
        
		if ( r_drawFlares->modified )
			vid_ref->modified = true;
        
        if ( r_parallax->modified )
			vid_ref->modified = true;

        if ( r_textureCompression->modified )
			vid_ref->modified = true;
		
		if ( r_vsync->modified )
			vid_ref->modified = true;
		
		if ( r_arbSamples->modified )
			vid_ref->modified = true;

		if ( r_nvSamplesCoverange->modified )
			vid_ref->modified = true;

		if ( r_radar->modified )
			vid_ref->modified = true;
		
		if(r_dof->modified)
			vid_ref->modified = true;

		if(r_radialBlur->modified)
			vid_ref->modified = true;
		
		if(r_softParticles->modified)
			vid_ref->modified = true;

		if(r_fxaa->modified)
			vid_ref->modified = true;
		
	M_ForceMenuOff();

}

static void CancelChanges( void *unused )
{
	extern void M_PopMenu( void );

	M_PopMenu();
}




static void abumpCB (void *s)
{
	menulist_s *box = ( menulist_s * ) s;

	Cvar_SetValue( "r_bumpAlias", box->curvalue * 1 );
}

static void wbumpCB (void *s)
{
	menulist_s *box = ( menulist_s * ) s;

	Cvar_SetValue( "r_bumpWorld", box->curvalue * 1 );
}

/*
** VID_MenuInit
*/
void VID_MenuInit( void )
{

	
	static char *resolutions[] = {		"[800 600][4:3]",   
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
										"[custom]", 0};
	
	static char	*yesno_names[]	=	{"no", "yes", 0};
	static char	*refresh[]		=	{"desktop", "60hz", "75hz", "85hz", "100hz", "120hz", 0};
	static char	*shadow_names[] =	{"off", "blob", "static volumes", "dynamic volumes", "full dynamic volumes", 0};

	static char	*samples[]		=	{"[off]", "[2x]", "[4x]", "[8x]", "[16x]", 0};
	static char	*samplesNV[]	=	{"[off]", "[8x]", "[8xQ]", "[16x]", "[16xQ]", 0};
	static char	*parallax[]		=	{"off", "Performance", "Quality", 0};
	static char	*vsync[]		=	{"off", "on", 0};
	static char	*alNo[]			=	{"not support", 0};
	static char	*radar[]		=	{"off", "map only", "map and entities", "move detector", 0};
	static char	*dof[]			=	{"off", "Performance", "Quality", 0};

	if (!r_mode)
		r_mode = Cvar_Get("r_mode", "0", 0);
		
    if (!r_anisotropic)
		r_anisotropic = Cvar_Get("r_anisotropic", "1",  CVAR_ARCHIVE);
	
	if (!r_textureCompression)
		  r_textureCompression = Cvar_Get( "r_textureCompression", "0",  CVAR_ARCHIVE );

	if ( !r_drawFlares )
		r_drawFlares = Cvar_Get( "r_drawFlares", "0",  CVAR_ARCHIVE );
	
	
	if ( !r_bloom )
		r_bloom = Cvar_Get( "r_bloom", "0", CVAR_ARCHIVE );
     
	if ( !r_shadows )
		r_shadows = Cvar_Get( "r_shadows", "0",  CVAR_ARCHIVE );
    
	if ( !r_parallax )
		r_parallax = Cvar_Get( "r_parallax", "0",  CVAR_ARCHIVE );

	if ( !r_dof )
		r_dof = Cvar_Get( "r_dof", "0",  CVAR_ARCHIVE );

	if(!r_radialBlur->value)
		r_radialBlur = Cvar_Get ("r_radialBlur", "0", CVAR_ARCHIVE);

	if (!r_bumpAlias)
		r_bumpAlias = Cvar_Get("r_bumpAlias", "0", CVAR_ARCHIVE);

		if (!r_bumpWorld)
		r_bumpWorld = Cvar_Get("r_bumpWorld", "0", CVAR_ARCHIVE);

	if ( !r_vsync )
		r_vsync = Cvar_Get( "r_vsync", "0", CVAR_ARCHIVE );

	if ( !scr_viewsize )
		scr_viewsize = Cvar_Get ("viewsize", "100", CVAR_ARCHIVE);

	if (!r_radar)
		r_radar = Cvar_Get ("r_radar", "0", CVAR_ARCHIVE);

	
	if (!cl_fontScale)
		cl_fontScale = Cvar_Get ("cl_fontScale", "1", CVAR_ARCHIVE);
		
	if(!r_softParticles->value)
		r_softParticles = Cvar_Get ("r_softParticles", 0, CVAR_ARCHIVE);

	if(!r_fxaa->value)
		r_fxaa = Cvar_Get ("r_fxaa", 0, CVAR_ARCHIVE);


	s_opengl_menu.x = viddef.width * 0.50;
	s_opengl_menu.nitems = 0;

	s_mode_list.generic.type = MTYPE_SPINCONTROL;
	s_mode_list.generic.name = "Screen Resolution";
	s_mode_list.generic.x = 0;
	s_mode_list.generic.y = 10*cl_fontScale->value;
	s_mode_list.itemnames = resolutions;
	s_mode_list.curvalue = r_mode->value;
	s_mode_list.generic.statusbar = "Requires Restart Video Sub-System";

	s_samples_list.generic.type = MTYPE_SPINCONTROL;
	if(gl_state.wgl_nv_multisample_coverage_aviable)
	s_samples_list.generic.name = "CSAA Multisampling";
	else
	s_samples_list.generic.name = "Multisampling";
	s_samples_list.generic.x = 0;
	s_samples_list.generic.y = 20*cl_fontScale->value;
	if(gl_state.wgl_nv_multisample_coverage_aviable)
	s_samples_list.itemnames = samplesNV;
	else
	s_samples_list.itemnames = samples;

	r_nvSamplesCoverange = Cvar_Get("r_nvSamplesCoverange", "8",  CVAR_ARCHIVE);
	r_arbSamples = Cvar_Get("r_arbSamples", "0",  CVAR_ARCHIVE);
	s_samples_list.generic.statusbar = "Requires Restart Video Sub-System";

	if(gl_state.wgl_nv_multisample_coverage_aviable){
	
	if (r_arbSamples->value == 8 && r_nvSamplesCoverange->value == 16) // 16xQ
		s_samples_list.curvalue = 4;
	else 
	if (r_arbSamples->value == 4 && r_nvSamplesCoverange->value == 16) //16x
		s_samples_list.curvalue = 3;
	else 
	if (r_arbSamples->value == 8 && r_nvSamplesCoverange->value == 8) //8xQ
		s_samples_list.curvalue = 2;
	else 
		if (r_arbSamples->value == 4 && r_nvSamplesCoverange->value == 8) //8x
		s_samples_list.curvalue = 1;
	else
	if (r_arbSamples->value == 2 && r_nvSamplesCoverange->value == 8) //8x
		s_samples_list.curvalue = 1;
	else
		s_samples_list.curvalue = 0; //off
	}
	else 
	if(!gl_state.wgl_nv_multisample_coverage){
	if (r_arbSamples->value == 16)
		s_samples_list.curvalue = 4;
	else 
	if (r_arbSamples->value == 8)
		s_samples_list.curvalue = 3;
	else 
	if (r_arbSamples->value == 4)
		s_samples_list.curvalue = 2;
	else 
		if (r_arbSamples->value == 2)
		s_samples_list.curvalue = 1;
	else
		s_samples_list.curvalue = 0;
	}

	// displayrefresh
	r_displayRefresh = Cvar_Get("r_displayRefresh", "0",  CVAR_ARCHIVE);
	if (r_displayRefresh->value >= 120)
		s_refresh_box.curvalue = 5;
	else if (r_displayRefresh->value >= 100)
		s_refresh_box.curvalue = 4;
	else if (r_displayRefresh->value >= 85)
		s_refresh_box.curvalue = 3;
	else if (r_displayRefresh->value >= 75)
		s_refresh_box.curvalue = 2;
	else if (r_displayRefresh->value >= 60)
		s_refresh_box.curvalue = 1;
	else
		s_refresh_box.curvalue = 0;

	s_screensize_slider.generic.type	= MTYPE_SLIDER;
	s_screensize_slider.generic.x		= 0;
	s_screensize_slider.generic.y		= 40*cl_fontScale->value;
	s_screensize_slider.generic.name	= "Screen Size";
	s_screensize_slider.curvalue = scr_viewsize->value/10;
	s_screensize_slider.minvalue = 3;
	s_screensize_slider.maxvalue = 12;
	s_screensize_slider.generic.callback = ScreenSizeCallback;

	s_brightness_slider.generic.type	= MTYPE_SLIDER;
	s_brightness_slider.generic.x	= 0;
	s_brightness_slider.generic.y	= 50*cl_fontScale->value;
	s_brightness_slider.generic.name	= "Brightness";
	s_brightness_slider.generic.callback = BrightnessCallback;
	s_brightness_slider.minvalue = 5;
	s_brightness_slider.maxvalue = 13;
	s_brightness_slider.curvalue = ( 1.3 - r_gamma->value + 0.5 ) * 10;

	s_refresh_box.generic.type	= MTYPE_SPINCONTROL;
	s_refresh_box.generic.x	    = 0;
	s_refresh_box.generic.y	    = 60*cl_fontScale->value;
	s_refresh_box.generic.name	= "Monitor Refresh";
    s_refresh_box.itemnames       = refresh;
	s_refresh_box.generic.statusbar = "Requires Restart Video Sub-System";

	s_fs_box.generic.type = MTYPE_SPINCONTROL;
	s_fs_box.generic.x	= 0;
	s_fs_box.generic.y	= 70*cl_fontScale->value;
	s_fs_box.generic.name	= "Fullscreen";
	s_fs_box.itemnames = yesno_names;
	s_fs_box.curvalue = r_fullScreen->value;
	s_fs_box.generic.statusbar = "Requires Restart Video Sub-System";

	// -----------------------------------------------------------------------

	s_aniso_slider.generic.type		= MTYPE_SLIDER;
	s_aniso_slider.generic.x		= 0;
	s_aniso_slider.generic.y		= 90*cl_fontScale->value;
	s_aniso_slider.generic.name		= "Texture Anisotropy Level";
	s_aniso_slider.minvalue			= 1;
	s_aniso_slider.maxvalue			= 16;
	s_aniso_slider.curvalue			=  r_anisotropic->value;
    s_aniso_slider.generic.callback	= AnisoCallback;

	s_tc_box.generic.type	= MTYPE_SPINCONTROL;
	s_tc_box.generic.x		= 0;
	s_tc_box.generic.y		= 100*cl_fontScale->value;
	s_tc_box.generic.name	= "Texture Compression";
    s_tc_box.itemnames		= yesno_names;
	s_tc_box.curvalue		= r_textureCompression->value;
	s_tc_box.generic.statusbar = "Requires Restart Video Sub-System";
	
	// -----------------------------------------------------------------------
	
	s_flare_box.generic.type	= MTYPE_SPINCONTROL;
	s_flare_box.generic.x	    = 0;
	s_flare_box.generic.y	    = 120*cl_fontScale->value;
	s_flare_box.generic.name	= "Flares";
	s_flare_box.itemnames       = yesno_names;
	s_flare_box.curvalue        = r_drawFlares->value;
	s_flare_box.generic.callback = FlareCallback;
		
	
	s_shadow_box.generic.type	  = MTYPE_SPINCONTROL;
	s_shadow_box.generic.x	      = 0;
	s_shadow_box.generic.y	      = 130*cl_fontScale->value;
	s_shadow_box.generic.name	  = "Shadows";
    s_shadow_box.itemnames        = shadow_names;
	s_shadow_box.curvalue         = r_shadows->value;
    s_shadow_box.generic.callback = ShadowsCallback;
		
	s_parallax_box.generic.type	  = MTYPE_SPINCONTROL;
	s_parallax_box.generic.x	      = 0;
	s_parallax_box.generic.y	      = 140*cl_fontScale->value;
	s_parallax_box.generic.name	  = "Parallax";
    s_parallax_box.itemnames        = parallax;
	s_parallax_box.curvalue         = r_parallax->value;
	s_parallax_box.generic.callback = ParallaxCallback;
	
	s_ab_list.generic.type = MTYPE_SPINCONTROL;
	s_ab_list.generic.name = "Models Bump Mapping";
	s_ab_list.generic.x = 0;
	s_ab_list.generic.y = 150*cl_fontScale->value;
	s_ab_list.itemnames = yesno_names;
	s_ab_list.curvalue = r_bumpAlias->value;
	s_ab_list.generic.callback = abumpCB;
	
	s_wb_list.generic.type = MTYPE_SPINCONTROL;
	s_wb_list.generic.name = "World Bump Mapping";
	s_wb_list.generic.x = 0;
	s_wb_list.generic.y = 160*cl_fontScale->value;
	s_wb_list.itemnames = yesno_names;
	s_wb_list.curvalue = r_bumpWorld->value;
	s_wb_list.generic.callback = wbumpCB;


	s_bloom_box.generic.type		= MTYPE_SPINCONTROL;
	s_bloom_box.generic.x			= 0;
	s_bloom_box.generic.y			= 170*cl_fontScale->value;
	s_bloom_box.generic.name		= "Light Blooms";
   	s_bloom_box.itemnames			= yesno_names;
	s_bloom_box.curvalue			= r_bloom->value;
    s_bloom_box.generic.callback	= BloomCallback;

	s_dof_box.generic.type		= MTYPE_SPINCONTROL;
	s_dof_box.generic.x			= 0;
	s_dof_box.generic.y			= 180*cl_fontScale->value;
	s_dof_box.generic.name		= "Depth of Field";
   	s_dof_box.itemnames			= yesno_names;
	s_dof_box.curvalue			= r_dof->value;
    s_dof_box.generic.callback	= DofCallback;
	
	s_radBlur_box.generic.type		= MTYPE_SPINCONTROL;
	s_radBlur_box.generic.x			= 0;
	s_radBlur_box.generic.y			= 190*cl_fontScale->value;
	s_radBlur_box.generic.name		= "Radial Blur";
   	s_radBlur_box.itemnames			= yesno_names;
	s_radBlur_box.curvalue			= r_radialBlur->value;
    s_radBlur_box.generic.callback	= RBCallback;
	
	s_softParticles.generic.type		= MTYPE_SPINCONTROL;
	s_softParticles.generic.x			= 0;
	s_softParticles.generic.y			= 200*cl_fontScale->value;
	s_softParticles.generic.name		= "Soft Particles";
   	s_softParticles.itemnames			= yesno_names;
	s_softParticles.curvalue			= r_softParticles->value;
    s_softParticles.generic.callback	= softPartCallback;

	s_fxaa_box.generic.type		= MTYPE_SPINCONTROL;
	s_fxaa_box.generic.x		= 0;
	s_fxaa_box.generic.y		= 210*cl_fontScale->value;
	s_fxaa_box.generic.name		= "FXAA";
   	s_fxaa_box.itemnames		= yesno_names;
	s_fxaa_box.curvalue			= r_fxaa->value;
    s_fxaa_box.generic.callback	= fxaaCallback;
	s_fxaa_box.generic.statusbar = "Nvidia Post-Process Anti-Aliasing";

	s_minimap.generic.type		= MTYPE_SPINCONTROL;
	s_minimap.generic.x			= 0;
	s_minimap.generic.y			= 220*cl_fontScale->value;
	s_minimap.generic.name		= "Draw Radar";
   	s_minimap.itemnames			= radar;
	s_minimap.curvalue			= r_radar->value;
    s_minimap.generic.callback	= RadarCallback;
	
	s_finish_box.generic.type = MTYPE_SPINCONTROL;
	s_finish_box.generic.x	= 0;
	s_finish_box.generic.y	= 230*cl_fontScale->value;
	s_finish_box.generic.name	= "Vertical Sync";
	s_finish_box.curvalue = r_vsync->value;
	s_finish_box.itemnames = yesno_names;
	s_finish_box.generic.statusbar = "Requires Restart Video Sub-System";
	
	s_defaults_action.generic.type = MTYPE_ACTION;
	s_defaults_action.generic.name = "reset to defaults";
	s_defaults_action.generic.x    = 0;
	s_defaults_action.generic.y    = 250*cl_fontScale->value;
	s_defaults_action.generic.callback = ResetDefaults;

	s_apply_action.generic.type = MTYPE_ACTION;
	s_apply_action.generic.name = "Apply Changes";
	s_apply_action.generic.x    = 0;
	s_apply_action.generic.y    = 260*cl_fontScale->value;
	s_apply_action.generic.callback = ApplyChanges;
	
	menuSize = 320;

	Menu_AddItem( &s_opengl_menu, ( void * ) &s_mode_list);
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_samples_list);
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_screensize_slider);
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_brightness_slider);
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_refresh_box );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_fs_box);
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_aniso_slider);
	
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_tc_box );
    
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_flare_box );

	Menu_AddItem( &s_opengl_menu, ( void * ) &s_shadow_box );
    Menu_AddItem( &s_opengl_menu, ( void * ) &s_parallax_box );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_ab_list);
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_wb_list);
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_bloom_box );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_dof_box );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_radBlur_box );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_softParticles );
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_fxaa_box);
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_minimap);
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_finish_box);

	Menu_AddItem( &s_opengl_menu, ( void * ) &s_defaults_action);
	Menu_AddItem( &s_opengl_menu, ( void * ) &s_apply_action);

	Menu_Center( &s_opengl_menu );
	s_opengl_menu.x -= 8;

}

/*
================
VID_MenuDraw
================
*/
void VID_MenuDraw (void)
{
	int w, h;

	s_current_menu = &s_opengl_menu;
	
	if(cl_fontScale->value == 1)
		menuSize = 180;

	// draw the banner
	Draw_GetPicSize( &w, &h, "m_banner_video" );
	Draw_PicScaled( (int)(viddef.width *0.5 - (w *0.5) *cl_fontScale->value), (int)((viddef.height *0.5 - menuSize )), cl_fontScale->value, cl_fontScale->value, "m_banner_video" );

	// move cursor to a reasonable starting position
	Menu_AdjustCursor( s_current_menu, 1 );

	// draw menu
	Menu_Draw( s_current_menu );
}

/*
================
VID_MenuKey
================
*/
int VID_MenuKey( int key )
{
	menuframework_s *m = s_current_menu;

	switch ( key )
	{
	case K_MOUSE2:
	case K_ESCAPE:
		CancelChanges(NULL);
		return 0;

	case K_MWHEELUP:
	case K_KP_UPARROW:
	case K_UPARROW:
		m->cursor--;
		Menu_AdjustCursor( m, -1);
		break;
	
	case K_MWHEELDOWN:
	case K_KP_DOWNARROW:
	case K_DOWNARROW:
		m->cursor++;
		Menu_AdjustCursor( m, 1);
		break;
	case K_KP_LEFTARROW:
	case K_LEFTARROW:
		Menu_SlideItem( m, -1);
		break;
	case K_KP_RIGHTARROW:
	case K_RIGHTARROW:
		Menu_SlideItem( m, 1);
		break;
	case K_KP_ENTER:
	case K_ENTER:
		if (!Menu_SelectItem(m))
			ApplyChanges( NULL );
		break;
	}

	return menu_in_sound;
}


