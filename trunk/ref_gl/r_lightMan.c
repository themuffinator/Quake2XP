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
	
	float c;
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


	c = (light->sColor[0] + light->sColor[1] + light->sColor[2]) * light->radius*(1.0/3.0);
		if(c < 0.1)
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
	float c;

	c = (dl->color[0] + dl->color[1] + dl->color[2]) * dl->intensity*(1.0/3.0);
	if(c < 0.1)
		return;

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
	VectorCopy(dl->color, light->sColor);
	light->style = 0;
	light->radius = dl->intensity;
	light->isStatic = false;
	light->isNoWorldModel = false;
}

void R_AddNoWorldModelLight() {
	
	worldShadowLight_t *light;

	light = &shadowLightsBlock[num_nwmLights++];
	memset(light, 0, sizeof(worldShadowLight_t));
	light->next = shadowLight_frame;
	shadowLight_frame = light;

	VectorSet(light->origin, -100, 100, 76);
	VectorSet(light->sColor, 1.0, 1.0, 1.0);
	light->radius = 1024;
	VectorSet(light->mins, -1024, -1024, -1024);
	VectorSet(light->maxs,  1024,  1024,  1024);
	light->style = 0;
	light->isStatic = false;
	light->isShadow = false;
	light->isNoWorldModel = true;
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
		
		VectorCopy(light->sColor, light->color);
		
		if(!(r_newrefdef.rdflags & RDF_NOWORLDMODEL)){
		light->color[0] *= r_newrefdef.lightstyles[light->style].rgb[0];
		light->color[1] *= r_newrefdef.lightstyles[light->style].rgb[1];
		light->color[2] *= r_newrefdef.lightstyles[light->style].rgb[2];
		}

		light->mins[0] = light->origin[0] - light->radius;
		light->mins[1] = light->origin[1] - light->radius;
		light->mins[2] = light->origin[2] - light->radius;
		light->maxs[0] = light->origin[0] + light->radius;
		light->maxs[1] = light->origin[1] + light->radius;
		light->maxs[2] = light->origin[2] + light->radius;
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

void SaveLights_f(void) {
	
	char				name[MAX_QPATH], path[MAX_QPATH];
	FILE				*f;
	int					i=0;
	
	if(!shadowLight_static) 
		return;

	FS_StripExtension(r_worldmodel->name, name, sizeof (name));
	Com_sprintf(path, sizeof(path),"%s/%s.xplit", FS_Gamedir(), name);
	
//	if (!strcmp(Cmd_Argv(1), "clean")){
		remove(path); //remove prev version
//		return;
//	}

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
		fprintf(f, "\"color\" \"%f %f %f\"\n",	currentShadowLight->sColor[0], currentShadowLight->sColor[1], currentShadowLight->sColor[2]);
		fprintf(f, "\"style\" \"%i\"\n",		(int)currentShadowLight->style);
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

void Light_Spawn_f(void) {
	vec3_t color = {1.0, 1.0, 0.0}, end, spawn;
	trace_t trace;

	if(!r_lightEditor->value)
		return;

	VectorMA(player_org, 1024, v_forward, end);
	trace = CL_PMTraceWorld(player_org, vec3_origin, vec3_origin, end, MASK_SOLID);
	
	if (trace.fraction != 1.0){
		VectorMA(trace.endpos, -10, v_forward, spawn);
		R_AddNewWorldLight(spawn, color, 300, 0, true, true, NULL);
	}
}

void Light_SpawnToCamera_f(void) {
	vec3_t color = {1.0, 1.0, 1.0};

	if(!r_lightEditor->value)
		return;

	R_AddNewWorldLight(player_org, color, 300, 0, true, true, NULL);

}

void Light_Copy_f(void) {
	vec3_t color, spawn, origin;
	float radius;
	int style;

	if(!r_lightEditor->value)
		return;

	VectorCopy(selectedShadowLight->origin, origin);
	VectorCopy(selectedShadowLight->color, color);
	radius = selectedShadowLight->radius;
	style = selectedShadowLight->style;

	VectorMA(origin, -50, v_forward, spawn);
	selectedShadowLight = R_AddNewWorldLight(spawn, color, radius, style, true, true, NULL);
	
}

void R_EditSelectedLight_f(void) {
	
	vec3_t color, origin;
	float radius = 10;
	int style = 0;
	
	if(!r_lightEditor->value)
		return;

	if(!selectedShadowLight)
	{
	Com_Printf("No selected light.\n");
		return;
	}

	VectorCopy(selectedShadowLight->origin, origin);
	VectorCopy(selectedShadowLight->color, color);
	radius = selectedShadowLight->radius;
	style = selectedShadowLight->style;
	
	if (!strcmp(Cmd_Argv(1), "origin")) {
		if(Cmd_Argc() != 5) {
			Com_Printf("usage: editLight: %s X Y Z\nCurrent Origin: %.4f %.4f %.4f", Cmd_Argv(0),
				selectedShadowLight->origin[0], 
				selectedShadowLight->origin[1], 
				selectedShadowLight->origin[2]);
			return;
		}
		origin[0] = atof(Cmd_Argv(2));
		origin[1] = atof(Cmd_Argv(3));
		origin[2] = atof(Cmd_Argv(4));
	} 
	else
	if (!strcmp(Cmd_Argv(1), "color")) {
		if(Cmd_Argc() != 5) {
			Com_Printf("usage: editLight: %s Red Green Blue\nCurrent Color: %.4f %.4f %.4f", Cmd_Argv(0),
			selectedShadowLight->color[0], 
			selectedShadowLight->color[1], 
			selectedShadowLight->color[2]);
			return;
		}
		color[0] = atof(Cmd_Argv(2));
		color[1] = atof(Cmd_Argv(3));
		color[2] = atof(Cmd_Argv(4));
	} 
	else
	if (!strcmp(Cmd_Argv(1), "radius")) {
		if(Cmd_Argc() != 3) {
			Com_Printf("usage: editLight: %s value\nCurrent Radius: %.1f\n", Cmd_Argv(0), 
			selectedShadowLight->radius);
			return;
		}
		radius = atof(Cmd_Argv(2));
	 } 
	else
	 if (Cmd_Argc() == 3 && !strcmp(Cmd_Argv(1), "style")) {
		if(Cmd_Argc() != 3) {
			Com_Printf("usage: editLight %s value\nCurrent Style %i", Cmd_Argv(0),
			selectedShadowLight->style);
			return;
		}
		style = atoi(Cmd_Argv(2)); 
	 }
	 else
		 Com_Printf("Light Properties: Origin: %.4f %.4f %.4f\nColor: %.4f %.4f %.4f\nRadius %.1f\nStyle %i\n",
		 selectedShadowLight->origin[0], selectedShadowLight->origin[1], selectedShadowLight->origin[2],
		 selectedShadowLight->color[0], selectedShadowLight->color[1], selectedShadowLight->color[2], 
		 selectedShadowLight->radius,
		 selectedShadowLight->style);
	
	if(style < 0 || style >11)
		style = 0;

	 DeleteCurrentLight(selectedShadowLight);
	 selectedShadowLight = R_AddNewWorldLight(origin, color, radius, style, true, true, NULL);

}

void R_MoveLightToRight_f(void) {
	
	vec3_t color, origin;
	float radius, offset;
	int style;
	
	if(!r_lightEditor->value)
		return;

	if(!selectedShadowLight)
	{
	Com_Printf("No selected light.\n");
		return;
	}

	VectorCopy(selectedShadowLight->origin, origin);
	VectorCopy(selectedShadowLight->color, color);
	radius = selectedShadowLight->radius;
	style = selectedShadowLight->style;

	if( Cmd_Argc() != 2 )
	{
		Com_Printf("Usage: moveLight_right: <value>\n");
		return;
	}

	offset = atof(Cmd_Argv(1)); 
	VectorMA(origin, offset, v_right, origin);
	
	DeleteCurrentLight(selectedShadowLight);
	selectedShadowLight = R_AddNewWorldLight(origin, color, radius, style, true, true, NULL);

}

void R_MoveLightForward_f(void) {
	
	vec3_t color, origin;
	float radius, offset, fix;
	int style;
	
	if(!r_lightEditor->value)
		return;

	if(!selectedShadowLight)
	{
	Com_Printf("No selected light.\n");
		return;
	}

	VectorCopy(selectedShadowLight->origin, origin);
	VectorCopy(selectedShadowLight->color, color);
	radius = selectedShadowLight->radius;
	style = selectedShadowLight->style;

	if( Cmd_Argc() != 2 )
	{
		Com_Printf("Usage: moveLight_forward: <value>\n");
		return;
	}

	offset = atof(Cmd_Argv(1)); 
	fix = origin[2];
	VectorMA(origin, offset, v_forward, origin);
	origin[2] = fix;

	DeleteCurrentLight(selectedShadowLight);
	selectedShadowLight = R_AddNewWorldLight(origin, color, radius, style, true, true, NULL);

}

void R_MoveLightUpDown_f(void) {
	
	vec3_t color, origin;
	float radius, offset;
	int style;
	
	if(!r_lightEditor->value)
		return;

	if(!selectedShadowLight)
	{
	Com_Printf("No selected light.\n");
		return;
	}

	VectorCopy(selectedShadowLight->origin, origin);
	VectorCopy(selectedShadowLight->color, color);
	radius = selectedShadowLight->radius;
	style = selectedShadowLight->style;

	if( Cmd_Argc() != 2 )
	{
		Com_Printf("Usage: moveSelectedLight_right: <value>\n");
		return;
	}

	offset = atof(Cmd_Argv(1)); 
	origin[2] += offset;

	DeleteCurrentLight(selectedShadowLight);
	selectedShadowLight = R_AddNewWorldLight(origin, color, radius, style, true, true, NULL);

}

void R_ChangeLightRadius_f(void) {
	
	vec3_t color, origin;
	float radius, offset;
	int style;
	
	if(!r_lightEditor->value)
		return;

	if(!selectedShadowLight)
	{
	Com_Printf("No selected light.\n");
		return;
	}

	VectorCopy(selectedShadowLight->origin, origin);
	VectorCopy(selectedShadowLight->color, color);
	radius = selectedShadowLight->radius;
	style = selectedShadowLight->style;

	if( Cmd_Argc() != 2 )
	{
		Com_Printf("Usage: ajustSelectedLightRadius: <value>\n");
		return;
	}

	offset = atof(Cmd_Argv(1));
	radius += offset;
	if(radius <10)
		radius = 10;
	DeleteCurrentLight(selectedShadowLight);
	selectedShadowLight = R_AddNewWorldLight(origin, color, radius, style, true, true, NULL);

}

void Light_Delete_f(void) {

	if(!r_lightEditor->value)
		return;
	
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

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if(!r_lightEditor->value)
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

	// check if it is in focus
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

worldShadowLight_t *R_AddNewWorldLight(vec3_t origin, vec3_t color, float radius, int style, qboolean isStatic, qboolean isShadow, msurface_t *surf) {
	
	worldShadowLight_t *light;
	int leafnum;
	int cluster;
	
	light = (worldShadowLight_t*)malloc(sizeof(worldShadowLight_t));
	light->s_next = shadowLight_static;
	shadowLight_static = light;

	VectorCopy(origin, light->origin);
	VectorCopy(color, light->sColor);
	light->radius = radius;
	light->isStatic = isStatic;
	light->isShadow = isShadow;
	light->isNoWorldModel = false;
	light->next = NULL;
	light->style = style;
		
	//// cull info
	light->surf = surf;
	leafnum = CM_PointLeafnum(light->origin);
	cluster = CM_LeafCluster(leafnum);
	light->area = CM_LeafArea(leafnum);
	Q_memcpy(light->vis, CM_ClusterPVS(cluster), (CM_NumClusters() + 7) >> 3);

	light->occQ = numLightQ;
	numLightQ++;

	if(!light->style)
		r_numWorlsShadowLights++;
	return light;
}

model_t *loadmodel;


// From bsp we load only model lights 
// like light_mine and light with styles

void Load_BspLights() {
	
	int addLight, style, numlights, addLight_mine, numLightStyles;
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

		color[0] = 0.5;
		color[1] = 0.5;
		color[2] = 0.5;
		radius = 0;
		origin[0] = 0;
		origin[1] = 0;
		origin[2] = 0;
		addLight = false;
		addLight_mine = false;
		style = 0;

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
					addLight_mine = true;
				}
				if(!Q_stricmp(value, "light_mine2")){
					addLight = true;
					addLight_mine = true;
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
			if((style > 0 && style < 12) || addLight_mine){
			R_AddNewWorldLight(origin, color, radius, style, true, true, NULL);
			numlights++;
			if(style > 0 && style < 12)
				numLightStyles++;
			}
		}
	}
	Com_DPrintf("loaded %i bsp lights whith styles %i\n",numlights, numLightStyles);

}

qboolean FoundReLight;

void Load_LightFile() {
	
	int		style, numLights = 0;
	char	*c, *token, key[256], *value;
	float	color[3], origin[3], radius;
	char	name[MAX_QPATH], path[MAX_QPATH];
	

	if(!loadmodel) {
		Com_Printf("No map loaded.\n");
		return;
	}

	FS_StripExtension(loadmodel->name, name, sizeof (name));
	Com_sprintf(path, sizeof(path),"%s.xplit", name);
	FS_LoadFile (path, (void **)&c);

	if(!c){
		Load_BspLights();
		FoundReLight = false;
		return;
	}
	
	FoundReLight = true;
	
	Com_Printf("Load lights from "S_COLOR_GREEN"%s"S_COLOR_WHITE".\n", path);

	while(1) {
		token = COM_Parse(&c);
		if(!c)
			break;

		color[0] = 1;
		color[1] = 1;
		color[2] = 1;
		radius = 0;
		origin[0] = 0;
		origin[1] = 0;
		origin[2] = 0;
		style = 0;

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
		}
	
		R_AddNewWorldLight(origin, color, radius, style, true, true, NULL);
		numLights++;
		}
	Com_Printf(""S_COLOR_MAGENTA"Load_LightFile:"S_COLOR_WHITE" add "S_COLOR_GREEN"%i"S_COLOR_WHITE" world lights\n", numLights);
}


/*====================================
AVERAGE LIGHTS. 
BUT NEVER REMOVE THE LIGHT WITH STYLE!
=====================================*/

void CleanDuplicateLights(void){

	worldShadowLight_t *light1, *light2, *light0, *plug;
	vec3_t tmp;

	for(light0 = shadowLight_static; light0; light0 = light0->s_next) {
		
		if(CL_PMpointcontents(light0->origin) & MASK_SOLID) // light out of level cut off!!!!
			DeleteCurrentLight(light0);
	}
	

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
	if(!r_useNvConditionalRender->value){
	if(BoundsAndSphereIntersect (currentShadowLight->mins, currentShadowLight->maxs, r_origin, 25))
		return true;
	}

	qglDisable (GL_CULL_FACE);
	qglDisable(GL_TEXTURE_2D);
	qglDepthMask(0);
	qglColorMask(0,0,0,0);
	
	// begin occlusion test
	qglBeginQueryARB(GL_SAMPLES_PASSED_ARB, lightsQueries[currentShadowLight->occQ]);

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
	qglEndQueryARB(GL_SAMPLES_PASSED_ARB);

	qglEnable(GL_CULL_FACE);
	qglEnable(GL_TEXTURE_2D);
	qglDepthMask(1);
	qglColorMask(1,1,1,1);

	if(gl_state.nv_conditional_render && r_useNvConditionalRender->value){
	// player in light bounds draw light in any case
	if(BoundsAndSphereIntersect (currentShadowLight->mins, currentShadowLight->maxs, r_origin, 25))
		glBeginConditionalRenderNV(lightsQueries[currentShadowLight->occQ], GL_QUERY_NO_WAIT_NV);
	else
		glBeginConditionalRenderNV(lightsQueries[currentShadowLight->occQ], GL_QUERY_WAIT_NV);
	
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