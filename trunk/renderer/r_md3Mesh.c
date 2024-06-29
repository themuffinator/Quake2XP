/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
#include "r_local.h"

int VectorCompareEpsilon(vec3_t v1, vec3_t v2, float epsilon)
{
	if (fabs(v1[0] - v2[0]) > epsilon)
		return 0;
	if (fabs(v1[1] - v2[1]) > epsilon)
		return 0;
	if (fabs(v1[2] - v2[2]) > epsilon)
		return 0;
	return 1;
}

void CalcTangent4MD3(uint16_t *index, md3Vertex_t *vertices, md3ST_t *texcos, vec3_t Tangent, vec3_t Binormal){
	float	*v0, *v1, *v2;
	float	*st0, *st1, *st2;
	vec3_t	vec1, vec2;
	vec3_t	planes[3];
	int		i;

	v0 = vertices[index[0]].xyz;
	v1 = vertices[index[1]].xyz;
	v2 = vertices[index[2]].xyz;
	st0 = texcos[index[0]].st;
	st1 = texcos[index[1]].st;
	st2 = texcos[index[2]].st;

	for (i = 0; i<3; i++){

		vec1[0] = v1[i]  - v0[i];
		vec1[1] = st1[0] - st0[0];
		vec1[2] = st1[1] - st0[1];
		vec2[0] = v2[i]  - v0[i];
		vec2[1] = st2[0] - st0[0];
		vec2[2] = st2[1] - st0[1];
		VectorNormalize(vec1);
		VectorNormalize(vec2);
		CrossProduct(vec1, vec2, planes[i]);
	}
	// Berserker's fix: some models could have degenerate triangles (or strongly elongated)
	//  corrected the divisor so that it was not very close to zero, otherwise we get division by zero or +/- INF
	if (fabs(planes[0][0]) <= DIV_EPSILON)
		planes[0][0] = 0.01 * sign(planes[0][0]);
	if (fabs(planes[1][0]) <= DIV_EPSILON)
		planes[1][0] = 0.01 * sign(planes[1][0]);
	if (fabs(planes[2][0]) <= DIV_EPSILON)
		planes[2][0] = 0.01 * sign(planes[2][0]);

	Tangent[0] = -planes[0][1] / planes[0][0];
	Tangent[1] = -planes[1][1] / planes[1][0];
	Tangent[2] = -planes[2][1] / planes[2][0];
	Binormal[0] = -planes[0][2] / planes[0][0];
	Binormal[1] = -planes[1][2] / planes[1][0];
	Binormal[2] = -planes[2][2] / planes[2][0];
	VectorNormalize(Tangent); //is this needed?
	VectorNormalize(Binormal);
}


int R_FindTriangleWithEdge(uint16_t *indexes, int numtris, uint16_t start, uint16_t end, int ignore)
{
	int i;
	int match, count;

	count = 0;
	match = -1;

	for (i = 0; i < numtris; i++, indexes += 3)
	{
		if ((indexes[0] == start && indexes[1] == end)
			|| (indexes[1] == start && indexes[2] == end)
			|| (indexes[2] == start && indexes[0] == end))
		{
			if (i != ignore)
				match = i;
			count++;
		}
		else if ((indexes[1] == start && indexes[0] == end)
			|| (indexes[2] == start && indexes[1] == end)
			|| (indexes[0] == start && indexes[2] == end))
		{
			count++;
		}
	}

	// detect edges shared by three triangles and make them seams
	if (count > 2)
		match = -1;

	return match;
}


/*
===============
R_BuildTriangleNeighbors
===============
*/
void R_BuildTriangleNeighbors(neighbours_t *neighbors, uint16_t *indexes, int numtris)
{
	int				i;
	neighbours_t	*n;
	uint16_t			*index;

	for (i = 0, index = indexes, n = neighbors; i < numtris; i++, index += 3, n++)
	{
		n->neighbours[0] = R_FindTriangleWithEdge(indexes, numtris, index[1], index[0], i);
		n->neighbours[1] = R_FindTriangleWithEdge(indexes, numtris, index[2], index[1], i);
		n->neighbours[2] = R_FindTriangleWithEdge(indexes, numtris, index[0], index[2], i);
	}
}

void *Mod_Hunk_Alloc(size_t size);
/*
=================
Mod_LoadAliasMD3Model
=================
*/
void Mod_LoadMD3(model_t *mod, void *buffer)
{
	int					version, i, j, k, l, m;
	dmd3_t				*inModel;
	dmd3frame_t			*inFrame;
	dmd3tag_t			*inTag;
	dmd3mesh_t			*inMesh;
	dmd3skin_t			*inSkin;
	dmd3coord_t			*inCoord;
	dmd3vertex_t		*inVerts;
	uint				*inIndex;

	uint16_t			*outIndex;
	md3Vertex_t			*outVerts;
	md3ST_t				*outCoord;
	md3Mesh_t			*outMesh;
	md3Tag_t			*outTag;
	md3Frame_t			*outFrame;
	md3Model_t			*outModel;
	static vec3_t		tangents[MD3_MAX_VERTS], binormals[MD3_MAX_VERTS];
	vec3_t				scaleMD3;
	char				name[MD3_MAX_PATH];
	float				lat, lng;

	VectorSet(scaleMD3, MD3_XYZ_SCALE, MD3_XYZ_SCALE, MD3_XYZ_SCALE);

	inModel = (dmd3_t *)buffer;
	version = LittleLong(inModel->version);

	if (version != MD3_ALIAS_VERSION)
	{
		VID_Error(ERR_DROP, "%s has wrong version number (%i should be %i)",
			mod->name, version, MD3_ALIAS_VERSION);
	}

	outModel = Mod_Hunk_Alloc(sizeof(md3Model_t));

	// byte swap the header fields and sanity check
	outModel->num_frames = LittleLong(inModel->num_frames);
	outModel->num_tags = LittleLong(inModel->num_tags);
	outModel->num_meshes = LittleLong(inModel->num_meshes);

	if (outModel->num_frames <= 0)
		VID_Error(ERR_DROP, "model %s has no frames", mod->name);
	else if (outModel->num_frames > MD3_MAX_FRAMES)
		VID_Error(ERR_DROP, "model %s has too many frames", mod->name);

	if (outModel->num_tags > MD3_MAX_TAGS)
		VID_Error(ERR_DROP, "model %s has too many tags", mod->name);
	else if (outModel->num_tags < 0)
		VID_Error(ERR_DROP, "model %s has invalid number of tags", mod->name);

	if (outModel->num_meshes <= 0)
		VID_Error(ERR_DROP, "model %s has no meshes", mod->name);
	else if (outModel->num_meshes > MD3_MAX_MESHES)
		VID_Error(ERR_DROP, "model %s has too many meshes", mod->name);

	//
	// load the frames
	//
	inFrame = (dmd3frame_t *)((byte *)inModel + LittleLong(inModel->ofs_frames));
	outFrame = outModel->frames = Mod_Hunk_Alloc(sizeof(md3Frame_t) * outModel->num_frames);

	for (i = 0; i < outModel->num_frames; i++, inFrame++, outFrame++)
	{
		for (j = 0; j < 3; j++)
		{
			outFrame->translate[j] = LittleFloat(inFrame->translate[j]);
			outFrame->mins[j] = LittleFloat(inFrame->mins[j]) + outFrame->translate[j];
			outFrame->maxs[j] = LittleFloat(inFrame->maxs[j]) + outFrame->translate[j];
		}

		outFrame->radius = LittleFloat(inFrame->radius);
	}

	//
	// load the tags
	//
	inTag = (dmd3tag_t *)((byte *)inModel + LittleLong(inModel->ofs_tags));
	outTag = outModel->tags = Mod_Hunk_Alloc(sizeof(md3Tag_t) * outModel->num_frames * outModel->num_tags);

	for (i = 0; i < outModel->num_frames; i++)
	{
		for (l = 0; l < outModel->num_tags; l++, inTag++, outTag++)
		{
			memcpy(outTag->name, inTag->name, MD3_MAX_PATH);
			for (j = 0; j < 3; j++) {
				outTag->orient.origin[j]	= LittleFloat(inTag->orient.origin[j]);
				outTag->orient.axis[0][j]	= LittleFloat(inTag->orient.axis[0][j]);
				outTag->orient.axis[1][j]	= LittleFloat(inTag->orient.axis[1][j]);
				outTag->orient.axis[2][j]	= LittleFloat(inTag->orient.axis[2][j]);
			}
		}
	}

	//
	// load the meshes
	//
	ClearBounds(mod->mins, mod->maxs);
	mod->flags = 0;

	inMesh = (dmd3mesh_t *)((byte *)inModel + LittleLong(inModel->ofs_meshes));
	outMesh = outModel->meshes = Mod_Hunk_Alloc(sizeof(md3Mesh_t) * outModel->num_meshes);

	for (i = 0; i < outModel->num_meshes; i++, outMesh++)
	{
		memcpy(outMesh->name, inMesh->name, MD3_MAX_PATH);

		if (strncmp((const char *)inMesh->id, "IDP3", 4))
		{
			VID_Error(ERR_DROP, "mesh %s in model %s has wrong id (%i should be %i)",
				outMesh->name, mod->name, LittleLong((int)inMesh->id), IDMD3HEADER);
		}

		outMesh->num_tris = LittleLong(inMesh->num_tris);
		outMesh->num_skins = LittleLong(inMesh->num_skins);
		outMesh->num_verts = LittleLong(inMesh->num_verts);

		if (outMesh->num_skins <= 0)
			VID_Error(ERR_DROP, "mesh %i in model %s has no skins", i, mod->name);
		else if (outMesh->num_skins > MD3_MAX_SHADERS)
			VID_Error(ERR_DROP, "mesh %i in model %s has too many skins", i, mod->name);

		if (outMesh->num_tris <= 0)
			VID_Error(ERR_DROP, "mesh %i in model %s has no triangles", i, mod->name);
		else if (outMesh->num_tris > MD3_MAX_TRIANGLES)
			VID_Error(ERR_DROP, "mesh %i in model %s has too many triangles", i, mod->name);

		if (outMesh->num_verts <= 0)
			VID_Error(ERR_DROP, "mesh %i in model %s has no vertices", i, mod->name);
		else if (outMesh->num_verts > MD3_MAX_VERTS)
			VID_Error(ERR_DROP, "mesh %i in model %s has too many vertices, %i (4096 max)", i, mod->name, outMesh->num_verts);

		//
		// register all skins
		//
		inSkin = (dmd3skin_t *)((byte *)inMesh + LittleLong(inMesh->ofs_skins));

		for (j = 0; j < outMesh->num_skins; j++, inSkin++)
		{
			if (!inSkin->name[0])
			{
				outMesh->albedo[j] = 
				outMesh->normalmap[j] = outMesh->emissive[j] =
				outMesh->envmap[j]	= outMesh->pbr[j] = outMesh->aomap[j] = i_missingTexture;
				continue;
			}

			char tex[128];
			memcpy(name, inSkin->name, MD3_MAX_PATH);
			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, ".dds");
			outMesh->albedo[j] = R_LoadDDS(tex, it_skin);
			if (!outMesh->albedo[j])
				outMesh->albedo[j] = i_missingTexture;

			// GlowMaps loading
			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_light.dds");
			outMesh->emissive[j] = R_LoadDDS(tex, it_skin);
			if (!outMesh->emissive[j])
				outMesh->emissive[j] = i_blackTexture1x1;

			// Normal maps loading
			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_bump.dds");
			outMesh->normalmap[j] = R_LoadDDS(tex, it_normal);

			if (!outMesh->normalmap[j])
				outMesh->normalmap[j] = i_defBump;

			// Roughness maps loading
			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_rgh.dds");
			outMesh->pbr[j] = R_LoadDDS(tex, it_skin);
			if (!outMesh->pbr[j])
				outMesh->pbr[j] = i_blackTexture1x1;

			// Env maps loading
			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_env.dds");
			outMesh->envmap[j] = R_LoadDDS(tex, it_skin);
			if (!outMesh->envmap[j])
				outMesh->envmap[j] = i_blackTexture1x1;

			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_ao.dds");
			outMesh->aomap[j] = R_LoadDDS(tex, it_skin);
			if (!outMesh->aomap[j])
				outMesh->aomap[j] = i_whiteMap;
		}

		//
		// load the indexes
		//
		inIndex = (unsigned *)((byte *)inMesh + LittleLong(inMesh->ofs_tris));
		outIndex = outMesh->indexes = (uint16_t*)Mod_Hunk_Alloc(sizeof(uint16_t) * outMesh->num_tris * 3);

		for (j = 0; j < outMesh->num_tris; j++, inIndex += 3, outIndex += 3)
		{
			outIndex[0] = (uint16_t)LittleLong(inIndex[0]);
			outIndex[1] = (uint16_t)LittleLong(inIndex[1]);
			outIndex[2] = (uint16_t)LittleLong(inIndex[2]);
		}

		//
		// load the texture coordinates
		//
		inCoord = (dmd3coord_t *)((byte *)inMesh + LittleLong(inMesh->ofs_tcs));
		outCoord = outMesh->stcoords = Mod_Hunk_Alloc(sizeof(md3ST_t) * outMesh->num_verts);

		for (j = 0; j < outMesh->num_verts; j++, inCoord++, outCoord++)
		{
			outCoord->st[0] = LittleFloat(inCoord->st[0]);
			outCoord->st[1] = LittleFloat(inCoord->st[1]);
		}

		//
		// load all vertexes and calc TBN
		//
		inVerts = (dmd3vertex_t *)((byte *)inMesh + LittleLong(inMesh->ofs_verts));
		outVerts = outMesh->vertexes = Mod_Hunk_Alloc(outModel->num_frames * outMesh->num_verts * sizeof(md3Vertex_t));

		for (l = 0; l < outModel->num_frames; l++){

			// for all frames
			memset(tangents, 0, outMesh->num_verts * sizeof(vec3_t));
			memset(binormals, 0, outMesh->num_verts * sizeof(vec3_t));

			outVerts = outMesh->vertexes + l * outMesh->num_verts;
			
			for (j = 0; j < outMesh->num_verts; j++, inVerts++, outVerts++){

				vec3_t	boundsPoints, norm, translate;
				int		x;

				for (x = 0; x < 3; x++){

					translate[x] = LittleFloat(inFrame->translate[x]);
					outVerts->xyz[x] = (float)LittleShort(inVerts->point[x]) * scaleMD3[x] + translate[x];
					boundsPoints[x] = outVerts->xyz[x] + outModel->frames[l].translate[x];
				}

				AddPointToBounds(boundsPoints, mod->mins, mod->maxs); // add points for bound box

				lat = (float)((inVerts->norm >> 8) & 0xFF) * M_PI / 128.0;
				lng = (float)((inVerts->norm >> 0) & 0xFF) * M_PI / 128.0;
				
				norm[0] = sin(lng) * cos(lat);
				norm[1] = sin(lng) * sin(lat);
				norm[2] = cos(lng);
				
				VectorNormalize(norm);
				VectorCopy(norm, outVerts->normal);
			}

			//for all tris
			outVerts = outMesh->vertexes + l * outMesh->num_verts;
			for (j = 0; j<outMesh->num_tris; j++)
			{
				static vec3_t tangent;
				static vec3_t binormal;

				CalcTangent4MD3(&outMesh->indexes[j * 3], outVerts, outMesh->stcoords, tangent, binormal);
				// for all vertices in the tri
				for (k = 0; k<3; k++){

					m = outMesh->indexes[j * 3 + k];
					VectorAdd(tangents[m], tangent, tangents[m]);
					VectorAdd(binormals[m], binormal, binormals[m]);
				}
			}

			// normalize averages
			for (j = 0; j<outMesh->num_verts; j++)
			{
				VectorNormalize(tangents[j]);
				VectorNormalize(binormals[j]);
				VectorCopy(tangents[j], outVerts[j].tangent);
				VectorCopy(binormals[j], outVerts[j].binormal);
				}
		}

		for (j = 0; j < outMesh->num_verts - 1; j++)
		{
			for (int b = j + 1; b < outMesh->num_verts; b++)
			{
				if (VectorCompareEpsilon(outVerts[j].xyz, outVerts[b].xyz, 0.001f))
				{
					if (DotProduct(outVerts[j].normal, outVerts[b].normal) >= 0.9848)   /// cos 10. Если угол меньше 10 градусов, то сглаживаем.
					{
						VectorAdd(outVerts[j].normal, outVerts[b].normal, outVerts[j].normal);
						VectorCopy(outVerts[j].normal, outVerts[b].normal);
						VectorNormalize(outVerts[j].normal);
						VectorNormalize(outVerts[b].normal);

						VectorAdd(outVerts[j].tangent, outVerts[b].tangent, outVerts[j].tangent);
						VectorCopy(outVerts[j].tangent, outVerts[b].tangent);

						VectorAdd(outVerts[j].binormal, outVerts[b].binormal, outVerts[j].binormal);
						VectorCopy(outVerts[j].binormal, outVerts[b].binormal);
					}
				}
			}
		}
		/// normalize it
		for (j = 0; j < outMesh->num_verts - 1; j++)
		{
			VectorNormalize(outVerts[j].tangent);
			VectorNormalize(outVerts[j].binormal);
		}
		
		//
		// build triangle neighbours
		//
		inMesh = (dmd3mesh_t *)((byte *)inMesh + LittleLong(inMesh->meshsize));
		outMesh->triangles = (neighbours_t*)Mod_Hunk_Alloc(sizeof(neighbours_t) * outMesh->num_tris);
		R_BuildTriangleNeighbors(outMesh->triangles, outMesh->indexes, outMesh->num_tris);

		if (!Q_strcasecmp(outMesh->name, "MF"))
			outMesh->muzzle = true;
		else
			outMesh->muzzle = false;
		
		if (!Q_strcasecmp(outMesh->name, "ALPHATEST")) {
			outMesh->skinAlphatest = true;
		}
		else
			outMesh->skinAlphatest = false;

		outMesh->flags = MESH_OPAQUE;

		if (!Q_strcasecmp(outMesh->name, "TRANSLUS")) 
			outMesh->flags = MESH_TRANSLUSCENT;

		if (!Q_strcasecmp(outMesh->name, "MESH_SSS"))
			outMesh->flags = MESH_SSS;

		if (!Q_strcasecmp(outMesh->name, "MESH_ALPHATEST"))
			outMesh->flags = MESH_ALPHATEST;
}

	mod->type = mod_alias_md3;

	/// Calc md3 bounds and radius...
	vec3_t	tempr, tempv;
	tempr[0] = mod->maxs[0] - mod->mins[0];
	tempr[1] = mod->maxs[1] - mod->mins[1];
	tempr[2] = 0;
	tempv[0] = 0;
	tempv[1] = 0;
	tempv[2] = mod->maxs[2] - mod->mins[2];
	mod->radius = max(VectorLength(tempr), VectorLength(tempv));

	for (i = 0; i<3; i++)
		mod->center[i] = (mod->maxs[i] + mod->mins[i]) * 0.5;
}


/*
=================
R_CullAliasModel
=================
*/
bool R_CullMD3Model(vec3_t bbox[8], entity_t *e)
{
	int			i, j;
	vec3_t		mins, maxs, tmp; //angles;
	vec3_t		vectors[3];
	md3Model_t	*md3Hdr;
	md3Frame_t	*currFrame, *oldFrame;
	int			mask, aggregatemask = ~0;

	md3Hdr = (md3Model_t *)currentmodel->extraData;

	if ((e->frame >= md3Hdr->num_frames) || (e->frame < 0))
	{
		Com_DPrintf("R_Cullmd3Model %s: no such frame %d\n", currentmodel->name, e->frame);
		e->frame = 0;
	}
	if ((e->oldFrame >= md3Hdr->num_frames) || (e->oldFrame < 0))
	{
		Com_DPrintf("R_Cullmd3Model %s: no such oldFrame %d\n", currentmodel->name, e->oldFrame);
		e->oldFrame = 0;
	}

	currFrame = md3Hdr->frames + e->frame;
	oldFrame = md3Hdr->frames + e->oldFrame;

	// compute axially aligned mins and maxs
	if (currFrame == oldFrame)
	{
		VectorCopy(currFrame->mins, mins);
		VectorCopy(currFrame->maxs, maxs);
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			if (currFrame->mins[i] < oldFrame->mins[i])
				mins[i] = currFrame->mins[i];
			else
				mins[i] = oldFrame->mins[i];

			if (currFrame->maxs[i] > oldFrame->maxs[i])
				maxs[i] = currFrame->maxs[i];
			else
				maxs[i] = oldFrame->maxs[i];
		}
	}

	// Compute and rotate bonding box
	AngleVectors(e->angles, vectors[0], vectors[1], vectors[2]);
	VectorSubtract(vec3_origin, vectors[1], vectors[1]); // AngleVectors returns "right" instead of "left"

	for (i = 0; i < 8; i++) {
		tmp[0] = ((i & 1) ? mins[0] : maxs[0]);
		tmp[1] = ((i & 2) ? mins[1] : maxs[1]);
		tmp[2] = ((i & 4) ? mins[2] : maxs[2]);

		bbox[i][0] = vectors[0][0] * tmp[0] + vectors[1][0] * tmp[1] + vectors[2][0] * tmp[2] + e->origin[0];
		bbox[i][1] = vectors[0][1] * tmp[0] + vectors[1][1] * tmp[1] + vectors[2][1] * tmp[2] + e->origin[1];
		bbox[i][2] = vectors[0][2] * tmp[0] + vectors[1][2] * tmp[1] + vectors[2][2] * tmp[2] + e->origin[2];
	}

	// cull
	for (i = 0; i< 8; i++)
	{
		mask = 0;
		for (j = 0; j<6; j++)
		{
			float dp = DotProduct(frustum[j].normal, bbox[i]);
			if ((dp - frustum[j].dist) < 0)
				mask |= (1 << j);
		}

		aggregatemask &= mask;
	}

	if (aggregatemask)
		return true;

	return false;
}

void CheckEntityFrameMD3(md3Model_t *paliashdr)
{
	if ((currententity->frame >= paliashdr->num_frames) || (currententity->frame < 0))
	{
		Com_Printf("^3CheckEntityFrameMD3, %s: no such frame %d\n", currentmodel->name, currententity->frame);
		currententity->frame = 0;
	}

	if ((currententity->oldFrame >= paliashdr->num_frames) || (currententity->oldFrame < 0))
	{
		Com_Printf("^3CheckEntityFrameMD3, %s: no such oldFrame %d\n", currentmodel->name, currententity->oldFrame);
		currententity->oldFrame = 0;
	}

}

void R_DrawMD3Mesh(bool weapon) {

	md3Model_t	*md3Hdr;
	vec3_t		bbox[8], temp, viewOrg;
	int			i, j, k;
	float		frontlerp, backlerp, lum;
	md3Frame_t	*frame, *oldFrame;
	vec3_t		move, delta, vectors[3];
	md3Vertex_t	*verts, *oldVerts;
	vec3_t		luminance = { 0.2125, 0.7154, 0.0721 };
	image_t     *albedo, *emissive, *normal, *ao;
	bool	noLerp = false;

	if (!r_drawEntities->integer)
		return;

	if (currententity->flags & RF_WEAPONMODEL)
		if (!weapon || r_leftHand->integer == 2)
			return;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	if (R_CullMD3Model(bbox, currententity))
		return;

	if (currententity->flags & RF_NOCULL)
		GL_Disable(GL_CULL_FACE);

	SetModelsLight();

	if (r_skipStaticLights->integer) {

		if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
			VectorSet(shadelight, 0.5, 0.5, 0.5);
	}
	else {
		if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
			VectorSet(shadelight, 0.01, 0.01, 0.01);
	}

	if (r_newrefdef.rdflags & RDF_IRGOGGLES)
		VectorSet(shadelight, 1, 1, 1);
	
	if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD | RF_FULLBRIGHT))
		VectorSet(shadelight, 1, 1, 1);

	if (currententity->flags & RF_MINLIGHT) {
		for (i = 0; i < 3; i++)
			if (shadelight[i] > 0.0019)
				break;
		if (i == 3) {
			shadelight[0] = 0.0019;
			shadelight[1] = 0.0019;
			shadelight[2] = 0.0019;
		}
	}
	
	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		GL_DepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));

	md3Hdr = (md3Model_t *)currentmodel->extraData;
	CheckEntityFrameMD3(md3Hdr);

	backlerp	= currententity->backLerp;
	frontlerp	= 1.0 - backlerp;
	frame		= md3Hdr->frames + currententity->frame;
	oldFrame	= md3Hdr->frames + currententity->oldFrame;

	VectorSubtract(currententity->oldOrigin, currententity->origin, delta);
	AngleVectors(currententity->angles, vectors[0], vectors[1], vectors[2]);
	move[0] = DotProduct(delta, vectors[0]);	// forward
	move[1] = -DotProduct(delta, vectors[1]);	// left
	move[2] = DotProduct(delta, vectors[2]);	// up

	VectorAdd(move, oldFrame->translate, move);

	for (j = 0; j<3; j++)
		move[j] = backlerp * move[j] + frontlerp * frame->translate[j];

	R_SetupEntityMatrix(currententity);

	GL_BindVAO(vao.stream3d);
	GL_BindVBO(vbo.stream3d);
	GL_BindVBO(vbo.dynamicIbo);

	// setup program
	GL_BindProgram(md3AmbientProgram);

	qglUniform1i(U_ENV_PASS, 0);
	qglUniform1i(U_SHELL_PASS, 0);
	qglUniform1i(U_TRANS_PASS, 0);

	float alphaShift = sin(ref_realtime * 5.666);
	alphaShift = (alphaShift + 3.0) * 0.5f;
	alphaShift = clamp(alphaShift, 0.01, 6.0);

	qglUniform1f(U_COLOR_OFFSET, alphaShift);
	qglUniform1f(U_ENV_SCALE, 0.1); 
	if (currententity->flags & RF_EMISSIVECOLOR)
		qglUniform3f(U_COLOR, currententity->addColor[0]* 2.0, currententity->addColor[1] * 2.0, currententity->addColor[2] * 2.0); // modificate emmission color
	else
		qglUniform3f(U_COLOR, 1.0, 1.0, 1.0);

	VectorSubtract(r_origin, currententity->origin, temp);
	Mat3_TransposeMultiplyVector(currententity->axis, temp, viewOrg);

	qglUniform3fv(U_VIEW_POS, 1, viewOrg);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float *)currententity->orMatrix);
	
	qglUniform1i(U_USE_SSAO, 1);

	if (!r_ssao->integer)
		qglUniform1i(U_USE_SSAO, 0);
	if(currententity->flags & RF_WEAPONMODEL)
		qglUniform1i(U_USE_SSAO, 0);
	if(r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		qglUniform1i(U_USE_SSAO, 0);
	if(r_newrefdef.rdflags & RDF_IRGOGGLES)
		qglUniform1i(U_USE_SSAO, 0);

	if (currententity->flags & RF_WEAPONMODEL)
		GL_DepthMask(1);
	else
		GL_DepthMask(0);

	if ((r_newrefdef.rdflags & RDF_NOWORLDMODEL) && !gl_config.useHdrDisplay)
		qglUniform1i(U_PARAM_INT_1, 1);
	else
		qglUniform1i(U_PARAM_INT_1, 0);
	
	if (frame == oldFrame)
		noLerp = true;

	for (i = 0; i < md3Hdr->num_meshes; i++) {

		md3Mesh_t *mesh = &md3Hdr->meshes[i];

		if (!(mesh->flags & MESH_OPAQUE)) 
			continue;

		verts		= mesh->vertexes + currententity->frame		* mesh->num_verts;
		oldVerts	= mesh->vertexes + currententity->oldFrame	* mesh->num_verts;
		
		c_aliasTris += md3Hdr->meshes[i].num_tris;

		if (mesh->muzzle) {
			GL_Enable(GL_BLEND);
			GL_DepthMask(0);
			qglUniform1f(U_COLOR_OFFSET, 1.0);
			qglUniform1f(U_COLOR_MUL, 4.0);
			qglUniform1i(U_USE_SSAO, 0);
			GL_BlendFunc(GL_ONE, GL_ONE);
		}
		if (currententity->flags & RF_WEAPONMODEL)
			GL_DepthMask(1);

		albedo = mesh->albedo[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		if (!albedo || albedo == i_missingTexture){

			if (currententity->skin){
				albedo = currententity->skin;	// custom player skin
			}
		}
		if (!albedo)
			albedo = i_missingTexture;

		emissive = mesh->emissive[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		if (!emissive)
			emissive = i_blackTexture1x1;

		normal = mesh->normalmap[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		if (!normal)
			normal = i_defBump;
		
		if (currententity->flags & RF_WEAPONMODEL && r_ssao->integer)
			ao = mesh->pbr[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		else
			ao = i_whiteMap;

		for (j = 0; j < mesh->num_verts; j++, verts++, oldVerts++) {

			if (mesh->muzzle)
				Vector4Set(tess3d.v[j].color, 1.0, 1.0, 1.0, 1.0);
			else
				Vector4Set(tess3d.v[j].color, shadelight[0], shadelight[1], shadelight[2], 1.0);

			tess3d.v[j].tc[0] = mesh->stcoords[j].st[0];
			tess3d.v[j].tc[1] = mesh->stcoords[j].st[1];

			if (noLerp || md3Hdr->num_frames < 1) {
				tess3d.v[j].pos[0] = move[0] + verts->xyz[0];
				tess3d.v[j].pos[1] = move[1] + verts->xyz[1];
				tess3d.v[j].pos[2] = move[2] + verts->xyz[2];
			}
			else {
				tess3d.v[j].pos[0] = move[0] + oldVerts->xyz[0] * backlerp + verts->xyz[0] * frontlerp;
				tess3d.v[j].pos[1] = move[1] + oldVerts->xyz[1] * backlerp + verts->xyz[1] * frontlerp;
				tess3d.v[j].pos[2] = move[2] + oldVerts->xyz[2] * backlerp + verts->xyz[2] * frontlerp;
			}
		}
		
		if (r_debugTbn->integer && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {
			verts		= mesh->vertexes + currententity->frame * mesh->num_verts;
			oldVerts	= mesh->vertexes + currententity->oldFrame * mesh->num_verts;

			for (k = 0; k < mesh->num_verts; k++) {

				if (noLerp || md3Hdr->num_frames < 1) {
					tess3d.v[k].tangent[0]	= verts[k].tangent[0];
					tess3d.v[k].tangent[1]	= verts[k].tangent[1];
					tess3d.v[k].tangent[2]	= verts[k].tangent[2];

					tess3d.v[k].binormal[0] = verts[k].binormal[0];
					tess3d.v[k].binormal[1] = verts[k].binormal[1];
					tess3d.v[k].binormal[2] = verts[k].binormal[2];

					tess3d.v[k].normal[0]	= verts[k].normal[0];
					tess3d.v[k].normal[1]	= verts[k].normal[1];
					tess3d.v[k].normal[2]	= verts[k].normal[2];
				}
				else {
					tess3d.v[k].tangent[0]	= verts[k].tangent[0] * frontlerp + oldVerts[k].tangent[0] * backlerp;
					tess3d.v[k].tangent[1]	= verts[k].tangent[1] * frontlerp + oldVerts[k].tangent[1] * backlerp;
					tess3d.v[k].tangent[2]	= verts[k].tangent[2] * frontlerp + oldVerts[k].tangent[2] * backlerp;

					tess3d.v[k].binormal[0] = verts[k].binormal[0] * frontlerp + oldVerts[k].binormal[0] * backlerp;
					tess3d.v[k].binormal[1] = verts[k].binormal[1] * frontlerp + oldVerts[k].binormal[1] * backlerp;
					tess3d.v[k].binormal[2] = verts[k].binormal[2] * frontlerp + oldVerts[k].binormal[2] * backlerp;

					tess3d.v[k].normal[0]	= verts[k].normal[0] * frontlerp + oldVerts[k].normal[0] * backlerp;
					tess3d.v[k].normal[1]	= verts[k].normal[1] * frontlerp + oldVerts[k].normal[1] * backlerp;
					tess3d.v[k].normal[2]	= verts[k].normal[2] * frontlerp + oldVerts[k].normal[2] * backlerp;
				}
			}
		}

		qglInvalidateBufferData(GL_ARRAY_BUFFER);
		qglInvalidateBufferData(GL_ELEMENT_ARRAY_BUFFER);
		qglBufferSubData(GL_ARRAY_BUFFER, 0, mesh->num_verts * sizeof(vertex3d_t), &tess3d);
		qglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mesh->num_tris * 3 * sizeof(uint16_t), mesh->indexes);

		GL_SetBindlessTexture(U_TMU0, albedo->handle);
		GL_SetBindlessTexture(U_TMU1, emissive->handle);
		GL_SetBindlessTexture(U_TMU2, i_environment->handle);
		GL_SetBindlessTexture(U_TMU3, normal->handle);
		GL_SetBindlessTexture(U_TMU4, i_ssaoColor[i_ssaoColorIndex]->handle);
		GL_SetBindlessTexture(U_TMU5, ao->handle);

		GL_DrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_SHORT, NULL);
	
		if (r_debugTbn->integer && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {
			GL_BindProgram(tbnDebugProgram);
			qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float*)currententity->orMatrix);
			if (currententity->flags & (RF_WEAPONMODEL))
				qglUniform1f(U_PARAM_FLOAT_0, 0.3);
			else
				qglUniform1f(U_PARAM_FLOAT_0, r_debugTbnLen->value);
			GL_DrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_SHORT, NULL);
			GL_BindProgram(md3AmbientProgram);
		}

		if (r_showTris->integer && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {

			GL_Disable(GL_DEPTH_TEST);
			qglLineWidth(1.5);
			qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			GL_BindProgram(showTrisProgram);
			qglUniform3f(U_COLOR, 1.0, 1.0, 0.0);
			qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float *)currententity->orMatrix);

			GL_DrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_SHORT, NULL);
			GL_BindProgram(md3AmbientProgram);

			qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			GL_Enable(GL_DEPTH_TEST);
		}

		if (mesh->muzzle) {
			GL_Disable(GL_BLEND);
			GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	}

		// Draw Transluscent meshes
		if (r_newrefdef.rdflags & RDF_IRGOGGLES)
			return;

		GL_Enable(GL_BLEND);
		GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GL_DepthMask(0);
		qglUniform1i(U_ENV_PASS, 1);
		qglUniform1i(U_TRANS_PASS, 1);
		qglUniform1i(U_USE_SSAO, 0);
		qglUniform1i(U_SHELL_PASS, 0);

		lum = DotProduct(luminance, shadelight);
		qglUniform1f(U_ENV_SCALE, lum);

		for (i = 0; i < md3Hdr->num_meshes; i++) {

			md3Mesh_t *mesh = &md3Hdr->meshes[i];
			verts		= mesh->vertexes + currententity->frame * mesh->num_verts;
			oldVerts	= mesh->vertexes + currententity->oldFrame * mesh->num_verts;

			c_aliasTris += md3Hdr->meshes[i].num_tris;

			if (mesh->muzzle)
				continue;

			if (!(mesh->flags & MESH_TRANSLUSCENT))
				continue;

			albedo = mesh->albedo[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
			if (!albedo || albedo == i_missingTexture){

				if (currententity->skin){
					albedo = currententity->skin;	// custom player skin
				}
			}
			if (!albedo)
				albedo = i_missingTexture;

			normal = mesh->normalmap[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
			if (!normal)
				normal = i_defBump;

			for (j = 0; j < mesh->num_verts; j++, verts++, oldVerts++) {
				
				if (mesh->muzzle)
					Vector4Set(tess3d.v[j].color, 1.0, 1.0, 1.0, 1.0);
				else
					Vector4Set(tess3d.v[j].color, shadelight[0], shadelight[1], shadelight[2], 0.5);

				tess3d.v[j].tc[0] = mesh->stcoords[j].st[0];
				tess3d.v[j].tc[1] = mesh->stcoords[j].st[1];

				if (noLerp || md3Hdr->num_frames < 1) {
					tess3d.v[j].pos[0] = move[0] + verts->xyz[0];
					tess3d.v[j].pos[1] = move[1] + verts->xyz[1];
					tess3d.v[j].pos[2] = move[2] + verts->xyz[2];
				}
				else {
					tess3d.v[j].pos[0] = move[0] + oldVerts->xyz[0] * backlerp + verts->xyz[0] * frontlerp;
					tess3d.v[j].pos[1] = move[1] + oldVerts->xyz[1] * backlerp + verts->xyz[1] * frontlerp;
					tess3d.v[j].pos[2] = move[2] + oldVerts->xyz[2] * backlerp + verts->xyz[2] * frontlerp;
				}
			}

			verts		= mesh->vertexes + currententity->frame * mesh->num_verts;
			oldVerts	= mesh->vertexes + currententity->oldFrame * mesh->num_verts;

			for (k = 0; k< mesh->num_verts; k++) {
				
				if (noLerp || md3Hdr->num_frames < 1) {
					tess3d.v[k].tangent[0] = verts[k].tangent[0];
					tess3d.v[k].tangent[1] = verts[k].tangent[1];
					tess3d.v[k].tangent[2] = verts[k].tangent[2];

					tess3d.v[k].binormal[0] = verts[k].binormal[0];
					tess3d.v[k].binormal[1] = verts[k].binormal[1];
					tess3d.v[k].binormal[2] = verts[k].binormal[2];

					tess3d.v[k].normal[0] = verts[k].normal[0];
					tess3d.v[k].normal[1] = verts[k].normal[1];
					tess3d.v[k].normal[2] = verts[k].normal[2];
				}
				else {
					tess3d.v[k].tangent[0] = verts[k].tangent[0] * frontlerp + oldVerts[k].tangent[0] * backlerp;
					tess3d.v[k].tangent[1] = verts[k].tangent[1] * frontlerp + oldVerts[k].tangent[1] * backlerp;
					tess3d.v[k].tangent[2] = verts[k].tangent[2] * frontlerp + oldVerts[k].tangent[2] * backlerp;

					tess3d.v[k].binormal[0] = verts[k].binormal[0] * frontlerp + oldVerts[k].binormal[0] * backlerp;
					tess3d.v[k].binormal[1] = verts[k].binormal[1] * frontlerp + oldVerts[k].binormal[1] * backlerp;
					tess3d.v[k].binormal[2] = verts[k].binormal[2] * frontlerp + oldVerts[k].binormal[2] * backlerp;

					tess3d.v[k].normal[0] = verts[k].normal[0] * frontlerp + oldVerts[k].normal[0] * backlerp;
					tess3d.v[k].normal[1] = verts[k].normal[1] * frontlerp + oldVerts[k].normal[1] * backlerp;
					tess3d.v[k].normal[2] = verts[k].normal[2] * frontlerp + oldVerts[k].normal[2] * backlerp;
				}
			}

			GL_SetBindlessTexture(U_TMU0, albedo->handle);
			GL_SetBindlessTexture(U_TMU1, i_blackTexture1x1->handle);
			GL_SetBindlessTexture(U_TMU2, i_environment->handle);
			GL_SetBindlessTexture(U_TMU3, normal->handle);

			qglInvalidateBufferData(GL_ARRAY_BUFFER);
			qglInvalidateBufferData(GL_ELEMENT_ARRAY_BUFFER);
			qglBufferSubData(GL_ARRAY_BUFFER, 0, mesh->num_verts * sizeof(vertex3d_t), &tess3d);
			qglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mesh->num_tris * 3 * sizeof(uint16_t), mesh->indexes);

			GL_DrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_SHORT, NULL);

			if (r_debugTbn->integer && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {
				GL_Disable(GL_BLEND);
				GL_BindProgram(tbnDebugProgram);
				qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float*)currententity->orMatrix);
				if (currententity->flags & (RF_WEAPONMODEL))
					qglUniform1f(U_PARAM_FLOAT_0, 0.3);
				else
					qglUniform1f(U_PARAM_FLOAT_0, r_debugTbnLen->value);
				GL_DrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_SHORT, NULL);
				GL_BindProgram(md3AmbientProgram);
			}

			if (r_showTris->integer && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {

				GL_Disable(GL_DEPTH_TEST);
				qglLineWidth(1.5);
				qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				GL_BindProgram(showTrisProgram);
				qglUniform3f(U_COLOR, 1.0, 1.0, 0.5);
				qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float *)currententity->orMatrix);

				GL_DrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_SHORT, NULL);
				GL_BindProgram(md3AmbientProgram);

				qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				GL_Enable(GL_DEPTH_TEST);
			}
		}
		GL_Disable(GL_BLEND);

	if (currententity->flags & RF_DEPTHHACK)
		GL_DepthRange(gldepthmin, gldepthmax);

	if (currententity->flags & RF_NOCULL)
		GL_Enable(GL_CULL_FACE);
}

bool R_Md3InLightBound() {

	vec3_t mins, maxs;
	int i;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	}
	else {
		VectorAdd(currententity->origin, currententity->model->maxs, maxs);
		VectorAdd(currententity->origin, currententity->model->mins, mins);
	}

	if (currentShadowLight->projector) {

		if (R_CullConeLight(mins, maxs, currentShadowLight->frust))
			return false;
	}
	else if (currentShadowLight->spherical) {

		if (!BoundsAndSphereIntersect(mins, maxs, currentShadowLight->origin, currentShadowLight->radius[0]))
			return false;
	}
	else {

		if (!BoundsIntersect(mins, maxs, currentShadowLight->mins, currentShadowLight->maxs))
			return false;
	}

	if (!InLightVISEntity())
		return false;

	return true;

}

void R_UpdateLightAliasUniforms();

void R_DrawMD3MeshLight(bool weapon) {

	md3Model_t	*md3Hdr;
	vec3_t		bbox[8];
	int			i, j, k;
	float		frontlerp, backlerp;
	md3Frame_t	*frame, *oldFrame;
	vec3_t		move, delta, vectors[3], maxs;
	md3Vertex_t	*verts, *oldVerts;
	image_t     *albedo, *pbr, *normal;
	bool	inWater, noLerp = false;
	vec3_t		tmp, oldLight, oldView;

	if (!r_drawEntities->integer)
		return;

	if (currententity->flags & RF_WEAPONMODEL)
		if (!weapon || r_leftHand->integer == 2)
			return;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	if (!(currententity->flags & RF_WEAPONMODEL)){

		if (R_CullMD3Model(bbox, currententity))
			return;
	}

	if (!R_Md3InLightBound())
		return;

	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		GL_DepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));

	R_SetupEntityMatrix(currententity);

	VectorCopy(currentShadowLight->origin, oldLight);
	VectorCopy(r_origin, oldView);

	VectorSubtract(currentShadowLight->origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, currentShadowLight->origin);

	VectorSubtract(r_origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, r_origin);

	md3Hdr = (md3Model_t *)currentmodel->extraData;
	CheckEntityFrameMD3(md3Hdr);

	backlerp	= currententity->backLerp;
	frontlerp	= 1.0 - backlerp;
	frame		= md3Hdr->frames + currententity->frame;
	oldFrame	= md3Hdr->frames + currententity->oldFrame;

	VectorSubtract(currententity->oldOrigin, currententity->origin, delta);
	AngleVectors(currententity->angles, vectors[0], vectors[1], vectors[2]);
	move[0] = DotProduct(delta, vectors[0]);	// forward
	move[1] = -DotProduct(delta, vectors[1]);	// left
	move[2] = DotProduct(delta, vectors[2]);	// up

	VectorAdd(move, oldFrame->translate, move);

	for (j = 0; j<3; j++)
		move[j] = backlerp * move[j] + frontlerp * frame->translate[j];

	GL_BindVAO(vao.stream3d);
	GL_BindVBO(vbo.stream3d);
	GL_BindVBO(vbo.dynamicIbo);

	GL_StencilFunc(GL_EQUAL, 128, 255);
	GL_StencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	GL_StencilMask(0);
	GL_DepthFunc(GL_LEQUAL);

	GL_PolygonOffset(-1.0, -1.0);

	// setup program
	GL_BindProgram(aliasBumpProgram);

	VectorAdd(currententity->origin, currententity->model->maxs, maxs);
	if (CL_PMpointcontents(maxs) & MASK_WATER)
		inWater = true;
	else
		inWater = false;

	R_UpdateLightAliasUniforms();

	qglUniform1i(U_USE_AUTOBUMP, 0);
	
	if ((inWater && currentShadowLight->castCaustics && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) 
		|| (!inWater && currentShadowLight->castCaustics2 && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)))
		qglUniform1i(U_USE_CAUSTICS, 1);
	else
		qglUniform1i(U_USE_CAUSTICS, 0);

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		qglUniform1i(U_PARAM_INT_4, 0);

	if (frame == oldFrame)
		noLerp = true;
	
	for (i = 0; i < md3Hdr->num_meshes; i++) {

		md3Mesh_t *mesh = &md3Hdr->meshes[i];
		verts		= mesh->vertexes + currententity->frame * mesh->num_verts;
		oldVerts	= mesh->vertexes + currententity->oldFrame * mesh->num_verts;
		
		if (mesh->muzzle)
			continue;

		if (r_ssao->integer && !(r_newrefdef.rdflags & RDF_IRGOGGLES) && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)&& !(currententity->flags & RF_WEAPONMODEL))
			qglUniform1i(U_USE_SSAO, 1);
		else
			qglUniform1i(U_USE_SSAO, 0);

		if (mesh->flags & MESH_SSS) {
			qglUniform1i(U_PARAM_INT_2, 1);
			qglUniform1i(U_PARAM_INT_4, 0);
		}
		else
			qglUniform1i(U_PARAM_INT_2, 0);

		if (mesh->skinAlphatest)
			qglUniform1i(U_PARAM_INT_1, 1);
		else
			qglUniform1i(U_PARAM_INT_1, 0);
		
		if ((r_newrefdef.rdflags & RDF_NOWORLDMODEL) && !gl_config.useHdrDisplay)
			qglUniform1i(U_PARAM_INT_5, 1);
		else
			qglUniform1i(U_PARAM_INT_5, 0);

		c_litAliasTris += md3Hdr->meshes[i].num_tris;

		albedo = mesh->albedo[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		if (!albedo || albedo == i_missingTexture){

			if (currententity->skin){
				albedo = currententity->skin;	// custom player skin
			}
		}
		if (!albedo)
			albedo = i_missingTexture;

		normal = mesh->normalmap[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		if (!normal)
			normal = i_defBump;

		pbr = mesh->pbr[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		if (!pbr)
			pbr = i_blackTexture1x1;

		for (j = 0; j < mesh->num_verts; j++, verts++, oldVerts++) {

			tess3d.v[j].tc[0] = mesh->stcoords[j].st[0];
			tess3d.v[j].tc[1] = mesh->stcoords[j].st[1];

			if (noLerp || md3Hdr->num_frames < 1) {
				tess3d.v[j].pos[0] = move[0] + verts->xyz[0];
				tess3d.v[j].pos[1] = move[1] + verts->xyz[1];
				tess3d.v[j].pos[2] = move[2] + verts->xyz[2];
			}
			else {
				tess3d.v[j].pos[0] = move[0] + oldVerts->xyz[0] * backlerp + verts->xyz[0] * frontlerp;
				tess3d.v[j].pos[1] = move[1] + oldVerts->xyz[1] * backlerp + verts->xyz[1] * frontlerp;
				tess3d.v[j].pos[2] = move[2] + oldVerts->xyz[2] * backlerp + verts->xyz[2] * frontlerp;
			}
		}

		verts = mesh->vertexes + currententity->frame * mesh->num_verts;
		oldVerts = mesh->vertexes + currententity->oldFrame * mesh->num_verts;

		for (k = 0; k < mesh->num_verts; k++) {

			if (noLerp || md3Hdr->num_frames < 1) {
				tess3d.v[k].tangent[0] = verts[k].tangent[0];
				tess3d.v[k].tangent[1] = verts[k].tangent[1];
				tess3d.v[k].tangent[2] = verts[k].tangent[2];

				tess3d.v[k].binormal[0] = verts[k].binormal[0];
				tess3d.v[k].binormal[1] = verts[k].binormal[1];
				tess3d.v[k].binormal[2] = verts[k].binormal[2];

				tess3d.v[k].normal[0] = verts[k].normal[0];
				tess3d.v[k].normal[1] = verts[k].normal[1];
				tess3d.v[k].normal[2] = verts[k].normal[2];
			}
			else {
				tess3d.v[k].tangent[0] = verts[k].tangent[0] * frontlerp + oldVerts[k].tangent[0] * backlerp;
				tess3d.v[k].tangent[1] = verts[k].tangent[1] * frontlerp + oldVerts[k].tangent[1] * backlerp;
				tess3d.v[k].tangent[2] = verts[k].tangent[2] * frontlerp + oldVerts[k].tangent[2] * backlerp;

				tess3d.v[k].binormal[0] = verts[k].binormal[0] * frontlerp + oldVerts[k].binormal[0] * backlerp;
				tess3d.v[k].binormal[1] = verts[k].binormal[1] * frontlerp + oldVerts[k].binormal[1] * backlerp;
				tess3d.v[k].binormal[2] = verts[k].binormal[2] * frontlerp + oldVerts[k].binormal[2] * backlerp;

				tess3d.v[k].normal[0] = verts[k].normal[0] * frontlerp + oldVerts[k].normal[0] * backlerp;
				tess3d.v[k].normal[1] = verts[k].normal[1] * frontlerp + oldVerts[k].normal[1] * backlerp;
				tess3d.v[k].normal[2] = verts[k].normal[2] * frontlerp + oldVerts[k].normal[2] * backlerp;
			}
		}
		
		GL_SetBindlessTexture(U_TMU0, normal->handle);
		GL_SetBindlessTexture(U_TMU1, albedo->handle);
		GL_SetBindlessTexture(U_TMU2, r_caustic[((int)(r_newrefdef.time * 15)) & (MAX_CAUSTICS - 1)]->handle);
		GL_SetBindlessTexture(U_TMU3, r_lightCubeMap[currentShadowLight->filter]->handle);
		GL_SetBindlessTexture(U_TMU4, pbr->handle);
		GL_SetBindlessTexture(U_TMU5, i_skinBump->handle);
		GL_SetBindlessTexture(U_TMU6, i_hdrBaseInterim->handle);
		GL_SetBindlessTexture(U_TMU7, i_linearDepth->handle);
		GL_SetBindlessTexture(U_TMU8, i_ssaoColor[i_ssaoColorIndex]->handle);

		qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
		qglUniformMatrix4fv(U_PROJ_MATRIX, 1, false, (const float*)r_newrefdef.projectionMatrix);

		if (pbr == i_blackTexture1x1)
			qglUniform1i(U_USE_RGH_MAP, 0);
		else {
			qglUniform1i(U_USE_RGH_MAP, 1);
		}

		qglInvalidateBufferData(GL_ARRAY_BUFFER);
		qglInvalidateBufferData(GL_ELEMENT_ARRAY_BUFFER);
		qglBufferSubData(GL_ARRAY_BUFFER, 0, mesh->num_verts * sizeof(vertex3d_t), &tess3d);
		qglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mesh->num_tris * 3 * sizeof(uint16_t), mesh->indexes);

		GL_DrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_SHORT, NULL);
	}

	VectorCopy(oldLight, currentShadowLight->origin);
	VectorCopy(oldView, r_origin);

	if (currententity->flags & RF_DEPTHHACK)
		GL_DepthRange(gldepthmin, gldepthmax);
}


void R_DrawMD3ShellMesh(bool weapon) {

	md3Model_t		*md3Hdr;
	vec3_t			bbox[8];
	int				i, j;
	float			frontlerp, backlerp;
	md3Frame_t		*frame, *oldFrame;
	vec3_t			move, delta, vectors[3], tmp, viewOrg;
	md3Vertex_t		*verts, *oldVerts;
	bool		noLerp = false;

	if (!r_drawEntities->integer)
		return;

	if (currententity->flags & RF_WEAPONMODEL)
		if (!weapon || r_leftHand->integer == 2)
			return;

	if (R_CullMD3Model(bbox, currententity))
		return;

	md3Hdr = (md3Model_t *)currentmodel->extraData;

	CheckEntityFrameMD3(md3Hdr);

	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		GL_DepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));

	backlerp	= currententity->backLerp;
	frontlerp	= 1.0 - backlerp;
	frame		= md3Hdr->frames + currententity->frame;
	oldFrame	= md3Hdr->frames + currententity->oldFrame;

	VectorSubtract(currententity->oldOrigin, currententity->origin, delta);
	AngleVectors(currententity->angles, vectors[0], vectors[1], vectors[2]);
	move[0] = DotProduct(delta, vectors[0]);	// forward
	move[1] = -DotProduct(delta, vectors[1]);	// left
	move[2] = DotProduct(delta, vectors[2]);	// up

	VectorAdd(move, oldFrame->translate, move);

	for (j = 0; j<3; j++)
		move[j] = backlerp * move[j] + frontlerp * frame->translate[j];

	R_SetupEntityMatrix(currententity);

	VectorSubtract(r_origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, viewOrg);

	GL_BindVAO(vao.stream3d);
	GL_BindVBO(vbo.stream3d);
	GL_BindVBO(vbo.dynamicIbo);

	// setup program
	GL_BindProgram(md3AmbientProgram);
	GL_BlendFunc(GL_SRC_COLOR, GL_ONE);
	vec2_t shellParams = { r_newrefdef.time * 0.45, 0.0 };

	qglUniform1i(U_SHELL_PASS, 1); // deform in vertex shader
	qglUniform3fv(U_VIEW_POS, 1, viewOrg);
	qglUniform2fv(U_SHELL_PARAMS, 1, shellParams);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float *)currententity->orMatrix);
	
	if (currententity->flags & RF_SHELL_BLUE)
		GL_SetBindlessTexture(U_TMU0, r_texshell[0]->handle);
	if (currententity->flags & RF_SHELL_RED)
		GL_SetBindlessTexture(U_TMU0, r_texshell[1]->handle);
	if (currententity->flags & RF_SHELL_GREEN)
		GL_SetBindlessTexture(U_TMU0, r_texshell[2]->handle);
	if (currententity->flags & RF_SHELL_GOD)
		GL_SetBindlessTexture(U_TMU0, r_texshell[3]->handle);
	if (currententity->flags & RF_SHELL_HALF_DAM)
		GL_SetBindlessTexture(U_TMU0, r_texshell[4]->handle);
	if (currententity->flags & RF_SHELL_DOUBLE)
		GL_SetBindlessTexture(U_TMU0, r_texshell[5]->handle);
	
	if (frame == oldFrame)
		noLerp = true;

	for (i = 0; i < md3Hdr->num_meshes; i++) {

		md3Mesh_t *mesh = &md3Hdr->meshes[i];

		if (mesh->muzzle)
			continue;

		c_aliasTris += md3Hdr->meshes[i].num_tris;
		verts		= mesh->vertexes + currententity->frame * mesh->num_verts;
		oldVerts	= mesh->vertexes + currententity->oldFrame * mesh->num_verts;

		for (j = 0; j < mesh->num_verts; j++, verts++, oldVerts++) {

			if (noLerp || md3Hdr->num_frames < 1) {
				tess3d.v[j].pos[0] = move[0] + verts->xyz[0];
				tess3d.v[j].pos[1] = move[1] + verts->xyz[1];
				tess3d.v[j].pos[2] = move[2] + verts->xyz[2];
			}
			else {
				tess3d.v[j].pos[0] = move[0] + oldVerts->xyz[0] * backlerp + verts->xyz[0] * frontlerp;
				tess3d.v[j].pos[1] = move[1] + oldVerts->xyz[1] * backlerp + verts->xyz[1] * frontlerp;
				tess3d.v[j].pos[2] = move[2] + oldVerts->xyz[2] * backlerp + verts->xyz[2] * frontlerp;
			}

			if(noLerp || md3Hdr->num_frames < 1){
				tess3d.v[j].normal[0] = verts->normal[0];
				tess3d.v[j].normal[1] = verts->normal[1];
				tess3d.v[j].normal[2] = verts->normal[2];
			}
			else {
				tess3d.v[j].normal[0] = oldVerts->normal[0] * backlerp + verts->normal[0] * frontlerp;
				tess3d.v[j].normal[1] = oldVerts->normal[1] * backlerp + verts->normal[1] * frontlerp;
				tess3d.v[j].normal[2] = oldVerts->normal[2] * backlerp + verts->normal[2] * frontlerp;
			}
		}

		qglInvalidateBufferData(GL_ARRAY_BUFFER);
		qglInvalidateBufferData(GL_ELEMENT_ARRAY_BUFFER);
		qglBufferSubData(GL_ARRAY_BUFFER, 0, mesh->num_verts * sizeof(vertex3d_t), &tess3d);
		qglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mesh->num_tris * 3 * sizeof(uint16_t), mesh->indexes);

		GL_DrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_SHORT, NULL);
	}

	GL_BlendFunc(GL_ONE, GL_ONE);

	if (currententity->flags & RF_DEPTHHACK)
		GL_DepthRange(gldepthmin, gldepthmax);

}