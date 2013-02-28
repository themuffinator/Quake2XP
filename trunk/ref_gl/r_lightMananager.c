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
worldShadowLight_t shadowLightsBlock[MAX_WORLD_SHADOW_LIHGTS];

static int num_dlits;
int num_nwmLights;
int num_visLights;
int numLightQ;
static int numCulledLights;
vec3_t player_org, v_forward, v_right, v_up;
trace_t CL_PMTraceWorld(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int mask);

qboolean R_CheckSharedArea(vec3_t p1, vec3_t p2)
{
	int leafnum;
	int cluster;
	int area1, area2;
	byte *mask;

	leafnum = CM_PointLeafnum(p1);
	cluster = CM_LeafCluster(leafnum);
	area1 = CM_LeafArea(leafnum);
	mask = CM_ClusterPVS(cluster);

	leafnum = CM_PointLeafnum(p2);
	cluster = CM_LeafCluster(leafnum);
	area2 = CM_LeafArea(leafnum);
	if (!CM_AreasConnected(area1, area2))
		return false;			
	return true;
}

qboolean R_CullLight(worldShadowLight_t *light) {
	
	vec3_t mins, maxs, none = {1, 1, 1};

	if (r_newrefdef.areabits){
		if (!(r_newrefdef.areabits[light->area >> 3] & (1 << (light->area & 7)))){
			return true;
		}
	}
	
	if(!R_CheckSharedArea(light->origin, r_origin))
		return true;

	if(CL_PMpointcontents(light->origin) & MASK_SOLID)
		return true;
	
	if(!HasSharedLeafs(light->vis, viewvis))
		return true;

	VectorMA(light->origin,  light->radius, none, maxs);
	VectorMA(light->origin, -light->radius, none, mins);

	if(R_CullBox(mins, maxs))
		return true;

	return false;

	}

void R_AddDynamicLight(dlight_t *dl) {
	
	worldShadowLight_t *light;
	vec3_t mins, maxs, none = {1, 1, 1};

	VectorMA(dl->origin, dl->intensity, none, maxs);
	VectorMA(dl->origin, -dl->intensity, none, mins);

	if(R_CullBox(mins, maxs)){
		numCulledLights++;
		return;
	}

	light = &shadowLightsBlock[num_dlits++];
	memset(light, 0, sizeof(worldShadowLight_t));
	light->next = shadowLight_frame;
	shadowLight_frame = light;

	VectorCopy(dl->origin, light->origin);
	VectorCopy(dl->color, light->startColor);
	light->style = 0;
	light->filter = 0;
	light->radius = dl->intensity;
	light->isStatic = 0;
	light->isNoWorldModel = 0;
	light->isShadow = 1;
}

void R_AddNoWorldModelLight() {
	
	worldShadowLight_t *light;

	light = &shadowLightsBlock[num_nwmLights++];
	memset(light, 0, sizeof(worldShadowLight_t));
	light->next = shadowLight_frame;
	shadowLight_frame = light;

	VectorSet(light->origin, -100, 100, 76);
	VectorSet(light->startColor, 1.0, 1.0, 1.0);
	light->radius = 1024;
	VectorSet(light->mins, -1024, -1024, -1024);
	VectorSet(light->maxs,  1024,  1024,  1024);
	light->style = 0;
	light->filter = 0;
	light->isStatic = 0;
	light->isShadow = 0;
	light->isNoWorldModel = 1;
}

void R_PrepareShadowLightFrame(void) {
	
	int i;
	worldShadowLight_t *light;
	
	num_dlits = 0;
	num_nwmLights = 0;
	shadowLight_frame = NULL;

	numCulledLights = 0;
	// add pre computed lights
	if(shadowLight_static) {
		for(light = shadowLight_static; light; light = light->s_next) {
			
			if(R_CullLight(light)){
				numCulledLights++;
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

	if(r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		R_AddNoWorldModelLight();

	Com_DPrintf("%i lights was culled\n", numCulledLights);
	
	if(!shadowLight_frame) 
		return;
		
	for(light = shadowLight_frame; light; light = light->next) {
		
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
		
		light->mins[0] = light->origin[0] - light->radius;
		light->mins[1] = light->origin[1] - light->radius;
		light->mins[2] = light->origin[2] - light->radius;
		light->maxs[0] = light->origin[0] + light->radius;
		light->maxs[1] = light->origin[1] + light->radius;
		light->maxs[2] = light->origin[2] + light->radius;
	}

}

#define Q_clamp(a,b,c) ((b) >= (c) ? (a)=(b) : (a) < (b) ? (a)=(b) : (a) > (c) ? (a)=(c) : (a))

#define RgbClamp(a) ((a) < (0) ? (a)=(0) : (a) > (1) ? (a)=(1) : (a))

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
		fprintf(f, "}\n");
		i++;
	}

	fclose(f);
	
		Com_Printf(""S_COLOR_MAGENTA"SaveLights_f: "S_COLOR_WHITE"Save lights to "S_COLOR_GREEN"%s.xplit\n"S_COLOR_WHITE"Save "S_COLOR_GREEN"%i"S_COLOR_WHITE" lights\n", name, i);

}

static void DeleteCurrentLight(worldShadowLight_t *l) {
	worldShadowLight_t *light;

	if(l == shadowLight_static) 
		shadowLight_static = l->s_next;
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
		R_AddNewWorldLight(spawn, color, 300, 0, 0, vec3_origin, vec3_origin, 1, 1);
	}
}

void R_Light_SpawnToCamera_f(void) {
	vec3_t color = {1.0, 1.0, 1.0};

	if(!r_lightEditor->value){
		Com_Printf("Type r_lightEditor 1 to enable light editing.\n");
		return;
	}

	R_AddNewWorldLight(player_org, color, 300, 0, 0, vec3_origin, vec3_origin, 1, 1);

}

void R_Light_Copy_f(void) {
	vec3_t color, spawn, origin, angles, speed;
	float radius;
	int style, filter, shadow;

	if(!r_lightEditor->value){
		Com_Printf("Type r_lightEditor 1 to enable light editing.\n");
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

	VectorMA(origin, -50, v_forward, spawn);
	selectedShadowLight = R_AddNewWorldLight(spawn, color, radius, style, filter, angles, vec3_origin, 1, shadow);
	
}

void R_EditSelectedLight_f(void) {
	
	vec3_t color, origin, angles, speed;
	float radius;
	int style, filter, shadow;
	
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
	else{
		 Com_Printf("Light Properties: Origin: %.4f %.4f %.4f\nColor: %.4f %.4f %.4f\nRadius %.1f\nStyle %i\nFilter Cube %i\nAngles: %.4f %.4f %.4f\nSpeed: %.4f %.4f %.4f\nShadows %i",
		 selectedShadowLight->origin[0], selectedShadowLight->origin[1], selectedShadowLight->origin[2],
		 selectedShadowLight->color[0], selectedShadowLight->color[1], selectedShadowLight->color[2], 
		 selectedShadowLight->radius,
		 selectedShadowLight->style,
		 selectedShadowLight->filter,
		 selectedShadowLight->angles[0], selectedShadowLight->angles[1], selectedShadowLight->angles[2],
		 selectedShadowLight->speed[0], selectedShadowLight->speed[1], selectedShadowLight->speed[2],
		 selectedShadowLight->isShadow);

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

worldShadowLight_t *R_AddNewWorldLight(vec3_t origin, vec3_t color, float radius, int style, 
									   int filter, vec3_t angles, vec3_t speed, qboolean isStatic, 
									   int isShadow) {
	
	worldShadowLight_t *light;
	int leafnum;
	int cluster;
	
	light = (worldShadowLight_t*)malloc(sizeof(worldShadowLight_t));
	light->s_next = shadowLight_static;
	shadowLight_static = light;

	VectorCopy(origin, light->origin);
	VectorCopy(color, light->startColor);
	VectorCopy(angles, light->angles);
	VectorCopy(speed, light->speed);

	Clamp2RGB(light->color);

	light->radius = radius;
	light->isStatic = isStatic;
	light->isShadow = isShadow;
	light->isNoWorldModel = false;
	light->next = NULL;
	light->style = style;
	light->filter = filter;
		
	//// cull info
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
	float color[3], origin[3], radius;

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
		}

		if(addLight) {
			if(style > 31 || style > 0 && style < 12){
			R_AddNewWorldLight(origin, color, radius, style, 0, vec3_origin, vec3_origin, 1, 1);
			numlights++;
			}
		}
	}
	Com_Printf("loaded %i bsp lights whith styles\n", numlights);

}

void Load_LightFile() {
	
	int		style, numLights = 0, filter, shadow;
	vec3_t	angles, speed, color, origin, lOrigin;
	char	*c, *token, key[256], *value;
	float	radius;
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
		}
	
		R_AddNewWorldLight(origin, color, radius, style, filter, angles, speed, 1, shadow);
		numLights++;
		}
	Com_Printf(""S_COLOR_MAGENTA"Load_LightFile:"S_COLOR_WHITE" add "S_COLOR_GREEN"%i"S_COLOR_WHITE" world lights\n", numLights);
}


/*====================================
AVERAGE LIGHTS. 
BUT NEVER REMOVE THE LIGHT WITH STYLE!
=====================================*/

void CleanDuplicateLights(void){

	worldShadowLight_t *light1, *light2, *plug;
	vec3_t tmp;

	if(FoundReLight)
		return;

	for(light1 = shadowLight_static; light1; light1 = light1->s_next) {
	
		if(light1->style)
			continue;

	for(light2 = light1->s_next; light2; light2 = plug) {
		
		plug = light2->s_next;

		if(light2->style)
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

qboolean R_DrawLightOccluders()
{
	vec3_t	v[8], org;
	float	rad;
	int		sampleCount;
	
	// direct visible, player in light bounds
	if(!r_useConditionalRender->value){
	if(BoundsAndSphereIntersect (currentShadowLight->mins, currentShadowLight->maxs, r_origin, 25))
		return true;
	}

	qglDisable (GL_CULL_FACE);
	qglDisable(GL_TEXTURE_2D);
	qglDepthMask(0);
	qglColorMask(0,0,0,0);
	
	// begin occlusion test
	qglBeginQueryARB(gl_state.query_passed, lightsQueries[currentShadowLight->occQ]);

	VectorCopy(currentShadowLight->origin, org);
	rad = currentShadowLight->radius * r_occLightBoundsSize->value;

	VectorSet(v[0], org[0]-rad, org[1]-rad, org[2]-rad);
	VectorSet(v[1], org[0]-rad, org[1]-rad, org[2]+rad);
	VectorSet(v[2], org[0]-rad, org[1]+rad, org[2]-rad);
	VectorSet(v[3], org[0]-rad, org[1]+rad, org[2]+rad);
	VectorSet(v[4], org[0]+rad, org[1]-rad, org[2]-rad);
	VectorSet(v[5], org[0]+rad, org[1]-rad, org[2]+rad);
	VectorSet(v[6], org[0]+rad, org[1]+rad, org[2]-rad);
	VectorSet(v[7], org[0]+rad, org[1]+rad, org[2]+rad);


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

	// end occlusion test
	qglEndQueryARB(gl_state.query_passed);

	qglEnable(GL_CULL_FACE);
	qglEnable(GL_TEXTURE_2D);
	qglDepthMask(1);
	qglColorMask(1,1,1,1);

	if(gl_state.conditional_render && r_useConditionalRender->value){
	// player in light bounds draw light in any case
	if(BoundsAndSphereIntersect (currentShadowLight->mins, currentShadowLight->maxs, r_origin, 25))
		glBeginConditionalRender(lightsQueries[currentShadowLight->occQ], GL_QUERY_NO_WAIT);
	else
		glBeginConditionalRender(lightsQueries[currentShadowLight->occQ], GL_QUERY_WAIT);
	
	return true;
	}
	else
	{
	qglGetQueryObjectivARB(lightsQueries[currentShadowLight->occQ], GL_QUERY_RESULT_ARB, &sampleCount);

	if (!sampleCount) 
		return false;
	else 
		return true;
	}	
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

