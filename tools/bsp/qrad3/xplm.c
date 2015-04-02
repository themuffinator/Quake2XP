#include "qrad.h"

static xplm_t	xplm;

//
// 3-vector radiosity basis for normal mapping
// need high precision to lower error on further bounces
//
const vec3_t xplm_basisVecs[XPLM_NUMVECS] = {
	{ 0.81649658092772603273242802490196f, 0.f, 0.57735026918962576450914878050195f },
	{ -0.40824829046386301636621401245098f, 0.70710678118654752440084436210485f, 0.57735026918962576450914878050195f },
	{ -0.40824829046386301636621401245098f, -0.70710678118654752440084436210485f, 0.57735026918962576450914878050195f }
};

void AddPointToTriangulation (patch_t *patch, triangulation_t *trian);
void TriangulatePoints (triangulation_t *trian);

triangulation_t	*AllocTriangulation (dplane_t *plane);
void FreeTriangulation (triangulation_t *tr);

void FreeTransfers (void);
void BounceLight (void);
void CheckPatches (void);

/*
===================
XP_MakeTransfers

Calculates diffuse energy transfer from the current patch
onto basis vectors of all other visible patches.
===================
*/
static void XP_MakeTransfers (const int patchIndex) {
	// KRIGS: might want to increase stack reserve size in linker settings
	float		dots[MAX_PATCHES];
	float		transfers[MAX_PATCHES][XPLM_NUMVECS];
	byte		pvs[(MAX_MAP_LEAFS + 7) / 8];
	int			i, j;
	vec3_t		delta, deltaTS;
	float		dist, scale, trans;
	patch_t		*patch = &patches[patchIndex], *patch2;
	float		total;
	//	int			itotal;
	int			cluster;

	if (!PvsForOrigin (patch->origin, pvs))
		return;

	// find out which patches will collect light from patch
	total = 0.f;
	patch->numtransfers = 0;

	for (j = 0, patch2 = patches; j < num_patches; j++, patch2++) {
		dots[j] = 0.f;

		for (i = 0; i < XPLM_NUMVECS; i++)
			transfers[j][i] = 0.f;

		if (j == patchIndex)
			continue;

		// check pvs bit
		if (!nopvs) {
			cluster = patch2->cluster;

			if (cluster == -1)
				continue;
			if (!(pvs[cluster >> 3] & (1 << (cluster & 7))))
				continue;	// not in pvs
		}

		// calculate vector
		VectorSubtract (patch->origin, patch2->origin, delta);

		dist = VectorNormalize (delta, delta);

		if (!dist)
			continue;	// should never happen

		// relative angles
		deltaTS[2] = DotProduct (delta, patch2->plane->normal);
		scale = deltaTS[2] * -DotProduct (delta, patch->plane->normal);

		if (scale <= 0.f)
			continue;

		// check exact transfer
		if (TestLine_r (0, patch->origin, patch2->origin))
			continue;

		scale *= patch2->area / (dist * dist);

		if (scale > 0.f) {
			// transform delta into tangent space
			// FIXME: transform basis into world space outside of the loop instead
			deltaTS[0] = DotProduct (delta, patch2->tangents[0]);
			deltaTS[1] = DotProduct (delta, patch2->tangents[1]);

			for (i = 0; i < XPLM_NUMVECS; i++) {
				trans = DotProduct (deltaTS, xplm_basisVecs[i]);

				if (trans > 0.f) {
					transfers[j][i] = trans * scale;
					total += transfers[j][i] * xplm_basisVecs[i][2];
				}
			}

			dots[j] = scale;

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

	if (patch->numtransfers) {
		transfer_t *t;

		if (patch->numtransfers < 0 || patch->numtransfers > MAX_PATCHES)
			Error ("XP_MakeTransfers(): weird numtransfers (%i).", patch->numtransfers);

		patch->transfers = malloc (patch->numtransfers * sizeof(transfer_t));

		if (!patch->transfers)
			Error ("XP_MakeTransfers(): memory allocation failure.");

		//		memset(patch->transfers, 0, patch->numtransfers * sizeof(transfer_t));

		//
		// normalize all transfers so all of the light
		// is transfered to the surroundings
		//

		//		itotal = 0;

		for (j = 0, t = patch->transfers; j < num_patches; j++) {
			if (dots[j] <= 0.f)
				continue;

			for (i = 0; i < XPLM_NUMVECS; i++) {
				t->transfer[i] = transfers[j][i] * 0x10000 / total;
				//				itotal += t->transfer[i] * xplm_basisVecs[i][2];
			}

			t->patch = j;
			t++;
		}

		//		qprintf("patch %i transfer sum: %i\n", patchIndex, itotal);
	}

	// don't bother locking around this, not that important
	total_transfer += patch->numtransfers;
}

/*
===============
XP_LerpTriangle

===============
*/
static void XP_LerpTriangle (triangulation_t *trian, triangle_t *t, vec3_t point, vec3_t colors[]) {
	patch_t		*p1, *p2, *p3;
	vec3_t		base[XPLM_NUMVECS], d1[XPLM_NUMVECS], d2[XPLM_NUMVECS];
	float		x, y, x1, y1, x2, y2;
	int			i;

	p1 = trian->points[t->edges[0]->p0];
	p2 = trian->points[t->edges[1]->p0];
	p3 = trian->points[t->edges[2]->p0];

	for (i = 0; i < XPLM_NUMVECS; i++) {
		VectorCopy (p1->totallight[i], base[i]);
		VectorSubtract (p2->totallight[i], base[i], d1[i]);
		VectorSubtract (p3->totallight[i], base[i], d2[i]);
	}

	x = DotProduct (point, t->edges[0]->normal) - t->edges[0]->dist;
	y = DotProduct (point, t->edges[2]->normal) - t->edges[2]->dist;

	x1 = 0.f;
	y1 = DotProduct (p2->origin, t->edges[2]->normal) - t->edges[2]->dist;

	x2 = DotProduct (p3->origin, t->edges[0]->normal) - t->edges[0]->dist;
	y2 = 0.f;

	if (fabsf (y1) < ON_EPSILON || fabsf (x2) < ON_EPSILON) {
		for (i = 0; i < XPLM_NUMVECS; i++)
			VectorCopy (base[i], colors[i]);

		return;
	}

	for (i = 0; i < XPLM_NUMVECS; i++) {
		VectorMA (base[i], x / x2, d2[i], colors[i]);
		VectorMA (colors[i], y / y1, d1[i], colors[i]);
	}
}

/*
===============
XP_SampleTriangulation

===============
*/
static void XP_SampleTriangulation (vec3_t point, triangulation_t *trian, vec3_t colors[]) {
	triangle_t	*t;
	triedge_t	*e;
	vec_t		d, best;
	patch_t		*p0, *p1;
	vec3_t		v1, v2;
	int			i, j;

	if (trian->numpoints == 0) {
		for (i = 0; i < XPLM_NUMVECS; i++)
			VectorClear (colors[i]);

		return;
	}

	if (trian->numpoints == 1) {
		for (i = 0; i < XPLM_NUMVECS; i++)
			VectorCopy (trian->points[0]->totallight[i], colors[i]);

		return;
	}

	// search for triangles
	for (t = trian->tris, j = 0; j < trian->numtris; t++, j++) {
		if (!PointInTriangle (point, t))
			continue;

		// this is it
		XP_LerpTriangle (trian, t, point, colors);

		return;
	}

	// search for exterior edge
	for (e = trian->edges, j = 0; j < trian->numedges; e++, j++) {
		if (e->tri)
			continue;	// not an exterior edge

		d = DotProduct (point, e->normal) - e->dist;

		if (d < 0.f)
			continue;	// not in front of edge

		p0 = trian->points[e->p0];
		p1 = trian->points[e->p1];

		VectorSubtract (p1->origin, p0->origin, v1);
		VectorNormalize (v1, v1);
		VectorSubtract (point, p0->origin, v2);

		d = DotProduct (v2, v1);

		if (d < 0.f || d > 1.f)
			continue;

		for (i = 0; i < XPLM_NUMVECS; i++)
		for (j = 0; j < 3; j++)
			colors[i][j] = p0->totallight[i][j] + d * (p1->totallight[i][j] - p0->totallight[i][j]);

		return;
	}

	// search for nearest point
	best = 99999;
	p1 = NULL;

	for (j = 0; j < trian->numpoints; j++) {
		p0 = trian->points[j];
		VectorSubtract (point, p0->origin, v1);

		d = VectorLength (v1);

		if (d < best) {
			best = d;
			p1 = p0;
		}
	}

	if (!p1)
		Error ("XP_SampleTriangulation(): no points.");

	for (i = 0; i < XPLM_NUMVECS; i++)
		VectorCopy (p1->totallight[i], colors[i]);
}

/*
=======================
XP_GatherSampleLight

'weight' is the normalizer for multisampling.
=======================
*/
static void XP_GatherSampleLight (const vec3_t pos, const vec3_t tbnMatrix[3], float **styleTables, int sampleIndex, int numPoints, float weight, vec3_t patchSample) {
	int				i, j;
	directlight_t	*l;
	byte			pvs[(MAX_MAP_LEAFS + 7) / 8];
	vec3_t			delta, deltaTS;
	float			dot;
	float			dist;
	float			scale, scale2;
	float			*dest;

	// get the PVS for the pos to limit the number of checks
	if (!PvsForOrigin (pos, pvs))
		return;

	for (i = 0; i < dvis->numclusters; i++) {
		if (!(pvs[i >> 3] & (1 << (i & 7))))
			continue;

		for (l = directlights[i]; l; l = l->next) {
			VectorSubtract (l->origin, pos, delta);
			dist = VectorNormalize (delta, delta);

			// check against surface plane
			deltaTS[2] = DotProduct (delta, tbnMatrix[2]);
			if (deltaTS[2] <= 0.001f)
				continue;	// behind sample surface

			switch (l->type) {
				case emit_point:
					// linear falloff
					scale = l->intensity - dist;
					break;
				case emit_surface:
					dot = -DotProduct (delta, l->normal);
					if (dot <= 0.001f)
						goto skipadd;	// behind light surface

					// quadratic falloff
					scale = l->intensity / (dist * dist) * dot;
					break;
				case emit_spotlight:
					dot = -DotProduct (delta, l->normal);
					if (dot <= l->stopdot)
						goto skipadd;	// outside light cone

					// linear falloff
					scale = l->intensity - dist;

					// cone falloff
					//				scale /= 1.f - l->stopdot;
					break;
				default:
					Error ("XP_GatherSampleLight(): bad l->type.");
			}

			if (scale <= 0.f)
				continue;
			if (TestLine_r (0, pos, l->origin))
				continue;	// occluded

			scale *= deltaTS[2] * weight;

			// if this style doesn't have a table yet, allocate one
			if (!styleTables[l->style]) {
				styleTables[l->style] = malloc (numPoints * XPLM_NUMVECS * sizeof(vec3_t));
				memset (styleTables[l->style], 0, numPoints * XPLM_NUMVECS * sizeof(vec3_t));
			}

			// accumulate the sample for patch
			if (l->style == 0)
				VectorMA (patchSample, scale, l->color, patchSample);

			// transform delta into tangent space
			// FIXME: transform basis into world space outside of the loops instead
			deltaTS[0] = DotProduct (delta, tbnMatrix[0]);
			deltaTS[1] = DotProduct (delta, tbnMatrix[1]);

			// calc contribution for each basis vector
			dest = styleTables[l->style] + sampleIndex * 3;

			for (j = 0; j < XPLM_NUMVECS; j++, dest += numPoints * 3) {
				scale2 = scale * DotProduct (deltaTS, xplm_basisVecs[j]);

				if (scale2 > 0.f)
					VectorMA (dest, scale2, l->color, dest);
			}
		skipadd:;
		}
	}
}

/*
===================
XP_BuildFaceLights

Collect initial lighting on the given face.
===================
*/
static void XP_BuildFaceLights (int facenum) {
	float		*styleTables[MAX_LSTYLES];
	dface_t		*f = &dfaces[facenum];
	facelight_t	*fl = &facelight[facenum];
	texinfo_t	*tex = &texinfo[f->texinfo];
	lightinfo_t	*l;
	patch_t		*patch;
	vec3_t		tbnMatrix[3];
	vec3_t		patchSample;	// single vanilla style sample against surface normal
	int			numsamples = extrasamples ? extrasamplesvalue : 1;
	int			size;
	int			i, j;

	if (tex->flags & (SURF_WARP | SURF_SKY))
		return;		// non-lit texture

	memset (styleTables, 0, sizeof(styleTables));

	l = malloc (numsamples * sizeof(lightinfo_t));

	for (i = 0; i < numsamples; i++) {
		memset (&l[i], 0, sizeof(l[i]));
		l[i].surfnum = facenum;
		l[i].face = f;
		VectorCopy (dplanes[f->planenum].normal, l[i].facenormal);
		l[i].facedist = dplanes[f->planenum].dist;

		if (f->side) {
			VectorSubtract (vec3_origin, l[i].facenormal, l[i].facenormal);
			l[i].facedist = -l[i].facedist;
		}

		// get the origin offset for rotating bmodels
		VectorCopy (face_offset[facenum], l[i].modelorg);

		CalcFaceVectors (&l[i]);
		CalcFaceExtents (&l[i]);
		CalcPoints (&l[i], sampleofs[i][0], sampleofs[i][1]);
	}

	// construct the world space -> tangent space matrix
	VectorNormalize (tex->vecs[0], tbnMatrix[0]);
	VectorNormalize (tex->vecs[1], tbnMatrix[1]);
	VectorCopy (l[0].facenormal, tbnMatrix[2]);

	// RGB float buffer
	size = l[0].numsurfpt * sizeof(vec3_t);

	fl->numsamples = l[0].numsurfpt;
	fl->origins = malloc (size);
	memcpy (fl->origins, l[0].surfpt, size);

	// each style table contains several pages (one for each basis vector) of RGB sample sets
	styleTables[0] = malloc (size * XPLM_NUMVECS);
	memset (styleTables[0], 0, size * XPLM_NUMVECS);

	for (i = 0; i < l[0].numsurfpt; i++) {
		VectorClear (patchSample);

		// FIXME: refine
		for (j = 0; j < numsamples; j++)
			XP_GatherSampleLight (l[j].surfpt[i], tbnMatrix, styleTables, i, l[0].numsurfpt, 1.f / numsamples, patchSample);

		// contribute the flat sample to one or more patches
		AddSampleToPatch (l[0].surfpt[i], patchSample, facenum);
	}

	// average up the direct light on each patch for radiosity
	for (patch = face_patches[facenum]; patch; patch = patch->next) {
		if (patch->samples)
			VectorScale (patch->samplelight, 1.f / patch->samples, patch->samplelight);
		//		else
		//			printf ("patch with no samples\n");
	}

	for (i = 0; i < MAX_LSTYLES; i++) {
		if (!styleTables[i])
			continue;
		if (fl->numstyles == MAX_STYLES)
			break;

		fl->samples[fl->numstyles] = styleTables[i];
		fl->stylenums[fl->numstyles] = i;
		fl->numstyles++;
	}

	//
	// the light from direct lights is sent out, but the
	// texture itself should still be full bright
	//

	if (face_patches[facenum]->baselight[0] >= DIRECT_LIGHT ||
		face_patches[facenum]->baselight[1] >= DIRECT_LIGHT ||
		face_patches[facenum]->baselight[2] >= DIRECT_LIGHT) {

		for (j = 0; j < XPLM_NUMVECS; j++) {
			float *spot = fl->samples[0] + j * l[0].numsurfpt * 3;

			for (i = 0; i < l[0].numsurfpt; i++, spot += 3)
				VectorMA (spot, xplm_basisVecs[j][2], face_patches[facenum]->baselight, spot);
		}
	}

	free (l);
}

/*
=============
XP_FinalLightFaces

Add the indirect lighting on top of the direct lighting & save into final map format.
=============
*/
static void XP_FinalLightFaces (int facenum) {
	dface_t		*f;
	int			i, j, k, l;
	patch_t		*patch;
	triangulation_t	*trian;
	facelight_t	*fl;
	float		_minlight, minl[3], maxl[3];
	float		cmax, newmax;
	const float	*smp;
	byte		*dest;
	int			pfacenum;
	vec3_t		facemins, facemaxs;
	vec3_t		lb, (*adds)[XPLM_NUMVECS];

	f = &dfaces[facenum];
	fl = &facelight[facenum];

	if (texinfo[f->texinfo].flags & (SURF_WARP | SURF_SKY))
		return;		// non-lit texture

	ThreadLock ();

	f->lightofs = xplm.dataSize;
	xplm.dataSize += fl->numstyles * fl->numsamples * XPLM_NUMVECS * 3;

	if (xplm.dataSize > MAX_XPLM_SIZE)
		Error ("XP_FinalLightFaces(): MAX_XPLM_SIZE (%i) exceeded.", MAX_XPLM_SIZE);

	ThreadUnlock ();

	f->styles[0] = 0;
	f->styles[1] = f->styles[2] = f->styles[3] = 0xFF;

	//
	// set up the triangulation
	//

	if (numbounce > 0) {
		ClearBounds (facemins, facemaxs);

		for (i = 0; i < f->numedges; i++) {
			int		ednum;

			ednum = dsurfedges[f->firstedge + i];
			if (ednum >= 0)
				AddPointToBounds (dvertexes[dedges[ednum].v[0]].point, facemins, facemaxs);
			else
				AddPointToBounds (dvertexes[dedges[-ednum].v[1]].point, facemins, facemaxs);
		}

		trian = AllocTriangulation (&dplanes[f->planenum]);

		// for all faces on the plane, add the nearby patches
		// to the triangulation
		for (pfacenum = planelinks[f->side][f->planenum]; pfacenum; pfacenum = facelinks[pfacenum]) {
			for (patch = face_patches[pfacenum]; patch; patch = patch->next) {
				for (i = 0; i < 3; i++) {
					if (facemins[i] - patch->origin[i] > subdiv * 2)
						break;
					if (patch->origin[i] - facemaxs[i] > subdiv * 2)
						break;
				}

				if (i != 3)
					continue;	// not needed for this face

				AddPointToTriangulation (patch, trian);
			}
		}

		for (i = 0; i < trian->numpoints; i++)
			memset (trian->edgematrix[i], 0, trian->numpoints*sizeof(trian->edgematrix[0][0]));

		TriangulatePoints (trian);

		// precache interpolated indirect lighting samples
		adds = malloc (fl->numsamples * XPLM_NUMVECS * sizeof(vec3_t));

		for (j = 0; j < fl->numsamples; j++)
			XP_SampleTriangulation (fl->origins + j * 3, trian, adds[j]);
	}

	//
	// sample the triangulation
	//

	if (fl->numstyles > MAXLIGHTMAPS) {
		fl->numstyles = MAXLIGHTMAPS;

		printf ("XP_FinalLightFaces(): face with too many lightstyles at (%f %f %f).\n",
			face_patches[facenum]->origin[0],
			face_patches[facenum]->origin[1],
			face_patches[facenum]->origin[2]);
	}

	// _minlight allows models that have faces that would not be
	// illuminated to receive a mottled light pattern instead of black
	_minlight = FloatForKey (face_entity[facenum], "_minlight") * 128.f;

	for (l = 0; l < 3; l++) {
		minl[l] = _minlight * xplm_basisVecs[l][2];
		maxl[l] = maxlight * xplm_basisVecs[l][2];
	}

	dest = xplm.data + f->lightofs;

	for (i = 0; i < fl->numstyles; i++) {
		f->styles[i] = fl->stylenums[i];
		smp = fl->samples[i];

		for (l = 0; l < XPLM_NUMVECS; l++) {
			for (j = 0; j < fl->numsamples; j++, smp += 3) {
				// direct lighting
				if (qrad_dlMode == 4)
					VectorClear (lb);
				else
					VectorCopy (smp, lb);

				// indirect lighting
				if (numbounce > 0 && i == 0)
					VectorAdd (lb, adds[j][l], lb);

				// add an ambient term if desired
				for (k = 0; k < 3; k++)
					lb[k] += ambient * xplm_basisVecs[l][2];

				VectorScale (lb, lightscale, lb);

				// we need to clamp without allowing hue to change
				for (k = 0; k < 3; k++)
				if (lb[k] < xplm_basisVecs[l][2])
					lb[k] = xplm_basisVecs[l][2];

				cmax = lb[0];

				if (lb[1] > cmax)
					cmax = lb[1];
				if (lb[2] > cmax)
					cmax = lb[2];

				newmax = cmax;

				if (newmax < minl[l])
					newmax = minl[l] + (rand () % 48) * xplm_basisVecs[l][2];
				if (newmax > maxl[l])
					newmax = maxl[l];

				for (k = 0; k < 3; k++)
					*dest++ = lb[k] * newmax / cmax;
			}
		}
	}

	if (numbounce > 0) {
		FreeTriangulation (trian);
		free (adds);
	}
}

/*
=============
XP_RadWorld

=============
*/
void XP_RadWorld (void) {
	if (numnodes == 0 || numfaces == 0)
		Error ("Empty map.");

	MakeBackplanes ();
	MakeParents (0, -1);
	MakeTnodes (&dmodels[0]);

	// turn each face into a single patch
	MakePatches ();

	// subdivide patches to a maximum dimension
	SubdividePatches ();

	// create direct lights out of patches & lights
	CreateDirectLights ();

	// build initial face lights (direct lighting)
	RunThreadsOnIndividual (numfaces, true, XP_BuildFaceLights);

	if (numbounce > 0) {
		// build transfer lists
		RunThreadsOnIndividual (num_patches, true, XP_MakeTransfers);

		qprintf ("transfer lists: %i total, %5.2f MB\n", total_transfer, (float)total_transfer * sizeof(transfer_t) / 1048576.f);

		// spread light around
		BounceLight ();

		FreeTransfers ();

		CheckPatches ();
	}

	//	if (glview)
	//		WriteGlView ();

	// blend bounced light into direct light and save
	PairEdges ();
	LinkPlaneFaces ();

	xplm.dataSize = 0;

	RunThreadsOnIndividual (numfaces, true, XP_FinalLightFaces);
}

/*
=============
XP_WriteXPLM

=============
*/
void XP_WriteXPLM (char *filename) {
	FILE *f;
	byte b;
	int i, j;

	f = SafeOpenWrite (filename);

	/*
		//
		// write id & version
		//

		//
		// write BSP stats
		//

		j = LittleLong(nummodels); SafeWrite(f, &j, 4);
		j = LittleLong(numbrushsides); SafeWrite(f, &j, 4);
		j = LittleLong(numplanes); SafeWrite(f, &j, 4);
		j = LittleLong(numtexinfo); SafeWrite(f, &j, 4);
		j = LittleLong(numvertexes); SafeWrite(f, &j, 4);
		j = LittleLong(numfaces); SafeWrite(f, &j, 4);
		j = LittleLong(numleafs); SafeWrite(f, &j, 4);
		j = LittleLong(numleaffaces); SafeWrite(f, &j, 4);
		j = LittleLong(numleafbrushes); SafeWrite(f, &j, 4);
		j = LittleLong(numsurfedges); SafeWrite(f, &j, 4);
		j = LittleLong(numedges); SafeWrite(f, &j, 4);
		*/

	//
	// face count & offsets
	//

	j = LittleLong (numfaces);
	SafeWrite (f, &j, 4);

	for (i = 0; i < numfaces; i++) {
		j = LittleLong (dfaces[i].lightofs);
		SafeWrite (f, &j, 4);
	}

	//
	// face style counts
	//

	//
	// map scale
	//

	b = (byte)lightmap_scale;
	SafeWrite (f, &b, 1);

	//
	// data
	//

	SafeWrite (f, xplm.data, (xplm.dataSize + 3) & ~3);

	fclose (f);
}
