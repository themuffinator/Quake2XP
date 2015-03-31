/*
===========================================================================
Copyright (C) 1997-2006 Id Software, Inc.

This file is part of Quake 2 Tools source code.

Quake 2 Tools source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake 2 Tools source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake 2 Tools source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "qrad.h"



/*

NOTES
-----

every surface must be divided into at least two patches each axis

*/

patch_t		*face_patches[MAX_MAP_FACES];
entity_t	*face_entity[MAX_MAP_FACES];
patch_t		patches[MAX_PATCHES];
unsigned	num_patches;

static int		leafparents[MAX_MAP_LEAFS];
static int		nodeparents[MAX_MAP_NODES];

vec3_t		radiosity[MAX_PATCHES];		// light leaving a patch, single value
vec3_t		illumination[MAX_PATCHES][XPLM_NUMVECS];	// light arriving at a patch, for each basis vector

vec3_t		face_offset[MAX_MAP_FACES];		// for rotating bmodels
dplane_t	backplanes[MAX_MAP_PLANES];

char		inbase[32], outbase[32];

int			fakeplanes;					// created planes for origin offset 

int			numbounce = 8;

qboolean	extrasamples;
int		extrasamplesvalue;

float		subdiv = 64;

qboolean	dumppatches;

qboolean	qrad_xplit = false;
qboolean	qrad_xplm = false;
int		qrad_numBasisVecs = 1;					// 3 for XPLM
int		qrad_dlMode = 2;

void BuildLightmaps (void);
int TestLine (vec3_t start, vec3_t stop);

int		junk;

float	ambient = 0.f;
float	maxlight = 196.f;

float	lightscale = 1.0;

qboolean	glview;
qboolean	nopvs;
qboolean	deluxeMapping;

char		source[1024];

float	direct_scale =	0.4f;
float	entity_scale =	1.f;

int	total_transfer;		// for MakeTransfers

/*
===================================================================

	MISC

===================================================================
*/

/*
=============
MakeBackplanes

=============
*/
void MakeBackplanes (void)
{
	int		i;

	for (i=0 ; i<numplanes ; i++)
	{
		backplanes[i].dist = -dplanes[i].dist;
		VectorSubtract (vec3_origin, dplanes[i].normal, backplanes[i].normal);
	}
}

/*
=============
MakeParents
=============
*/
void MakeParents (int nodenum, int parent)
{
	int		i, j;
	dnode_t	*node;

	nodeparents[nodenum] = parent;
	node = &dnodes[nodenum];

	for (i=0 ; i<2 ; i++)
	{
		j = node->children[i];
		if (j < 0)
			leafparents[-j - 1] = nodenum;
		else
			MakeParents (j, nodenum);
	}
}

/*
===================================================================

	TRANSFER SCALES

===================================================================
*/
int PointInLeafnum (const vec3_t point) {
	int			nodenum;
	float		dist;
	dnode_t		*node;
	dplane_t	*plane;

	nodenum = 0;

	while (nodenum >= 0) {
		node = &dnodes[nodenum];
		plane = &dplanes[node->planenum];

		dist = DotProduct (point, plane->normal) - plane->dist;

		if (dist > 0.f)
			nodenum = node->children[0];
		else
			nodenum = node->children[1];
	}

	return -nodenum - 1;
}

dleaf_t *PointInLeaf (const vec3_t point) {
	return &dleafs[PointInLeafnum(point)];
}

qboolean PvsForOrigin (const vec3_t org, byte *pvs) {
	dleaf_t	*leaf;

	if (!visdatasize) {
		memset (pvs, 255, (numleafs+7)/8 );
		return true;
	}

	leaf = PointInLeaf (org);

	if (leaf->cluster == -1)
		return false;		// in solid leaf

	DecompressVis (dvisdata + dvis->bitofs[leaf->cluster][DVIS_PVS], pvs);

	return true;
}


/*
=============
MakeTransfers

=============
*/
void MakeTransfers (int i)
{
	int			j;
	vec3_t		delta;
	vec_t		dist, scale;
	float		trans;
	int			itrans;
	patch_t		*patch, *patch2;
	float		total;
	dplane_t	plane;
	vec3_t		origin;
	float		transfers[MAX_PATCHES];
	int			itotal;
	byte		pvs[(MAX_MAP_LEAFS+7)/8];
	int			cluster;

	patch = patches + i;
	total = 0;

	VectorCopy (patch->origin, origin);
	plane = *patch->plane;

	if (!PvsForOrigin (patch->origin, pvs))
		return;

	// find out which patch2s will collect light
	// from patch

	patch->numtransfers = 0;

	for (j=0, patch2 = patches ; j<num_patches ; j++, patch2++)
	{
		transfers[j] = 0;

		if (j == i)
			continue;

		// check pvs bit
		if (!nopvs)
		{
			cluster = patch2->cluster;
			if (cluster == -1)
				continue;
			if ( ! ( pvs[cluster>>3] & (1<<(cluster&7)) ) )
				continue;	// not in pvs
		}

		// calculate vector
		VectorSubtract (patch2->origin, origin, delta);

		dist = VectorNormalize (delta, delta);

		if (!dist)
			continue;	// should never happen

		// relative angles
		scale = DotProduct (delta, plane.normal);
		scale *= -DotProduct (delta, patch2->plane->normal);

		if (scale <= 0.f)
			continue;

		// check exact transfer
		if (TestLine_r (0, patch->origin, patch2->origin) )
			continue;

		trans = scale * patch2->area / (dist * dist);

		if (trans < 0)
			trans = 0;		// rounding errors...

		transfers[j] = trans;

		if (trans > 0.f)
		{
			total += trans;
			patch->numtransfers++;
		}
	}

	//
	// copy the transfers out and normalize
	// total should be somewhere near PI if everything went right
	// because partial occlusion isn't accounted for, and nearby
	// patches have underestimated form factors, it will usually
	// be higher than PI
	//

	if (patch->numtransfers)
	{
		transfer_t	*t;
		
		if (patch->numtransfers < 0 || patch->numtransfers > MAX_PATCHES)
			Error ("Weird numtransfers");

		patch->transfers = malloc (patch->numtransfers * sizeof(transfer_t));

		if (!patch->transfers)
			Error ("Memory allocation failure");

		//
		// normalize all transfers so all of the light
		// is transfered to the surroundings
		//

		t = patch->transfers;
		itotal = 0;

		for (j=0 ; j<num_patches ; j++)
		{
			if (transfers[j] <= 0.f)
				continue;

			itrans = transfers[j] * 0x10000 / total;
			itotal += itrans;
			t->transfer[0] = itrans;
			t->patch = j;
			t++;
		}
	}

	// don't bother locking around this, not that important
	total_transfer += patch->numtransfers;
}

/*
=============
FreeTransfers

=============
*/
void FreeTransfers (void) {
	int		i;

	for (i=0 ; i<num_patches ; i++)
	{
		free (patches[i].transfers);
		patches[i].transfers = NULL;
	}
}


//===================================================================

/*
=============
WriteWorld

=============
*/
void WriteWorld (char *name) {
	int		i, j;
	FILE		*out;
	patch_t		*patch;
	winding_t	*w;

	out = fopen (name, "w");
	if (!out)
		Error ("Couldn't open %s", name);

	for (j=0, patch=patches ; j<num_patches ; j++, patch++)
	{
		w = patch->winding;
		fprintf (out, "%i\n", w->numpoints);
		for (i=0 ; i<w->numpoints ; i++)
		{
			if (qrad_xplm) {
				fprintf (out, "%5.2f %5.2f %5.2f: %5.3f %5.3f %5.3f, %5.3f %5.3f %5.3f, %5.3f %5.3f %5.3f\n",
					w->p[i][0],
					w->p[i][1],
					w->p[i][2],
					patch->totallight[0][0],
					patch->totallight[0][1],
					patch->totallight[0][2],
					patch->totallight[1][0],
					patch->totallight[1][1],
					patch->totallight[1][2],
					patch->totallight[2][0],
					patch->totallight[2][1],
					patch->totallight[2][2]);
			}
			else {
				fprintf (out, "%5.2f %5.2f %5.2f: %5.3f %5.3f %5.3f, %5.3f %5.3f %5.3f, %5.3f %5.3f %5.3f\n",
					w->p[i][0],
					w->p[i][1],
					w->p[i][2],
					patch->totallight[0][0],
					patch->totallight[0][1],
					patch->totallight[0][2]);
			}
		}
		fprintf (out, "\n");
	}

	fclose (out);
}

/*
=============
WriteGlView

=============
*/
void WriteGlView (void) {
	char	name[1024];
	FILE	*f;
	int		i, j;
	patch_t	*p;
	winding_t	*w;

	strcpy (name, source);
	StripExtension (name);
	strcat (name, ".glr");

	f = fopen (name, "w");
	if (!f)
		Error ("Couldn't open %s", f);

	for (j=0 ; j<num_patches ; j++)
	{
		p = &patches[j];
		w = p->winding;
		fprintf (f, "%i\n", w->numpoints);
		for (i=0 ; i<w->numpoints ; i++)
		{
			if (qrad_xplm) {
				fprintf (f, "%5.2f %5.2f %5.2f: %5.3f %5.3f %5.3f, %5.3f %5.3f %5.3f, %5.3f %5.3f %5.3f\n",
					w->p[i][0],
					w->p[i][1],
					w->p[i][2],
					p->totallight[0][0] / 128,
					p->totallight[0][1] / 128,
					p->totallight[0][2] / 128,
					p->totallight[1][0] / 128,
					p->totallight[1][1] / 128,
					p->totallight[1][2] / 128,
					p->totallight[2][0] / 128,
					p->totallight[2][1] / 128,
					p->totallight[2][2] / 128);
			}
			else {
				fprintf (f, "%5.2f %5.2f %5.2f: %5.3f %5.3f %5.3f, %5.3f %5.3f %5.3f, %5.3f %5.3f %5.3f\n",
					w->p[i][0],
					w->p[i][1],
					w->p[i][2],
					p->totallight[0][0] / 128,
					p->totallight[0][1] / 128,
					p->totallight[0][2] / 128);
			}
		}
		fprintf (f, "\n");
	}

	fclose (f);
}

//==============================================================

/*
=============
CollectLight

=============
*/
float CollectLight (void) {
	int		i, j, k;
	patch_t	*patch;
	float f, total = 0.f;

	for (i=0, patch=patches ; i<num_patches ; i++, patch++) {
		// clear outgoing light
		VectorClear (radiosity[i]);

		// sky's never collect light, it is just dropped
		if (patch->sky) {
			for (k = 0; k < qrad_numBasisVecs; k++)
				VectorClear (illumination[i][k]);

			continue;
		}

		for (k = 0; k < qrad_numBasisVecs; k++) {
			for (j=0 ; j<3 ; j++) {
				// receive incoming light
				patch->totallight[k][j] += illumination[i][k][j] / patch->area;

				// get ready for the next bounce
				f = illumination[i][k][j] * patch->reflectivity[j];

				// for XP lightmaps, express outgoing diffuse light by single RGB color,
				// summing up all incoming light by combining dot products
				// of each basis vector with tangent space normal N={0,0,1}
				if (qrad_xplm)
					f *= xplm_basisVecs[k][2];

				radiosity[i][j] += f;
			}

			// collected now
			VectorClear (illumination[i][k]);
		}

		total += radiosity[i][0] + radiosity[i][1] + radiosity[i][2];
	}

	return total;
}

/*
=============
ShootLight

Send light out to other patches.
Runs multi-threaded.
=============
*/
void ShootLight (int patchnum) {
	int			i, k, l;
	patch_t		*patch = &patches[patchnum];
	transfer_t	*trans;
	vec3_t		send;

	//
	// this is the amount of light we are distributing
	// prescale it so that multiplying by the 16 bit
	// transfer values gives a proper output value
	//

	for (l=0 ; l<3 ; l++)
		send[l] = radiosity[patchnum][l] / 0x10000;

	for (k=0, trans=patch->transfers; k<patch->numtransfers ; k++, trans++)
		for (i=0 ; i<qrad_numBasisVecs ; i++)
			for (l=0 ; l<3 ; l++)
				illumination[trans->patch][i][l] += send[l] * trans->transfer[i];
}

/*
=============
BounceLight

=============
*/
void BounceLight (void) {
	int		i, j;
	char	name[64];
	float	f;
	patch_t	*p;

	for (i=0, p = patches ; i<num_patches ; i++, p++)
		for (j=0 ; j<3 ; j++)
			radiosity[i][j] = p->samplelight[j] * p->reflectivity[j] * p->area;

	for (i=0 ; i<numbounce ; i++) {
		RunThreadsOnIndividual (num_patches, false, ShootLight);

		f = CollectLight();

		qprintf ("bounce: %i added: %.2f \n", i, f);

		if (dumppatches && (i == 0 || i == numbounce - 1)) {
			sprintf (name, "bounce%i.txt", i);
			WriteWorld (name);
		}
	}
}

//==============================================================

/*
=============
CheckPatches

=============
*/
void CheckPatches (void) {
	int		i, k;
	patch_t	*patch;

	for (i=0, patch = patches ; i<num_patches ; i++, patch++) {
		for (k = 0; k < qrad_numBasisVecs; k++) {
			if (patch->totallight[k][0] < 0 || patch->totallight[k][1] < 0 || patch->totallight[k][2] < 0)
				Error ("CheckPatches(): negative patch %i totallight.\n", i);
		}
	}
}

/*
=============
RadWorld

=============
*/
void RadWorld (void) {
	if (numnodes == 0 || numfaces == 0)
		Error ("Empty map.");

	MakeBackplanes ();
	MakeParents (0, -1);
	MakeTnodes (&dmodels[0]);

	// turn each face into a single patch
	MakePatches ();

	// subdivide patches to a maximum dimension
	SubdividePatches ();

	// create directlights out of patches and lights
	CreateDirectLights ();

	// build initial facelights
	RunThreadsOnIndividual (numfaces, true, BuildFacelights);

	if (numbounce > 0)
	{
		// build transfer lists
		RunThreadsOnIndividual (num_patches, true, MakeTransfers);

		qprintf ("transfer lists: %i total, %5.2f MB\n", total_transfer, (float)total_transfer * sizeof(transfer_t) / (1024*1024));

		// spread light around
		BounceLight ();
		
		FreeTransfers ();

		CheckPatches ();
	}

	if (glview)
		WriteGlView ();

	// blend bounced light into direct light and save
	PairEdges ();
	LinkPlaneFaces ();

	lightdatasize = 0;
	RunThreadsOnIndividual (numfaces, true, FinalLightFace);
}


/*
===============
AddLightmapScaleKey

===============
*/
static void AddLightmapScaleKey(void) {
	epair_t	*w, *e, *last;
	char	s[256];

	// convert value into string
	sprintf(s, "%d", lightmap_scale);

	// search for it in entity string, moving to the end of the list
	for (w = entities->epairs; w; w = w->next) {
		if (!stricmp(w->key, "lightmap_scale")) {
			// found it already there, give it a new value and bye-bye
			w->value = copystring(s);
			return;
		}

		if (!w->next)
			last = w;
	}

	// create a new key and link it into the chain
	e = malloc(sizeof(epair_t));
	memset(e, 0, sizeof(epair_t));
	e->key = "lightmap_scale";
	e->value = copystring(s);

	last->next = e;
}

static void AddDeluxeKey(void) {
	epair_t	*w, *e, *last;
	char	s[256];

	// convert value into string
	sprintf(s, "%d", 1);

	// search for it in entity string, moving to the end of the list
	for (w = entities->epairs; w; w = w->next) {
		if (!stricmp(w->key, "deluxe")) {
			// found it already there, give it a new value and bye-bye
			w->value = copystring(s);
			return;
		}

		if (!w->next)
			last = w;
	}

	// create a new key and link it into the chain
	e = malloc(sizeof(epair_t));
	memset(e, 0, sizeof(epair_t));
	e->key = "deluxe";
	e->value = copystring(s);

	last->next = e;
}

/*
========
main

Light model file.
========
*/
int main (int argc, char **argv) {
	int			i;
	double		start, end;
	char		name[1024];
	float		f;

	printf ("----- Radiosity ----\n");

	verbose = false;

	for (i=1 ; i<argc ; i++)
	{
		if (!strcmp(argv[i],"-dump"))
			dumppatches = true;
		else if (!strcmp(argv[i],"-bounce"))
		{
			numbounce = atoi (argv[i+1]);
			numbounce = Q_clamp(numbounce, 0, 20);
			i++;

		}
		else if (!strcmp(argv[i],"-v"))
		{
			verbose = true;
		}
		else if (!strcmp(argv[i],"-samples") || !strcmp(argv[i],"-extra"))
		{
			extrasamples = true;
			extrasamplesvalue = atoi (argv[i+1]);
			extrasamplesvalue = Q_clamp(extrasamplesvalue, 1, MAX_SAMPLES);

			i++;

			printf ("using %i samples\n", extrasamplesvalue);
		}
		else if (!strcmp(argv[i],"-threads"))
		{
			numthreads = atoi (argv[i+1]);
			numthreads = Q_clamp(numthreads, 1, 32);
			i++;
		}
		
		else if (!strcmp(argv[i],"-chop"))
		{
			subdiv = atoi (argv[i+1]);
			subdiv = Q_clamp(subdiv, 32, 256);
			i++;
		}
		else if (!strcmp(argv[i],"-lightscale") || !strcmp(argv[i],"-scale"))
		{
			lightscale = atof (argv[i+1]);
			lightscale = max(0.f, lightscale);
			i++;
		}
		else if (!strcmp(argv[i],"-directscale") || !strcmp(argv[i],"-direct"))
		{
			f = atof(argv[i+1]);
			direct_scale *= max(0.f, f);
			printf ("direct light scaling at %f\n", direct_scale);
			i++;
		}
		else if (!strcmp(argv[i],"-entityscale") || !strcmp(argv[i],"-entity"))
		{
			f = atof(argv[i+1]);
			entity_scale *= max(0.f, f);
			printf ("entity light scaling at %f\n", entity_scale);
			i++;
		}
		else if (!strcmp(argv[i],"-glview"))
		{
			glview = true;
			printf ("glview = true\n");
		}
		else if (!strcmp(argv[i],"-nopvs"))
		{
			nopvs = true;
			printf ("nopvs = true\n");
		}
		else if (!strcmp(argv[i],"-ambient"))
		{
			ambient = atof (argv[i+1]);
			ambient = Q_clamp(ambient, 0.f, 1.f);
			ambient *= 255.f;
			i++;
		}
		else if (!strcmp(argv[i],"-maxlight"))
		{
			maxlight = atof (argv[i+1]);
			maxlight = Q_clamp(maxlight, 0.f, 1.f);
			maxlight *= 255.f;
			i++;
		}
		else if (!strcmp(argv[i],"-lightmap_scale"))
		{
			lightmap_scale = atoi(argv[i+1]);
			lightmap_scale = Q_clamp(lightmap_scale, 4, 128);
			i++;
		}
		else if (!strcmp(argv[i],"-deluxe"))
			deluxeMapping = true;
		else if (!strcmp (argv[i],"-tmpin"))
			strcpy (inbase, "/tmp");
		else if (!strcmp (argv[i],"-tmpout"))
			strcpy (outbase, "/tmp");
		else if (!strcmp(argv[i], "-xplit"))
			qrad_xplit = true;
		else if (!strcmp(argv[i], "-xplm"))
			qrad_xplm = true;
		else if (!strcmp(argv[i],"-xpdlmode"))
		{
			qrad_dlMode = atoi (argv[i+1]);
			qrad_dlMode = Q_clamp(qrad_dlMode, 0, 4);
			i++;
		}
		else
			break;
	}

	if (!lightmap_scale)
		lightmap_scale = 16;

	ThreadSetDefault ();

	if (maxlight > 255.f)
		maxlight = 255.f;

	if (i != argc - 1) {
		printf ("USAGE: q2xprad.exe [OPTIONS] FILENAME.bsp\n\n"

			"	Options:\n\n"

			"[-v]\n"
			"	verbose\n"
			"[-chop 32-256] (%i)\n"
			"	maximum patch size\n"
			"[-bounce 0-20] (%i)\n"
			"	number of light bounces\n"
			"[-samples 1-%i] (1)\n"
			"	number of light samples\n"
			"[-lightscale 0-N] (%.1f)\n"
			"	light scale overall\n"
			"[-directscale 0-N] (%.1f)\n"
			"	light scale from direct (surface) lights\n"
			"[-entityscale 0-N] (%.1f)\n"
			"	light scale from entity (level) lights\n"
			"[-ambient 0-1] (%.1f)\n"
			"	flat lighting added\n"
			"[-maxlight 0-1] (%.5f)\n"
			"	maximum lighting value stored\n"
			"[-threads 1-32] (max)\n"
			"	CPU cores to use\n"
			"[-lightmap_scale 4-128] (%i)\n"
			"	lightmap texel size\n"
			"[-xplit]\n"
			"	load lights from FILENAME.xplit\n"
			"[-xplm]\n"
			"	bake Q2XP126 format lightmaps into FILENAME.xplm\n"
			"[-xpdlmode 0-4 (2)]\n"
			"	store direct lighting from:\n\n"

			"	0 - all light surfaces, all lights\n"
			"	1 - all light surfaces, lights with 'lm_only' '1' keyword\n"
			"	2 - only sky surfaces, all lights\n"
			"	3 - only sky surfaces, lights with 'lm_only' '1' keyword\n"
			"	4 - none\n",
			64,		// subdiv
			8,		// numbounce
			MAX_SAMPLES,	// extrasamples (max)
			1.f,		// lightscale
			0.4f,		// direct_scale
			1.f,		// entity_scale
			0.f,		// ambient
			196.f / 255.f,	// maxlight
			16);		// lightmap_scale

		exit (1);
	}

	start = I_FloatTime ();

	SetQdirFromPath (argv[i]);	
	strcpy (source, ExpandArg(argv[i]));
	StripExtension (source);
	DefaultExtension (source, ".bsp");
	sprintf (name, "%s%s", inbase, source);
	printf ("reading %s\n", name);
	LoadBSPFile (name);

	ParseEntities ();
	CalcTextureReflectivity ();

	if (qrad_xplit) {
		//
		// load lights from .xplit file instead
		//

		char	source2[1024];
		char	name2[1024];

		strcpy (source2, source);
		StripExtension (source2);
		DefaultExtension (source2, ".xplit");
		sprintf (name2, "%s%s", inbase, source2);
		printf ("reading %s\n", name2);
		LoadXPLights (name2);
	}

	if (!visdatasize) {
		printf ("No vis information, direct lighting only.\n");
		numbounce = 0;
		ambient = 0.1;
	}

	if (qrad_xplm) {
		qrad_numBasisVecs = XPLM_NUMVECS;

		//
		// Q2XP126 SH lightmaps
		//

		XP_RadWorld();

		StripExtension(source);
		DefaultExtension(source, ".xplm");
		sprintf(name, "%s%s", outbase, source);
		printf("writing %s\n", name);

		XP_WriteXPLM (name);
	}
	else {
		//
		// normal Q2 lightmaps
		//

		RadWorld();

		// add 'lightmap_scale' key to worldspawn
		AddLightmapScaleKey();

		// add deluxe key to worldspawn
		if(deluxeMapping)
			AddDeluxeKey();

		// write new entity string
		UnparseEntities();

		sprintf(name, "%s%s", outbase, source);
		printf("writing %s\n", name);

		WriteBSPFile (name);
	}

	end = I_FloatTime ();

	printf ("%5.0f seconds elapsed\n", end - start);
	
	return 0;
}
