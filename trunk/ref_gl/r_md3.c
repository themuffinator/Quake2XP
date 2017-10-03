#include "r_local.h"

typedef uint	index32_t; //index_t is ushort

void SinCos(float radians, float *sine, float *cosine)
{
/*	_asm
	{
		fld	dword ptr[radians]
		fsincos

		mov edx, dword ptr[cosine]
		mov eax, dword ptr[sine]

		fstp dword ptr[edx]
		fstp dword ptr[eax]
	}*/
	*sine = sinf(radians);
	*cosine = cosf(radians);
}

void Tangent4TrisMD3(index32_t *index, md3Vertex_t *vertices, md3ST_t *texcos, vec3_t Tangent, vec3_t Binormal)
{
	float *v0, *v1, *v2;
	float *st0, *st1, *st2;
	vec3_t vec1, vec2;
	vec3_t planes[3];
	int i;

	v0 = vertices[index[0]].xyz;
	v1 = vertices[index[1]].xyz;
	v2 = vertices[index[2]].xyz;
	st0 = texcos[index[0]].st;
	st1 = texcos[index[1]].st;
	st2 = texcos[index[2]].st;

	for (i = 0; i<3; i++)
	{
		vec1[0] = v1[i] - v0[i];
		vec1[1] = st1[0] - st0[0];
		vec1[2] = st1[1] - st0[1];
		vec2[0] = v2[i] - v0[i];
		vec2[1] = st2[0] - st0[0];
		vec2[2] = st2[1] - st0[1];
		VectorNormalize(vec1);
		VectorNormalize(vec2);
		CrossProduct(vec1, vec2, planes[i]);
	}

	Tangent[0] = -planes[0][1] / planes[0][0];
	Tangent[1] = -planes[1][1] / planes[1][0];
	Tangent[2] = -planes[2][1] / planes[2][0];
	Binormal[0] = -planes[0][2] / planes[0][0];
	Binormal[1] = -planes[1][2] / planes[1][0];
	Binormal[2] = -planes[2][2] / planes[2][0];
	VectorNormalize(Tangent); //is this needed?
	VectorNormalize(Binormal);
}


static int Mod_FindTriangleWithEdge(md3Mesh_t *mesh, index32_t p1, index32_t p2, int ignore)
{
	int		i, match, count;
	index32_t *indexes;

	count = 0;
	match = -1;

	for (i = 0, indexes = mesh->indexes; i<mesh->num_tris; i++, indexes += 3)
	{
		if ((indexes[0] == p2 && indexes[1] == p1)
			|| (indexes[1] == p2 && indexes[2] == p1)
			|| (indexes[2] == p2 && indexes[0] == p1)) {
			if (i != ignore)
				match = i;
			count++;
		}
		else if ((indexes[0] == p1 && indexes[1] == p2)
			|| (indexes[1] == p1 && indexes[2] == p2)
			|| (indexes[2] == p1 && indexes[0] == p2))
			count++;
	}

	// detect edges shared by three triangles and make them seams
	if (count > 2)
		match = -1;

	return match;
}

/*
===============
Mod_BuildTriangleNeighbors
===============
*/
static void Mod_BuildTriangleNeighbors(md3Mesh_t *mesh)
{
	int		i, *n;
	index32_t	*index;

	for (i = 0, n = mesh->neighbours, index = mesh->indexes; i<mesh->num_tris; i++, n += 3, index += 3)
	{
		n[0] = Mod_FindTriangleWithEdge(mesh, index[0], index[1], i);
		n[1] = Mod_FindTriangleWithEdge(mesh, index[1], index[2], i);
		n[2] = Mod_FindTriangleWithEdge(mesh, index[2], index[0], i);
	}
}


/*
=================
Mod_LoadAliasMD3Model
=================
*/
void Mod_LoadMD3(model_t *mod, void *buffer)
{
	int					version, i, j, l, ll, k;
	dmd3_t				*inModel;
	dmd3frame_t			*inFrame;
	dmd3tag_t			*inTag;
	dmd3mesh_t			*inMesh;
	dmd3skin_t			*inSkin;
	dmd3coord_t			*inCoord;
	dmd3vertex_t		*inVerts;
	index32_t			*inIndex, *outIndex;
	
	md3Vertex_t			*outVerts;
	md3ST_t				*outCoord;
	md3Skin_t			*outSkin;
	md3Mesh_t			*outMesh;
	md3Tag_t			*outTag;
	md3Frame_t			*outFrame;
	md3Model_t			*outModel;
	char				name[MD3_MAX_PATH];
	float				lat, lng;
	vec3_t				tangents[MD3_MAX_VERTS], 
						binormals[MD3_MAX_VERTS];
	
	inModel = (dmd3_t *)buffer;
	version = LittleLong(inModel->version);

	if (version != MD3_ALIAS_VERSION)
	{
		VID_Error(ERR_DROP, "%s has wrong version number (%i should be %i)",
			mod->name, version, MD3_ALIAS_VERSION);
	}

	outModel = Hunk_Alloc(sizeof(md3Model_t));

	// byte swap the header fields and sanity check
	outModel->num_frames	= LittleLong(inModel->num_frames);
	outModel->num_tags		= LittleLong(inModel->num_tags);
	outModel->num_meshes	= LittleLong(inModel->num_meshes);

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
	inFrame		= (dmd3frame_t *)((byte *)inModel + LittleLong(inModel->ofs_frames));
	outFrame	= outModel->frames = Hunk_Alloc(sizeof(md3Frame_t) * outModel->num_frames);

	mod->radius = 0;
	ClearBounds(mod->mins, mod->maxs);

	for (i = 0; i < outModel->num_frames; i++, inFrame++, outFrame++)
	{
		for (j = 0; j < 3; j++)
		{
			outFrame->mins[j]		= LittleFloat(inFrame->mins[j]);
			outFrame->maxs[j]		= LittleFloat(inFrame->maxs[j]);
			outFrame->scale[j]		= MD3_XYZ_SCALE;
			outFrame->translate[j]	= LittleFloat(inFrame->translate[j]);
		}

		outFrame->radius = LittleFloat(inFrame->radius);

		mod->radius = max(mod->radius, outFrame->radius);
		AddPointToBounds(outFrame->mins, mod->mins, mod->maxs);
		AddPointToBounds(outFrame->maxs, mod->mins, mod->maxs);
	}

	//
	// load the tags
	//
	inTag	= (dmd3tag_t *)((byte *)inModel + LittleLong(inModel->ofs_tags));
	outTag	= outModel->tags = Hunk_Alloc(sizeof(md3Tag_t) * outModel->num_frames * outModel->num_tags);

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
	inMesh	= (dmd3mesh_t *)((byte *)inModel + LittleLong(inModel->ofs_meshes));
	outMesh = outModel->meshes = Hunk_Alloc(sizeof(md3Mesh_t)*outModel->num_meshes);

	for (i = 0; i < outModel->num_meshes; i++, outMesh++)
	{
		memcpy(outMesh->name, inMesh->name, MD3_MAX_PATH);

		if (strncmp((const char *)inMesh->id, "IDP3", 4))
		{
			VID_Error(ERR_DROP, "mesh %s in model %s has wrong id (%i should be %i)",
				outMesh->name, mod->name, LittleLong((int)inMesh->id), IDMD3HEADER);
		}

		outMesh->num_tris	= LittleLong(inMesh->num_tris);
		outMesh->num_skins	= LittleLong(inMesh->num_skins);
		outMesh->num_verts	= LittleLong(inMesh->num_verts);

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
			VID_Error(ERR_DROP, "mesh %i in model %s has too many vertices", i, mod->name);

		//
		// register all skins
		//
		inSkin	= (dmd3skin_t *)((byte *)inMesh + LittleLong(inMesh->ofs_skins));
		outSkin = outMesh->skins = Hunk_Alloc(sizeof(md3Skin_t) * outMesh->num_skins);

		for (j = 0; j < outMesh->num_skins; j++, inSkin++, outSkin++)
		{
			char tex[128];
			memcpy(name, inSkin->name, MD3_MAX_PATH);
			mod->skinsMD3[i][j] = GL_FindImage(name, it_skin);
			
			// GlowMaps loading
			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_light.tga");
			mod->skinsMD3_glow[i][j] = GL_FindImage(tex, it_skin);
			if (!mod->skinsMD3_glow[i][j])
				mod->skinsMD3_glow[i][j] = r_notexture;

			// Normal maps loading
			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_bump.tga");
			mod->skinsMD3_normal[i][j] = GL_FindImage(tex, it_skin);
			if (!mod->skinsMD3_normal[i][j])
				mod->skinsMD3_normal[i][j] = r_notexture;

			// Roughness maps loading
			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_rgh.tga");
			mod->skinsMD3_roughness[i][j] = GL_FindImage(tex, it_skin);
			if (!mod->skinsMD3_roughness[i][j])
				mod->skinsMD3_roughness[i][j] = r_notexture;
		}

		//
		// load the indexes
		//
		inIndex		= (index32_t *)((byte *)inMesh + LittleLong(inMesh->ofs_tris));
		outIndex	= outMesh->indexes = Hunk_Alloc(sizeof(index32_t) * outMesh->num_tris * 3);

		for (j = 0; j < outMesh->num_tris; j++, inIndex += 3, outIndex += 3)
		{
			outIndex[0] = (index32_t)LittleLong(inIndex[0]);
			outIndex[1] = (index32_t)LittleLong(inIndex[1]);
			outIndex[2] = (index32_t)LittleLong(inIndex[2]);
		}

		//
		// load the texture coordinates
		//
		inCoord		= (dmd3coord_t *)((byte *)inMesh + LittleLong(inMesh->ofs_tcs));
		outCoord	= outMesh->stcoords = Hunk_Alloc(sizeof(md3ST_t) * outMesh->num_verts);

		for (j = 0; j < outMesh->num_verts; j++, inCoord++, outCoord++)
		{
			outCoord->st[0] = LittleFloat(inCoord->st[0]);
			outCoord->st[1] = LittleFloat(inCoord->st[1]);
		}
	
		//
		// Calculate TBN
		//
		inVerts = (dmd3vertex_t *)((byte *)inMesh + LittleLong(inMesh->ofs_verts));
		outVerts = outMesh->vertexes = Hunk_Alloc(outModel->num_frames * outMesh->num_verts * sizeof(md3Vertex_t));

		for (l = 0; l < outModel->num_frames; l++)
		{
			// for all frames
				memset(tangents, 0, outMesh->num_verts * sizeof(vec3_t));
				memset(binormals, 0, outMesh->num_verts * sizeof(vec3_t));
				outVerts = outMesh->vertexes + l * outMesh->num_verts;
				for (j = 0; j < outMesh->num_verts; j++, inVerts++, outVerts++)
				{
					vec3_t	vertex;
					int		y;
					for (y = 0; y<3; y++)
					{
						outVerts->xyz[y] = (float)LittleShort(inVerts->point[y]);
						vertex[y] = outVerts->xyz[y] + outModel->frames[l].translate[y];
					}

					lat = (inVerts->norm >> 8) & 0xff;
					lng = (inVerts->norm & 0xff);

					lat *= M_PI / 128;
					lng *= M_PI / 128;
					 
					vec3_t	norma;
					float	slat, clat, slng, clng;
					SinCos(lat, &slat, &clat);
					SinCos(lng, &slng, &clng);

					norma[0] = clat * slng;
					norma[1] = slat * slng;
					norma[2] = clng;
					outVerts->normal = Normal2Index(norma);
				}
				//for all tris
				outVerts = outMesh->vertexes + l * outMesh->num_verts;
				for (j = 0; j<outMesh->num_tris; j++)
				{
					vec3_t tangent;
					vec3_t binormal;

					Tangent4TrisMD3(&outMesh->indexes[j * 3], outVerts, outMesh->stcoords, tangent, binormal);
					// for all vertices in the tri
					for (k = 0; k<3; k++)
					{
						ll = outMesh->indexes[j * 3 + k];
						VectorAdd(tangents[ll], tangent, tangents[ll]);
						VectorAdd(binormals[ll], binormal, binormals[ll]);
					}
				}
				// normalize averages
				for (j = 0; j<outMesh->num_verts; j++)
				{
					VectorNormalize(tangents[j]);
					VectorNormalize(binormals[j]);
					outVerts[j].tangent = Normal2Index(tangents[j]);
					outVerts[j].binormal = Normal2Index(binormals[j]);
				}
			
		}

		//
		// load the vertexes
		//

		inVerts = (dmd3vertex_t *)((byte *)inMesh + LittleLong(inMesh->ofs_verts));
		outVerts = outMesh->vertexes;
		for (l = 0; l < outModel->num_frames; l++)
		{
			for (j = 0; j < outMesh->num_verts; j++, inVerts++, outVerts++)
			{
				vec3_t	vertex;
				int		y;
				for (y = 0; y<3; y++)
				{
					outVerts->xyz[y] = (float)LittleShort(inVerts->point[y]);
					vertex[y] = outVerts->xyz[y] + outModel->frames[l].translate[y];
				}
				AddPointToBounds(vertex, mod->mins, mod->maxs);
			}
		}
		//
		// build triangle neighbours
		//
		inMesh = (dmd3mesh_t *)((byte *)inMesh + LittleLong(inMesh->meshsize));
		outMesh->neighbours = Hunk_Alloc(sizeof(int) * outMesh->num_tris * 3);
		Mod_BuildTriangleNeighbors(outMesh);
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
qboolean R_CullMD3Model(vec3_t bbox[8], entity_t *e)
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
		Com_Printf("R_Cullmd3Model %s: no such frame %d\n", currentmodel->name, e->frame);
		e->frame = 0;
	}
	if ((e->oldframe >= md3Hdr->num_frames) || (e->oldframe < 0))
	{
		Com_Printf("R_Cullmd3Model %s: no such oldframe %d\n", currentmodel->name, e->oldframe);
		e->oldframe = 0;
	}

	currFrame = md3Hdr->frames + e->frame;
	oldFrame = md3Hdr->frames + e->oldframe;

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
	for (i = 0; i<8; i++)
	{
		mask = 0;
		for (j = 0; j<4; j++)
		{
			float dp = DotProduct(frustum[j].normal, bbox[i]);
			if ((dp - frustum[j].dist) < 0)
				mask |= (1 << j);
		}

		aggregatemask &= mask;
	}

	if (aggregatemask)
		return qtrue;

	return qfalse;
}

vec3_t	md3vertexCache[MAX_VERTICES * 4];

void GL_LerpMD3Verts(md3Mesh_t *mesh, md3Model_t *md3Hdr) {
	
	int			j;
	float		backlerp, frontlerp;
	vec3_t		move, delta, vectors[3];
	md3Vertex_t	*v, *ov;
	md3Frame_t	*frame, *oldFrame;
	
	backlerp = currententity->backlerp;
	frontlerp = 1.0 - backlerp;

	frame = md3Hdr->frames + currententity->frame;
	oldFrame = md3Hdr->frames + currententity->oldframe;

	VectorSubtract(currententity->oldorigin, currententity->origin, delta);
	AngleVectors(currententity->angles, vectors[0], vectors[1], vectors[2]);
	move[0] = DotProduct(delta, vectors[0]);	// forward
	move[1] = -DotProduct(delta, vectors[1]);	// left
	move[2] = DotProduct(delta, vectors[2]);	// up

	VectorAdd(move, oldFrame->translate, move);

	for (j = 0; j<3; j++)
		move[j] = backlerp * move[j] + frontlerp * frame->translate[j];

	v = mesh->vertexes + currententity->frame * mesh->num_verts;
	ov = mesh->vertexes + currententity->oldframe * mesh->num_verts;

	for (j = 0; j < mesh->num_verts; j++, v++, ov++) {
			
			VectorSet(md3vertexCache[j],
			move[0] + ov->xyz[0] * backlerp + v->xyz[0] * frontlerp,
			move[1] + ov->xyz[1] * backlerp + v->xyz[1] * frontlerp,
			move[2] + ov->xyz[2] * backlerp + v->xyz[2] * frontlerp);
	}

}
void GL_DrawMD3AliasFrameLerpLight(md3Mesh_t *mesh, md3Model_t *md3Hdr, int nmesh)
{
	int		i;
	vec3_t	normalArray[MD3_MAX_VERTS], 
			tangentArray[MD3_MAX_VERTS], 
			binormalArray[MD3_MAX_VERTS];

	float		backlerp, frontlerp;
	md3Vertex_t	*verts, *oldVerts;

	backlerp = currententity->backlerp;
	frontlerp = 1.0 - backlerp;
	
	verts = mesh->vertexes + currententity->frame * mesh->num_verts;
	oldVerts = mesh->vertexes + currententity->oldframe * mesh->num_verts;

		for (i = 0; i<mesh->num_verts; i++)
		{
				normalArray[i][0] = q_byteDirs[verts[i].normal][0] * frontlerp + q_byteDirs[oldVerts[i].normal][0] * backlerp;
				normalArray[i][1] = q_byteDirs[verts[i].normal][1] * frontlerp + q_byteDirs[oldVerts[i].normal][1] * backlerp;
				normalArray[i][2] = q_byteDirs[verts[i].normal][2] * frontlerp + q_byteDirs[oldVerts[i].normal][2] * backlerp;
			
				tangentArray[i][0] = q_byteDirs[verts[i].tangent][0] * frontlerp + q_byteDirs[oldVerts[i].tangent][0] * backlerp;
				tangentArray[i][1] = q_byteDirs[verts[i].tangent][1] * frontlerp + q_byteDirs[oldVerts[i].tangent][1] * backlerp;
				tangentArray[i][2] = q_byteDirs[verts[i].tangent][2] * frontlerp + q_byteDirs[oldVerts[i].tangent][2] * backlerp;
			

				binormalArray[i][0] = q_byteDirs[verts[i].binormal][0] * frontlerp + q_byteDirs[oldVerts[i].binormal][0] * backlerp;
				binormalArray[i][1] = q_byteDirs[verts[i].binormal][1] * frontlerp + q_byteDirs[oldVerts[i].binormal][1] * backlerp;
				binormalArray[i][2] = q_byteDirs[verts[i].binormal][2] * frontlerp + q_byteDirs[oldVerts[i].binormal][2] * backlerp;
		
		}
	
	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TANGENT);
	qglEnableVertexAttribArray(ATT_BINORMAL);
	qglEnableVertexAttribArray(ATT_NORMAL);
	qglEnableVertexAttribArray(ATT_TEX0);

	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0,	md3vertexCache);
	qglVertexAttribPointer(ATT_TANGENT, 3, GL_FLOAT, qfalse, 0,		tangentArray);
	qglVertexAttribPointer(ATT_BINORMAL, 3, GL_FLOAT, qfalse, 0,	binormalArray);
	qglVertexAttribPointer(ATT_NORMAL, 3, GL_FLOAT, qfalse, 0,		normalArray);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0,		mesh->stcoords);

	qglDrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_INT, mesh->indexes);

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TANGENT);
	qglDisableVertexAttribArray(ATT_BINORMAL);
	qglDisableVertexAttribArray(ATT_NORMAL);
	qglDisableVertexAttribArray(ATT_TEX0);

}
