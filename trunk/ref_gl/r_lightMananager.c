/*
Copyright (C) 2004-2013 Quake2xp Team.

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
// r_lightMan.c
// Per-pixel light manager and editor basic idea based on Discoloda work

#include "r_local.h"

worldShadowLight_t *currentShadowLight;
worldShadowLight_t *shadowLight_static = NULL, *shadowLight_frame = NULL, *selectedShadowLight = NULL;
worldShadowLight_t	shadowLightsBlock[MAX_WORLD_SHADOW_LIHGTS];

static int num_dlits;
int num_nwmLights;
int num_visLights;
int numLightQ;
int vboPos, iboPos;
vec3_t player_org, v_forward, v_right, v_up;
trace_t CL_PMTraceWorld(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int mask);
qboolean R_MarkLightLeaves (worldShadowLight_t *light);


qboolean R_AddLightToFrame(worldShadowLight_t *light) {
	
	if (r_newrefdef.areabits){
		if (!(r_newrefdef.areabits[light->area >> 3] & (1 << (light->area & 7)))){
			return false;
		}
	}

	if(!HasSharedLeafs(light->vis, viewvis))
		return false;

	 if(!SphereInFrustum(light->origin, light->radius))
		 return false;
	 
	 return true;
}

void R_AddDynamicLight(dlight_t *dl) {
	
	worldShadowLight_t *light;
	int i;

	if(!SphereInFrustum(dl->origin, dl->intensity))
		return;
	
	light = &shadowLightsBlock[num_dlits++];
	memset(light, 0, sizeof(worldShadowLight_t));
	light->next = shadowLight_frame;
	shadowLight_frame = light;

	VectorCopy(dl->origin, light->origin);
	VectorCopy(dl->color, light->startColor);
	VectorCopy(dl->angles, light->angles);

	for (i = 0; i < 3; i++) {
		light->mins[i] = light->origin[i] - dl->intensity;
		light->maxs[i] = light->origin[i] + dl->intensity;
	}

	light->style = 0;
	light->filter = dl->filter;
	light->radius = dl->intensity;
	light->isStatic = 0;
	light->_cone = dl->_cone;
	light->isNoWorldModel = 0;
	light->isShadow = 1;
}

void R_AddNoWorldModelLight() {
	
	worldShadowLight_t *light;
	int i;

	light = &shadowLightsBlock[num_nwmLights++];
	memset(light, 0, sizeof(worldShadowLight_t));
	light->next = shadowLight_frame;
	shadowLight_frame = light;
	VectorSet(light->origin, -100, 76, 76);
	VectorSet(light->startColor, 1.0, 1.0, 1.0);
	VectorSet(light->angles, 0, 0, 0);
	light->radius = 256;

	for (i = 0; i < 3; i++) {
		light->mins[i] = light->origin[i] - light->radius;
		light->maxs[i] = light->origin[i] + light->radius;
	}

	light->style = 0;
	light->filter = 0;
	light->isStatic = 1;
	light->isShadow = 0;
	light->_cone = 0;
	light->isNoWorldModel = 1;
}

int lightVissFrame;

void R_PrepareShadowLightFrame(void) {
	
	int i;
	worldShadowLight_t *light;
	
	num_dlits = 0;
	num_nwmLights = 0;
	shadowLight_frame = NULL;

	// add pre computed lights
	if(shadowLight_static) {
		for(light = shadowLight_static; light; light = light->s_next) {
				
		if (!gl_state.createVbo){
			if(!R_AddLightToFrame(light))
				continue;
		}

			light->next = shadowLight_frame;
			shadowLight_frame = light;
		}
	}

	// add tempory lights
	for(i=0;i<r_newrefdef.num_dlights;i++) {
		if(num_dlits > MAX_WORLD_SHADOW_LIHGTS)
			break;
		R_AddDynamicLight(&r_newrefdef.dlights[i]);
	}


	if(r_newrefdef.rdflags & RDF_NOWORLDMODEL){
		R_AddNoWorldModelLight();
	}

	if(!shadowLight_frame) 
		return;
		
	for(light = shadowLight_frame; light; light = light->next) {
	
	if(!light->isStatic)
		if(!R_MarkLightLeaves(light))
			return;

	//fully/partially in frustum
	if(!intersectsBoxPoint(light->mins, light->maxs, r_origin))
	{
		if(r_newrefdef.fov_x <= 90)
			/// Berserker: этот способ быстрее, но при углах обзора больше 90, дает артефакты куллинга;
			/// поэтому юзаем его тока при углах меньших или равных 90.
			R_ProjectSphere(light, light->scizz.coords);
		else
			boxScreenSpaceRect(light, light->scizz.coords);
	}
	else
	{	//viewport is ofs/width based
		light->scizz.coords[0] = r_viewport[0];
		light->scizz.coords[1] = r_viewport[1];
		light->scizz.coords[2] = r_viewport[0] + r_viewport[2];
		light->scizz.coords[3] = r_viewport[1] + r_viewport[3];
	}

	MakeFrustum4Light(light, true);

	VectorCopy(light->startColor, light->color);
		
	if(!(r_newrefdef.rdflags & RDF_NOWORLDMODEL)){
		
		if(light->style < 31){
			light->color[0] *= r_newrefdef.lightstyles[light->style].rgb[0];
			light->color[1] *= r_newrefdef.lightstyles[light->style].rgb[1];
			light->color[2] *= r_newrefdef.lightstyles[light->style].rgb[2];
		}
		else
		{
			light->color[0] += r_newrefdef.lightstyles[light->style].rgb[0]*0.5 - light->startColor[0];
			light->color[1] += r_newrefdef.lightstyles[light->style].rgb[1]*0.5 - light->startColor[1];
			light->color[2] += r_newrefdef.lightstyles[light->style].rgb[2]*0.5 - light->startColor[2];
		}
	}

	}

}

void FS_StripExtension (const char *in, char *out, size_t size_out)
{
	char *last = NULL;

	if (size_out == 0)
		return;

	while (*in && size_out > 1)
	{
		if (*in == '.')
			last = out;
		else if (*in == '/' || *in == '\\' || *in == ':')
			last = NULL;
		*out++ = *in++;
		size_out--;
	}
	if (last)
		*last = 0;
	else
		*out = 0;
}

void R_SaveLights_f(void) {
	
	char				name[MAX_QPATH], path[MAX_QPATH];
	FILE				*f;
	int					i=0;
	
	if(!r_lightEditor->value){
		Com_Printf("Type r_lightEditor 1 to enable light editing.\n");
		return;
	}

	if(!shadowLight_static) 
		return;

	FS_StripExtension(r_worldmodel->name, name, sizeof (name));
	Com_sprintf(path, sizeof(path),"%s/%s.xplit", FS_Gamedir(), name);
	remove(path); //remove prev version

	f = fopen(path, "w");
	if(!f) {
		Com_Printf("Could not open %s.\n", path);
		return;
	}
	
	fprintf(f, "//ReLight configuration file\n//Generated by quake2xp\n\n");

	for(currentShadowLight = shadowLight_static; currentShadowLight; currentShadowLight = currentShadowLight->s_next){

		fprintf(f, "//Light %i\n", i);
		fprintf(f, "{\n");
		fprintf(f, "\"classname\" \"light\"\n");
		fprintf(f, "\"origin\" \"%i %i %i\"\n", (int)currentShadowLight->origin[0], (int)currentShadowLight->origin[1], (int)currentShadowLight->origin[2]);
		fprintf(f, "\"radius\" \"%i\"\n",		(int)currentShadowLight->radius);
		fprintf(f, "\"color\" \"%f %f %f\"\n",		 currentShadowLight->startColor[0],	 currentShadowLight->startColor[1],	 currentShadowLight->startColor[2]);
		fprintf(f, "\"style\" \"%i\"\n",		(int)currentShadowLight->style);
		fprintf(f, "\"filter\" \"%i\"\n",		(int)currentShadowLight->filter);
		fprintf(f, "\"angles\" \"%i %i %i\"\n", (int)currentShadowLight->angles[0], (int)currentShadowLight->angles[1], (int)currentShadowLight->angles[2]);
		fprintf(f, "\"speed\" \"%f %f %f\"\n",		 currentShadowLight->speed[0],		 currentShadowLight->speed[1],	     currentShadowLight->speed[2]);
		if (!strcmp(Cmd_Argv(1), "forceshadow"))
			fprintf(f, "\"shadow\" \"%i\"\n", 1);
		else
			fprintf(f, "\"shadow\" \"%i\"\n", currentShadowLight->isShadow);
		fprintf(f, "\"ambient\" \"%i\"\n", currentShadowLight->isAmbient);
		fprintf(f, "\"_cone\" \"%f\"\n", currentShadowLight->_cone);
		fprintf(f, "}\n");
		i++;
	}

	fclose(f);
	
		Com_Printf(""S_COLOR_MAGENTA"SaveLights_f: "S_COLOR_WHITE"Save lights to "S_COLOR_GREEN"%s.xplit\n"S_COLOR_WHITE"Save "S_COLOR_GREEN"%i"S_COLOR_WHITE" lights\n", name, i);

}

static void DeleteCurrentLight(worldShadowLight_t *l) {
	worldShadowLight_t *light;

	if(l == shadowLight_static) {
		shadowLight_static = l->s_next;
	}
	else {
		for(light = shadowLight_static; light; light = light->s_next) {
			if(light->s_next == l) {
				light->s_next = l->s_next;
				break;
			}
		}
	}

	free(l);
}

void R_Light_Spawn_f(void) {
	vec3_t color = {1.0, 1.0, 1.0}, end, spawn;
	trace_t trace;

	if(!r_lightEditor->value){
		Com_Printf("Type r_lightEditor 1 to enable light editing.\n");
		return;
	}

	VectorMA(player_org, 1024, v_forward, end);
	trace = CL_PMTraceWorld(player_org, vec3_origin, vec3_origin, end, MASK_SOLID);
	
	if (trace.fraction != 1.0){
		VectorMA(trace.endpos, -10, v_forward, spawn);
		R_AddNewWorldLight(spawn, color, 300, 0, 0, vec3_origin, vec3_origin, 1, 1, 0, 0);
	}
}

void R_Light_SpawnToCamera_f(void) {
	vec3_t color = {1.0, 1.0, 1.0};

	if(!r_lightEditor->value){
		Com_Printf("Type r_lightEditor 1 to enable light editing.\n");
		return;
	}

	R_AddNewWorldLight(player_org, color, 300, 0, 0, vec3_origin, vec3_origin, 1, 1, 0, 0);
}

void R_Light_Copy_f(void) {
	vec3_t color, spawn, origin, angles, speed;
	float radius, _cone;
	int style, filter, shadow, ambient;

	if(!r_lightEditor->value){
		Com_Printf("Type r_lightEditor 1 to enable light editing.\n");
		return;
	}
	
	if(!selectedShadowLight)
	{
	Com_Printf("No selected light.\n");
		return;
	}

	VectorCopy(selectedShadowLight->origin, origin);
	VectorCopy(selectedShadowLight->color, color);
	VectorCopy(selectedShadowLight->angles, angles);
	VectorCopy(selectedShadowLight->speed, speed);
	radius = selectedShadowLight->radius;
	style = selectedShadowLight->style;
	filter = selectedShadowLight->filter;
	shadow = selectedShadowLight->isShadow;
	ambient = selectedShadowLight->isAmbient;
	_cone = selectedShadowLight->_cone;

	VectorMA(origin, -50, v_forward, spawn);
	selectedShadowLight = R_AddNewWorldLight(spawn, color, radius, style, filter, angles, vec3_origin, 1, shadow, ambient, _cone);
	R_MarkLightLeaves(selectedShadowLight);
	MakeFrustum4Light(selectedShadowLight, true);
	
	//reset vbo data - recalc it!
	selectedShadowLight->vboId = 0;
	selectedShadowLight->iboId = 0;
	selectedShadowLight->iboNumIndices = 0;

}

void R_EditSelectedLight_f(void) {
	
	vec3_t color, origin, angles, speed;
	float radius, _cone;
	int style, filter, shadow, ambient;
	
	if(!r_lightEditor->value){
		Com_Printf("Type r_lightEditor 1 to enable light editing.\n");
		return;
	}

	if(!selectedShadowLight)
	{
	Com_Printf("No selected light.\n");
		return;
	}
	
	VectorCopy(selectedShadowLight->origin, origin);
	VectorCopy(selectedShadowLight->color, color);
	VectorCopy(selectedShadowLight->angles, angles);
	radius = selectedShadowLight->radius;
	style = selectedShadowLight->style;
	filter = selectedShadowLight->filter;
	shadow = selectedShadowLight->isShadow;
	ambient = selectedShadowLight->isAmbient;
	_cone = selectedShadowLight->_cone;

	if (!strcmp(Cmd_Argv(1), "origin")) {
		if(Cmd_Argc() != 5) {
			Com_Printf("usage: editLight: %s X Y Z\nCurrent Origin: %.4f %.4f %.4f\n", Cmd_Argv(0),
				selectedShadowLight->origin[0], 
				selectedShadowLight->origin[1], 
				selectedShadowLight->origin[2]);
			return;
		}
		origin[0] = atof(Cmd_Argv(2));
		origin[1] = atof(Cmd_Argv(3));
		origin[2] = atof(Cmd_Argv(4));
		VectorCopy(origin, selectedShadowLight->origin);
		//reset vbo data - recalc it!
	selectedShadowLight->vboId = 0;
	selectedShadowLight->iboId = 0;
	selectedShadowLight->iboNumIndices = 0;
	} 
	else
	if (!strcmp(Cmd_Argv(1), "color")) {
		if(Cmd_Argc() != 5) {
			Com_Printf("usage: editLight: %s Red Green Blue\nCurrent Color: %.4f %.4f %.4f\n", Cmd_Argv(0),
			selectedShadowLight->color[0], 
			selectedShadowLight->color[1], 
			selectedShadowLight->color[2]);
			return;
		}
		color[0] = atof(Cmd_Argv(2));
		color[1] = atof(Cmd_Argv(3));
		color[2] = atof(Cmd_Argv(4));
		VectorCopy(color, selectedShadowLight->startColor);
	} 
	else
	if (!strcmp(Cmd_Argv(1), "speed")) {
		if(Cmd_Argc() != 5) {
			Com_Printf("usage: editLight: %s X rotate speed Y rotate speed Z rotate speed\nCurrent speed rotations: %.4f %.4f %.4f\n", Cmd_Argv(0),
			selectedShadowLight->speed[0], 
			selectedShadowLight->speed[1], 
			selectedShadowLight->speed[2]);
			return;
		}
		speed[0] = atof(Cmd_Argv(2));
		speed[1] = atof(Cmd_Argv(3));
		speed[2] = atof(Cmd_Argv(4));
		VectorCopy(speed, selectedShadowLight->speed);
	} 
	else
	if (!strcmp(Cmd_Argv(1), "radius")) {
		if(Cmd_Argc() != 3) {
			Com_Printf("usage: editLight: %s value\nCurrent Radius: %.1f\n", Cmd_Argv(0), 
			selectedShadowLight->radius);
			return;
		}
		radius = atof(Cmd_Argv(2));
		selectedShadowLight->radius = radius;

		selectedShadowLight->vboId = 0;
		selectedShadowLight->iboId = 0;
		selectedShadowLight->iboNumIndices = 0;
	 } 
	else
	if (!strcmp(Cmd_Argv(1), "cone")) {
		if(Cmd_Argc() != 3) {
			Com_Printf("usage: editLight: %s value\nCurrent Light Cone: %.1f\n", Cmd_Argv(0), 
				selectedShadowLight->_cone);
			return;
		}
		_cone = atof(Cmd_Argv(2));
		selectedShadowLight->_cone = _cone;

		selectedShadowLight->vboId = 0;
		selectedShadowLight->iboId = 0;
		selectedShadowLight->iboNumIndices = 0;
	 } 
	else
	 if (!strcmp(Cmd_Argv(1), "style")) {
		if(Cmd_Argc() != 3) {
			Com_Printf("usage: editLight %s value\nCurrent Style %i\n", Cmd_Argv(0),
			selectedShadowLight->style);
			return;
		}
		style = atoi(Cmd_Argv(2)); 
		selectedShadowLight->style = style;
	 }
	 else
		if (!strcmp(Cmd_Argv(1), "filter")) {
		if(Cmd_Argc() != 3) {
			Com_Printf("usage: editLight %s value\nCurrent Cube Filter %i\n", Cmd_Argv(0),
			selectedShadowLight->filter);
			return;
		}
		filter = atoi(Cmd_Argv(2)); 
		selectedShadowLight->filter = filter;
	 }
	 else
		if (!strcmp(Cmd_Argv(1), "angles")) {
		if(Cmd_Argc() != 5) {
			Com_Printf("usage: editLight: %s X Y Z \nCurrent Angles: %.4f %.4f %.4f\n", Cmd_Argv(0),
			selectedShadowLight->angles[0], 
			selectedShadowLight->angles[1], 
			selectedShadowLight->angles[2]);
			return;
		}
		angles[0] = atof(Cmd_Argv(2));
		angles[1] = atof(Cmd_Argv(3));
		angles[2] = atof(Cmd_Argv(4));
		VectorCopy (angles, selectedShadowLight->angles);
	} 
	else
		if (!strcmp(Cmd_Argv(1), "shadow")) {
		if(Cmd_Argc() != 3) {
			Com_Printf("usage: editLight %s value\nCurrent Shadow Flag is %i\n", Cmd_Argv(0),
			selectedShadowLight->isShadow);
			return;
		}
		shadow = atoi(Cmd_Argv(2)); 
		selectedShadowLight->isShadow = shadow;
	 }
	else
		if (!strcmp(Cmd_Argv(1), "ambient")) {
		if(Cmd_Argc() != 3) {
			Com_Printf("usage: editLight %s value\nCurrent Ambient Flag is %i\n", Cmd_Argv(0),
				selectedShadowLight->isAmbient);
			return;
		}
		ambient = atoi(Cmd_Argv(2)); 
		selectedShadowLight->isAmbient = ambient;
	 }
	else{
		 Com_Printf("Light Properties: Origin: %.4f %.4f %.4f\nColor: %.4f %.4f %.4f\nRadius %.1f\nStyle %i\nFilter Cube %i\nAngles: %.4f %.4f %.4f\nSpeed: %.4f %.4f %.4f\nShadows %i\nAmbient %i\nCone %f\n",
		 selectedShadowLight->origin[0], selectedShadowLight->origin[1], selectedShadowLight->origin[2],
		 selectedShadowLight->color[0], selectedShadowLight->color[1], selectedShadowLight->color[2], 
		 selectedShadowLight->radius,
		 selectedShadowLight->style,
		 selectedShadowLight->filter,
		 selectedShadowLight->angles[0], selectedShadowLight->angles[1], selectedShadowLight->angles[2],
		 selectedShadowLight->speed[0], selectedShadowLight->speed[1], selectedShadowLight->speed[2],
		 selectedShadowLight->isShadow, 
		 selectedShadowLight->isAmbient,
		 selectedShadowLight->_cone);

	}

}



void R_MoveLightToRight_f(void) {
	
	vec3_t origin;
	float offset;
	
	if(!r_lightEditor->value){
		Com_Printf("Type r_lightEditor 1 to enable light editing.\n");
		return;
	}

	if(!selectedShadowLight)
	{
	Com_Printf("No selected light.\n");
		return;
	}

	if( Cmd_Argc() != 2 )
	{
		Com_Printf("Usage: moveLight_right: <value>\n");
		return;
	}

	offset = atof(Cmd_Argv(1)); 

	VectorCopy(selectedShadowLight->origin, origin);

	if(r_CameraSpaceLightMove->value)
		VectorMA(origin, offset, v_right, origin);
	else
		origin[0] += offset;

	VectorCopy(origin, selectedShadowLight->origin);

	//reset vbo data - recalc it!
	selectedShadowLight->vboId = 0;
	selectedShadowLight->iboId = 0;
	selectedShadowLight->iboNumIndices = 0;
}

void R_MoveLightForward_f(void) {
	
	vec3_t origin;
	float  offset, fix;
	
	if(!r_lightEditor->value){
		Com_Printf("Type r_lightEditor 1 to enable light editing.\n");
		return;
	}

	if(!selectedShadowLight)
	{
	Com_Printf("No selected light.\n");
		return;
	}

	if( Cmd_Argc() != 2 )
	{
		Com_Printf("Usage: moveLight_forward: <value>\n");
		return;
	}

	offset = atof(Cmd_Argv(1)); 

	VectorCopy(selectedShadowLight->origin, origin);
	
	if(r_CameraSpaceLightMove->value){
		fix = origin[2];
		VectorMA(origin, offset, v_forward, origin);
		origin[2] = fix;
	}
	else
		origin[1] += offset;

	VectorCopy(origin, selectedShadowLight->origin);
	
	//reset vbo data - recalc it!
	selectedShadowLight->vboId = 0;
	selectedShadowLight->iboId = 0;
	selectedShadowLight->iboNumIndices = 0;
}

void R_MoveLightUpDown_f(void) {
	
	vec3_t origin;
	float offset;
	
	if(!r_lightEditor->value){
		Com_Printf("Type r_lightEditor 1 to enable light editing.\n");
		return;
	}

	if(!selectedShadowLight)
	{
	Com_Printf("No selected light.\n");
		return;
	}


	if( Cmd_Argc() != 2 )
	{
		Com_Printf("Usage: moveSelectedLight_right: <value>\n");
		return;
	}

	VectorCopy(selectedShadowLight->origin, origin);

	offset = atof(Cmd_Argv(1)); 
	origin[2] += offset;

	VectorCopy(origin, selectedShadowLight->origin);

	//reset vbo data - recalc it!
	selectedShadowLight->vboId = 0;
	selectedShadowLight->iboId = 0;
	selectedShadowLight->iboNumIndices = 0;
}

void R_ChangeLightRadius_f(void) {
	
	float radius, offset;
	
	if(!r_lightEditor->value){
		Com_Printf("Type r_lightEditor 1 to enable light editing.\n");
		return;
	}

	if(!selectedShadowLight)
	{
	Com_Printf("No selected light.\n");
		return;
	}

	if( Cmd_Argc() != 2 )
	{
		Com_Printf("Usage: ajustSelectedLightRadius: <value>\n");
		return;
	}

	radius = selectedShadowLight->radius;

	offset = atof(Cmd_Argv(1));
	
	radius += offset;
	if(radius <10)
		radius = 10;

	selectedShadowLight->radius = radius;
	
	//reset vbo data - recalc it!
	selectedShadowLight->vboId = 0;
	selectedShadowLight->iboId = 0;
	selectedShadowLight->iboNumIndices = 0;

}

void R_ChangeLightCone_f(void) {
	
	float cone, offset;
	
	if(!r_lightEditor->value){
		Com_Printf("Type r_lightEditor 1 to enable light editing.\n");
		return;
	}

	if(!selectedShadowLight)
	{
	Com_Printf("No selected light.\n");
		return;
	}

	if( Cmd_Argc() != 2 )
	{
		Com_Printf("Usage: ajustSelectedLightRadius: <value>\n");
		return;
	}

	cone = selectedShadowLight->_cone;

	offset = atof(Cmd_Argv(1));
	
	cone += offset;
	if(cone >999999)
		cone = 999999;

	if(cone < 0)
		cone = 0;

	selectedShadowLight->_cone = cone;
	
	//reset vbo data - recalc it!
	selectedShadowLight->vboId = 0;
	selectedShadowLight->iboId = 0;
	selectedShadowLight->iboNumIndices = 0;

}


void R_Light_Delete_f(void) {

	if(!r_lightEditor->value){
		Com_Printf("Type r_lightEditor 1 to enable light editing.\n");
		return;
	}
	
	if(!selectedShadowLight)
	{
	Com_Printf("No selected light.\n");
		return;
	}

	DeleteCurrentLight(selectedShadowLight);
	selectedShadowLight = NULL;
}


char buff0[128];
char buff1[128];
char buff2[128];
char buff3[128];
char buff4[128];
char buff5[128];
char buff6[128];
char buff7[128];
char buff8[128];
char buff9[128];

void UpdateLightEditor(void){

	vec3_t end_trace, mins = { -5.0f, -5.0f, -5.0f }, maxs = { 5.0f, 5.0f, 5.0f };
	vec3_t	corners[8], tmp;
	int j;
	float rad;
	float fraction = 1.0;
	vec3_t	v[8];
	trace_t trace_light, trace_bsp;
	unsigned	headNode;
	vec3_t tmpOrg;
	float tmpRad;

	if(!r_lightEditor->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if(!currentShadowLight->isStatic)
		return;

	qglDisable(GL_BLEND);
	qglDisable(GL_STENCIL_TEST);

	// stupid player camera and angles corruption, fixed
	VectorCopy(r_origin, player_org);
	AngleVectors(r_newrefdef.viewangles, v_forward, v_right, v_up);

	// create a temp hull from bounding box
	headNode = CM_HeadnodeForBox (mins, maxs);
	VectorMA(r_origin, 1024, v_forward, end_trace);

	trace_bsp = CL_PMTraceWorld(r_origin, vec3_origin, vec3_origin, end_trace, MASK_SOLID); //bsp collision with bmodels

	// light in focus?
	trace_light = CM_TransformedBoxTrace(	r_origin, trace_bsp.endpos, vec3_origin, vec3_origin, headNode, MASK_ALL, 
											currentShadowLight->origin, vec3_origin); // find light

	if(trace_light.fraction  < fraction){
		selectedShadowLight = currentShadowLight;
		fraction = trace_light.fraction;
	}

	qglDisable(GL_TEXTURE_2D);
	qglDisable(GL_CULL_FACE);

	if(currentShadowLight != selectedShadowLight){

	for (j=0; j<8; j++) {
		tmp[0] = (j & 1) ? -5.0f : 5.0f;
		tmp[1] = (j & 2) ? -5.0f : 5.0f;
		tmp[2] = (j & 4) ? -5.0f : 5.0f;

	VectorAdd(tmp, currentShadowLight->origin, corners[j]);
	}
	qglColor3fv(currentShadowLight->color);
	qglEnable(GL_LINE_SMOOTH);
	qglLineWidth(3.0);

	qglBegin(GL_LINE_LOOP);
	qglVertex3fv(corners[0]);
	qglVertex3fv(corners[2]);
	qglVertex3fv(corners[3]);
	qglVertex3fv(corners[1]);
	qglEnd();

	qglBegin(GL_LINE_LOOP);
	qglVertex3fv(corners[4]);
	qglVertex3fv(corners[6]);
	qglVertex3fv(corners[7]);
	qglVertex3fv(corners[5]);
	qglEnd();

	qglBegin(GL_LINES);
	qglVertex3fv(corners[0]);
	qglVertex3fv(corners[4]);
	qglVertex3fv(corners[1]);
	qglVertex3fv(corners[5]);
	qglVertex3fv(corners[2]);
	qglVertex3fv(corners[6]);
	qglVertex3fv(corners[3]);
	qglVertex3fv(corners[7]);
	qglEnd();
	qglDisable(GL_LINE_SMOOTH);
	qglColor3f(1,1,1);

	}

	if(selectedShadowLight){	

	qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	qglEnable(GL_LINE_SMOOTH);
	qglDisable(GL_DEPTH_TEST);
	qglLineWidth(3.0);
	VectorCopy(selectedShadowLight->origin, tmpOrg);
	tmpRad = selectedShadowLight->radius;
	qglColor3fv(selectedShadowLight->color);
	rad = tmpRad;
	sprintf(buff0,	"Origin: %.3f %.3f %.3f",	selectedShadowLight->origin[0], 
												selectedShadowLight->origin[1], 
												selectedShadowLight->origin[2]);
	sprintf(buff1,	"Color: %.3f %.3f %.3f",	selectedShadowLight->color[0], 
												selectedShadowLight->color[1], 
												selectedShadowLight->color[2]);
	sprintf(buff2,	"Radius: %.3f",				selectedShadowLight->radius);		
	sprintf(buff3,	"Style: %i",				selectedShadowLight->style);
	sprintf(buff4,	"Filter: %i",				selectedShadowLight->filter);
	sprintf(buff5,	"Angles: %.3f %.3f %.3f",	selectedShadowLight->angles[0], 
												selectedShadowLight->angles[1], 
												selectedShadowLight->angles[2]);
	sprintf(buff6,	"Speed: %.3f %.3f %.3f",	selectedShadowLight->speed[0], 
												selectedShadowLight->speed[1], 
												selectedShadowLight->speed[2]);
	sprintf(buff7,	"Shadow: %i",				selectedShadowLight->isShadow);
	sprintf(buff8,	"Ambient: %i",				selectedShadowLight->isAmbient);
	sprintf(buff9,	"Cone: %f",					selectedShadowLight->_cone);

	VectorSet(v[0], tmpOrg[0]-rad, tmpOrg[1]-rad, tmpOrg[2]-rad);
	VectorSet(v[1], tmpOrg[0]-rad, tmpOrg[1]-rad, tmpOrg[2]+rad);
	VectorSet(v[2], tmpOrg[0]-rad, tmpOrg[1]+rad, tmpOrg[2]-rad);
	VectorSet(v[3], tmpOrg[0]-rad, tmpOrg[1]+rad, tmpOrg[2]+rad);
	VectorSet(v[4], tmpOrg[0]+rad, tmpOrg[1]-rad, tmpOrg[2]-rad);
	VectorSet(v[5], tmpOrg[0]+rad, tmpOrg[1]-rad, tmpOrg[2]+rad);
	VectorSet(v[6], tmpOrg[0]+rad, tmpOrg[1]+rad, tmpOrg[2]-rad);
	VectorSet(v[7], tmpOrg[0]+rad, tmpOrg[1]+rad, tmpOrg[2]+rad);


	qglBegin(GL_TRIANGLE_FAN);
	qglVertex3fv(v[4]);
	qglVertex3fv(v[0]);
	qglVertex3fv(v[1]);
	qglVertex3fv(v[5]);
	qglVertex3fv(v[7]);
	qglVertex3fv(v[6]);
	qglVertex3fv(v[2]);
	qglVertex3fv(v[0]);
	qglEnd();

	qglBegin(GL_TRIANGLE_FAN);
	qglVertex3fv(v[3]);
	qglVertex3fv(v[0]);
	qglVertex3fv(v[1]);
	qglVertex3fv(v[5]);
	qglVertex3fv(v[7]);
	qglVertex3fv(v[6]);
	qglVertex3fv(v[2]);
	qglVertex3fv(v[0]);
	qglEnd();
	
	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	qglDisable(GL_LINE_SMOOTH);
	// draw small light box
	VectorSet(v[0], tmpOrg[0]-5, tmpOrg[1]-5, tmpOrg[2]-5);
	VectorSet(v[1], tmpOrg[0]-5, tmpOrg[1]-5, tmpOrg[2]+5);
	VectorSet(v[2], tmpOrg[0]-5, tmpOrg[1]+5, tmpOrg[2]-5);
	VectorSet(v[3], tmpOrg[0]-5, tmpOrg[1]+5, tmpOrg[2]+5);
	VectorSet(v[4], tmpOrg[0]+5, tmpOrg[1]-5, tmpOrg[2]-5);
	VectorSet(v[5], tmpOrg[0]+5, tmpOrg[1]-5, tmpOrg[2]+5);
	VectorSet(v[6], tmpOrg[0]+5, tmpOrg[1]+5, tmpOrg[2]-5);
	VectorSet(v[7], tmpOrg[0]+5, tmpOrg[1]+5, tmpOrg[2]+5);


	qglBegin(GL_TRIANGLE_FAN);
	qglVertex3fv(v[4]);
	qglVertex3fv(v[0]);
	qglVertex3fv(v[1]);
	qglVertex3fv(v[5]);
	qglVertex3fv(v[7]);
	qglVertex3fv(v[6]);
	qglVertex3fv(v[2]);
	qglVertex3fv(v[0]);
	qglEnd();

	qglBegin(GL_TRIANGLE_FAN);
	qglVertex3fv(v[3]);
	qglVertex3fv(v[0]);
	qglVertex3fv(v[1]);
	qglVertex3fv(v[5]);
	qglVertex3fv(v[7]);
	qglVertex3fv(v[6]);
	qglVertex3fv(v[2]);
	qglVertex3fv(v[0]);
	qglEnd();

	qglColor3f(1,1,1);
	qglEnable(GL_DEPTH_TEST);
}
	qglColor3f(1.0, 1.0, 1.0);
	qglEnable(GL_TEXTURE_2D);
	qglEnable(GL_CULL_FACE);
	qglEnable(GL_BLEND);
	qglEnable(GL_STENCIL_TEST);

}

void Clamp2RGB(vec3_t color)
{
	if (color[0] > 1.0)
		color[0] = 1.0;
	if (color[0] < 0.0)
		color[0] = 0.0;

	if (color[1] > 1.0)
		color[1] = 1.0;
	if (color[1] < 0.0)
		color[1] = 0.0;

	if (color[2] > 1.0)
		color[2] = 1.0;
	if (color[2] < 0.0)
		color[2] = 0.0;
}



void CreateNormal(vec3_t dst, vec3_t xyz0, vec3_t xyz1, vec3_t xyz2)
{
	float x10,y10,z10;
	float x20,y20,z20;

	x10=xyz1[0]-xyz0[0];
	x20=xyz2[0]-xyz0[0];
	y10=xyz1[1]-xyz0[1];
	y20=xyz2[1]-xyz0[1];
	z10=xyz1[2]-xyz0[2];
	z20=xyz2[2]-xyz0[2];
	dst[0]=y10*z20-y20*z10;
	dst[1]=z10*x20-z20*x10;
	dst[2]=x10*y20-x20*y10;
	VectorNormalize(dst);
}


void MakeFrustum4Light(worldShadowLight_t *light, qboolean ingame)
{
	vec3_t		v0, v1, v2, v3, v4;
	vec3_t		forward, right, up;
	vec3_t		angles, rspeed;

	if (!light->_cone)
		return;	// ”йдем, если фрустум не надо рассчитывать (не задан параметр _cone)

	if(ingame)
		VectorCopy(light->speed, rspeed);
	else
		VectorClear(rspeed);

	angles[0] = light->angles[0] + rspeed[0] * r_newrefdef.time * 1000;
	angles[1] = light->angles[1] + rspeed[1] * r_newrefdef.time * 1000;
	angles[2] = light->angles[2] + rspeed[2] * r_newrefdef.time * 1000;

	AngleVectors (angles, forward, right, up);

	VectorScale(right, light->_cone, right);
	VectorScale(up, light->_cone, up);
	VectorCopy(light->origin, v0);

	v1[0] = v0[0] + (forward[0] - right[0] - up[0]);
	v1[1] = v0[1] + (forward[1] - right[1] - up[1]);
	v1[2] = v0[2] + (forward[2] - right[2] - up[2]);

	v2[0] = v0[0] + (forward[0] - right[0] + up[0]);
	v2[1] = v0[1] + (forward[1] - right[1] + up[1]);
	v2[2] = v0[2] + (forward[2] - right[2] + up[2]);

	v3[0] = v0[0] + (forward[0] + right[0] + up[0]);
	v3[1] = v0[1] + (forward[1] + right[1] + up[1]);
	v3[2] = v0[2] + (forward[2] + right[2] + up[2]);

	v4[0] = v0[0] + (forward[0] + right[0] - up[0]);
	v4[1] = v0[1] + (forward[1] + right[1] - up[1]);
	v4[2] = v0[2] + (forward[2] + right[2] - up[2]);

	CreateNormal(light->frust[0].normal, v1, v2, v0);
	light->frust[0].dist = DotProduct(light->frust[0].normal, v0);

	CreateNormal(light->frust[1].normal, v2, v3, v0);
	light->frust[1].dist = DotProduct(light->frust[1].normal, v0);

	CreateNormal(light->frust[2].normal, v3, v4, v0);
	light->frust[2].dist = DotProduct(light->frust[2].normal, v0);

	CreateNormal(light->frust[3].normal, v4, v1, v0);
	light->frust[3].dist = DotProduct(light->frust[3].normal, v0);
}


worldShadowLight_t *R_AddNewWorldLight(vec3_t origin, vec3_t color, float radius, int style, 
									   int filter, vec3_t angles, vec3_t speed, qboolean isStatic, 
									   int isShadow, int isAmbient, float cone) {
	
	worldShadowLight_t *light;
	int i, leafnum, cluster;
	mat3_t lightAxis;
	vec3_t tmp;

	light = (worldShadowLight_t*)malloc(sizeof(worldShadowLight_t));
	light->s_next = shadowLight_static;
	shadowLight_static = light;
	
	VectorClear(light->frust[0].normal);
	VectorClear(tmp);
	for (i=0; i<8; i++)
		VectorClear(light->corners[i]);

	VectorCopy(origin, light->origin);
	VectorCopy(color, light->startColor);
	VectorCopy(angles, light->angles);
	VectorCopy(speed, light->speed);

	Clamp2RGB(light->color);

	light->radius = radius;
	light->_cone = cone;
	light->isStatic = isStatic;
	light->isShadow = isShadow;
	light->isAmbient = isAmbient;
	light->isNoWorldModel = 0;
	light->next = NULL;
	light->style = style;
	light->filter = filter;
	light->vboId = light->iboId = light->iboNumIndices = 0;
	light->depthBounds[0] = 0.0;
	light->depthBounds[1] = 1.0;

	for (i = 0; i < 3; i++) {
		light->mins[i] = light->origin[i] - light->radius;
		light->maxs[i] = light->origin[i] + light->radius;

		light->mins_cone[i] = light->origin[i] - light->radius;
		light->maxs_cone[i] = light->origin[i] + light->radius;

	}

	 for (i=0; i<8; i++) {
		tmp[0] = (i & 1) ? -light->radius : light->radius;
		tmp[1] = (i & 2) ? -light->radius : light->radius;
		tmp[2] = (i & 4) ? -light->radius : light->radius;

		AnglesToMat3(light->angles, lightAxis);
		Mat3_TransposeMultiplyVector(lightAxis, tmp, light->corners[i]);
		VectorAdd(light->corners[i], light->origin, light->corners[i]);
	}

	MakeFrustum4Light(light, false);

	//// simple cull info for new light 
	leafnum = CM_PointLeafnum(light->origin);
	cluster = CM_LeafCluster(leafnum);
	light->area = CM_LeafArea(leafnum);
	Q_memcpy(light->vis, CM_ClusterPVS(cluster), (CM_NumClusters() + 7) >> 3);
	light->occQ = numLightQ;
	numLightQ++;

	r_numWorlsShadowLights++;
	return light;
}

model_t *loadmodel;
qboolean FoundReLight;

// From bsp we load only model lights 
// like light_mine and light with styles

void Load_BspLights() {
	
	int addLight, style, numlights, addLight_mine, numLightStyles, filter;
	char *c, *token, key[256], *value;
	float color[3], origin[3], radius, cone;

	if(!loadmodel) {
		Com_Printf("No map loaded.\n");
		return;
	}

	c = CM_EntityString();
	numlights = 0;
	numLightStyles = 0;

	while(1) {
		token = COM_Parse(&c);
		if(!c)
			break;

		VectorClear(origin);
		VectorClear(color);
		radius = 0;
		style = 0;
		filter = 0;
		cone = 0;

		addLight = false;
		addLight_mine = false;

		while(1) {
			token = COM_Parse(&c);
			if(token[0] == '}')
				break;

			strncpy(key, token, sizeof(key)-1);

			value = COM_Parse(&c);
			if(!Q_stricmp(key, "classname")) {
				if(!Q_stricmp(value, "light"))
					addLight = true;
				if(!Q_stricmp(value, "light_mine1")){
					addLight = true;
				}
				if(!Q_stricmp(value, "light_mine2")){
					addLight = true;
				}
			}

			if(!Q_stricmp(key, "light"))
				radius = atof(value);
			else if(!Q_stricmp(key, "origin"))
				sscanf(value, "%f %f %f", &origin[0], &origin[1], &origin[2]);
			else if(!Q_stricmp(key, "_color"))
				sscanf(value, "%f %f %f", &color[0], &color[1], &color[2]);
			else if(!Q_stricmp(key, "style"))
				style = atoi(value);
			else if(!Q_stricmp(key, "_cone"))
				cone = atoi(value);
		}

		if(addLight) {
			if(style > 31 || style > 0 && style < 12 || cone){
			R_AddNewWorldLight(origin, color, radius, style, 0, vec3_origin, vec3_origin, 1, 1, 0, cone);
			numlights++;
			}
		}
	}
	Com_Printf("loaded %i bsp lights whith styles or cone\n", numlights);

}

void Load_LightFile() {
	
	int		style, numLights = 0, filter, shadow, ambient;
	vec3_t	angles, speed, color, origin, lOrigin;
	char	*c, *token, key[256], *value;
	float	radius, cone;
	char	name[MAX_QPATH], path[MAX_QPATH];

	if(!loadmodel) {
		Com_Printf("No map loaded.\n");
		return;
	}

	FS_StripExtension(loadmodel->name, name, sizeof (name));
	Com_sprintf(path, sizeof(path),"%s.xplit", name);
	FS_LoadFile (path, (void **)&c);

	if(!c){
		FoundReLight = false;
		Load_BspLights();
		return;
	}
	
	FoundReLight = true;
	
	Com_Printf("Load lights from "S_COLOR_GREEN"%s"S_COLOR_WHITE".\n", path);

	while(1) {
		token = COM_Parse(&c);
		if(!c)
			break;

		radius = 0;
		style = 0;
		filter = 0;
		shadow = 0;
		ambient = 0;
		cone = 0;

		VectorClear(angles);
		VectorClear(speed);
		VectorClear(origin);
		VectorClear(lOrigin);
		VectorClear(color);

		while(1) {
			token = COM_Parse(&c);
			if(token[0] == '}')
				break;

			strncpy(key, token, sizeof(key)-1);

			value = COM_Parse(&c);
			
			if(!Q_stricmp(key, "radius"))
				radius = atof(value);
			else if(!Q_stricmp(key, "origin"))
				sscanf(value, "%f %f %f", &origin[0], &origin[1], &origin[2]);
			else if(!Q_stricmp(key, "color"))
				sscanf(value, "%f %f %f", &color[0], &color[1], &color[2]);
			else if(!Q_stricmp(key, "style"))
				style = atoi(value);
			else if(!Q_stricmp(key, "filter"))
				filter = atoi(value);
			else if(!Q_stricmp(key, "angles"))
				sscanf(value, "%f %f %f", &angles[0], &angles[1], &angles[2]);
			else if(!Q_stricmp(key, "speed"))
				sscanf(value, "%f %f %f", &speed[0], &speed[1], &speed[2]);
			else if(!Q_stricmp(key, "shadow"))
				shadow = atoi(value);
			else if(!Q_stricmp(key, "ambient"))
				ambient = atoi(value);
			else if(!Q_stricmp(key, "_cone"))
				cone = atoi(value);
		}
	
		R_AddNewWorldLight(origin, color, radius, style, filter, angles, speed, 1, shadow, ambient, cone);
		numLights++;
		}
	Com_Printf(""S_COLOR_MAGENTA"Load_LightFile:"S_COLOR_WHITE" add "S_COLOR_GREEN"%i"S_COLOR_WHITE" world lights\n", numLights);
}

/*
===============
R_MarkLightLeaves

Marks nodes from the light, this is used for
gross culling during svbsp creation.
===============
*/
qboolean R_MarkLightLeaves (worldShadowLight_t *light)
{
	int contents, leafnum, cluster;
	int		leafs[MAX_MAP_LEAFS];
	int		i, count;
	vec3_t	mins, maxs;
	byte	vis[MAX_MAP_LEAFS/8];

	contents = CL_PMpointcontents(light->origin);
	if (contents & CONTENTS_SOLID)
		goto skip;

	leafnum = CM_PointLeafnum (light->origin);
	cluster = CM_LeafCluster (leafnum);
	light->area = CM_LeafArea (leafnum);

	if(!light->area)
	{
skip:	Com_DPrintf("Out of BSP, rejected light at %f %f %f\n", light->origin[0], light->origin[1], light->origin[2]);
		return false;
	}

	// build vis-data
	memcpy (&light->vis, CM_ClusterPVS(cluster), (((CM_NumClusters()+31)>>5)<<2));

	for (i=0 ; i<3 ; i++)
	{
		mins[i] = light->origin[i] - light->radius;
		maxs[i] = light->origin[i] + light->radius;
	}

	count = CM_BoxLeafnums (mins, maxs, leafs, r_worldmodel->numleafs, NULL);
	if (count < 1)
		Com_Error (ERR_FATAL, "R_MarkLightLeaves: count < 1");

	// convert leafs to clusters
	for (i=0 ; i<count ; i++)
		leafs[i] = CM_LeafCluster(leafs[i]);

	memset(&vis, 0, (((r_worldmodel->numleafs+31)>>5)<<2));
	for (i=0 ; i<count ; i++)
		vis[leafs[i]>>3] |= (1<<(leafs[i]&7));

	for (i=0 ; i<((r_worldmodel->numleafs+31)>>5); i++)
		((long *)light->vis)[i] &= ((long *)vis)[i];

	return true;
}

qboolean InLightVISEntity()
{
	int		leafs[MAX_MAP_LEAFS];
	int		i, count;
	int		longs;
	vec3_t	mins, maxs;
		
	if(r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return true;

	if (!r_worldmodel)
		return false;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	}
	else
	{
	VectorAdd(currententity->origin, currententity->model->maxs, maxs);
	VectorAdd(currententity->origin, currententity->model->mins, mins);
	}

	count = CM_BoxLeafnums (mins, maxs, leafs, r_worldmodel->numleafs, NULL);
	if (count < 1)
		Com_Error (ERR_FATAL, "InLightVISEntity: count < 1");
	longs = (CM_NumClusters()+31)>>5;

	// convert leafs to clusters
	for (i=0 ; i<count ; i++)
		leafs[i] = CM_LeafCluster(leafs[i]);

	memset(&currententity->vis, 0, (((r_worldmodel->numleafs+31)>>5)<<2));
	for (i=0 ; i<count ; i++)
		currententity->vis[leafs[i]>>3] |= (1<<(leafs[i]&7));

	return HasSharedLeafs (currentShadowLight->vis, currententity->vis);
}

void R_DrawBspModelVolumes(qboolean precalc);

void CalcLightVis(void){
	worldShadowLight_t *light;

		for(light = shadowLight_static; light; light = light->s_next) {

			if(!R_MarkLightLeaves(light))
				continue;
		}
}

void DeleteShadowVertexBuffers(void){
	worldShadowLight_t *light;

		for(light = shadowLight_static; light; light = light->s_next) {
			
			qglDeleteBuffers(1, &light->vboId);
			qglDeleteBuffers(1, &light->iboId);
			light->vboId = light->iboId = light->iboNumIndices = 0;
		}
}


void CleanDuplicateLights(void){

	worldShadowLight_t *light1, *light2, *plug;
	vec3_t tmp;

	if(FoundReLight)
		return;

	for(light1 = shadowLight_static; light1; light1 = light1->s_next) {
	
		if(light1->style || light1->_cone)
			continue;

	for(light2 = light1->s_next; light2; light2 = plug) {
		
		plug = light2->s_next;

		if(light2->style || light2->_cone)
			continue;

		VectorSubtract(light2->origin, light1->origin, tmp);
  
		if ((VectorLength(tmp) < r_lightsWeldThreshold->value)){
						
			VectorAdd(light1->origin, light2->origin, tmp);
			VectorScale(tmp, 0.5f, light1->origin);
			DeleteCurrentLight(light2);
			
		}
	}
}
}


void R_ClearWorldLights(void)
{
	worldShadowLight_t *light, *next;

	if(shadowLight_static) {
		for(light = shadowLight_static; light; light = next) {
			next = light->s_next;
			free(light);
		}
		shadowLight_static = NULL;
	}

	memset(shadowLightsBlock, 0, sizeof(worldShadowLight_t) * MAX_WORLD_SHADOW_LIHGTS);

	r_numWorlsShadowLights = 0;
}

/*
=============
GL_SetupCubeMapMatrix

Loads the current matrix with a tranformation used for light filters
=============
*/
void GL_SetupCubeMapMatrix(qboolean model)
{
	float	a, b, c;

	qglMatrixMode(GL_TEXTURE);
	qglLoadIdentity();

	a = currentShadowLight->angles[2] + (currentShadowLight->speed[2] * r_newrefdef.time * 1000);
	b = currentShadowLight->angles[0] + (currentShadowLight->speed[0] * r_newrefdef.time * 1000);
	c = currentShadowLight->angles[1] + (currentShadowLight->speed[1] * r_newrefdef.time * 1000);

	if (a)
		qglRotatef ( -a,  1, 0, 0);
	if (b)
		qglRotatef ( -b,  0, 1, 0);
	if (c)
		qglRotatef ( -c,  0, 0, 1);

	if (model){

	if (currententity->angles[1])
		qglRotatef (currententity->angles[1],  0, 0, 1);
	if (currententity->angles[0])
		qglRotatef (currententity->angles[0],  0, 1, 0);		/// fixed "stupid quake bug" ;)
	if (currententity->angles[2])
		qglRotatef (currententity->angles[2],  1, 0, 0);
	
	}
	qglTranslatef(-currentShadowLight->origin[0], -currentShadowLight->origin[1], -currentShadowLight->origin[2]);
	qglMatrixMode(GL_MODELVIEW);
}


qboolean R_DrawLightOccluders()
{
	vec3_t	v[8];
	vec3_t	tmpOrg;
	float	radius;
	int		sampleCount;

	if(!r_useLightOccluders->value)
		return true;

	
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return true;
	
	
	if(BoundsAndSphereIntersect (currentShadowLight->mins, currentShadowLight->maxs, r_origin, 0)){
	//	glBeginConditionalRender(lightsQueries[currentShadowLight->occQ], GL_QUERY_NO_WAIT);
		return true;
	}
	qglColorMask(0,0,0,0);
	qglDisable(GL_TEXTURE_2D);
	qglDisable(GL_CULL_FACE);
	qglDisable(GL_BLEND);
	qglDisable(GL_STENCIL_TEST);
	
	VectorCopy(currentShadowLight->origin, tmpOrg);
	radius = currentShadowLight->radius * r_occLightBoundsSize->value;

	VectorSet(v[0], tmpOrg[0]-radius, tmpOrg[1]-radius, tmpOrg[2]-radius);
	VectorSet(v[1], tmpOrg[0]-radius, tmpOrg[1]-radius, tmpOrg[2]+radius);
	VectorSet(v[2], tmpOrg[0]-radius, tmpOrg[1]+radius, tmpOrg[2]-radius);
	VectorSet(v[3], tmpOrg[0]-radius, tmpOrg[1]+radius, tmpOrg[2]+radius);
	VectorSet(v[4], tmpOrg[0]+radius, tmpOrg[1]-radius, tmpOrg[2]-radius);
	VectorSet(v[5], tmpOrg[0]+radius, tmpOrg[1]-radius, tmpOrg[2]+radius);
	VectorSet(v[6], tmpOrg[0]+radius, tmpOrg[1]+radius, tmpOrg[2]-radius);
	VectorSet(v[7], tmpOrg[0]+radius, tmpOrg[1]+radius, tmpOrg[2]+radius);

	currentShadowLight->occ_frame = r_visframecount;
	qglBeginQueryARB(gl_state.query_passed, lightsQueries[currentShadowLight->occQ]);

	qglBegin(GL_TRIANGLE_FAN);
	qglVertex3fv(v[4]);
	qglVertex3fv(v[0]);
	qglVertex3fv(v[1]);
	qglVertex3fv(v[5]);
	qglVertex3fv(v[7]);
	qglVertex3fv(v[6]);
	qglVertex3fv(v[2]);
	qglVertex3fv(v[0]);
	qglEnd();

	qglBegin(GL_TRIANGLE_FAN);
	qglVertex3fv(v[3]);
	qglVertex3fv(v[0]);
	qglVertex3fv(v[1]);
	qglVertex3fv(v[5]);
	qglVertex3fv(v[7]);
	qglVertex3fv(v[6]);
	qglVertex3fv(v[2]);
	qglVertex3fv(v[0]);
	qglEnd();
	
	qglEndQueryARB(gl_state.query_passed);

	qglColorMask(1,1,1,1);
	qglEnable(GL_TEXTURE_2D);
	qglEnable(GL_CULL_FACE);
	qglEnable(GL_BLEND);
	qglEnable(GL_STENCIL_TEST);


//	glBeginConditionalRender(lightsQueries[currentShadowLight->occQ], GL_QUERY_WAIT);
	if(currentShadowLight->occ_frame == lightVissFrame - 1)
		qglGetQueryObjectivARB(lightsQueries[currentShadowLight->occQ], GL_QUERY_RESULT_ARB, &sampleCount);

	if (!sampleCount) 
		return false;
	else 
		return true;

//		return false;
}