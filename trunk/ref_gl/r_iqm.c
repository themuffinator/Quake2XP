/*
Copyright (C) 2010 COR Entertainment, LLC.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "r_local.h"
#include "r_iqm.h"

static vec3_t NormalsArray[MAX_VERTICES];
static vec4_t TangentsArray[MAX_VERTICES];

float modelpitch;

void IQM_LoadVertexArrays(model_t *iqmmodel, float *vposition, float *vnormal, float *vtangent)
{
	int i;

	if(iqmmodel->numVertexes > 16384)
		return;

	iqmmodel->vertexes = (mvertex_t*)Hunk_Alloc(iqmmodel->numVertexes * sizeof(mvertex_t));
	iqmmodel->normal = (mnormal_t*)Hunk_Alloc(iqmmodel->numVertexes * sizeof(mnormal_t));
	iqmmodel->tangent = (mtangent_t*)Hunk_Alloc(iqmmodel->numVertexes * sizeof(mtangent_t));

	//set this now for later use
	iqmmodel->animatevertexes = (mvertex_t*)Hunk_Alloc(iqmmodel->numVertexes * sizeof(mvertex_t));
	iqmmodel->animatenormal = (mnormal_t*)Hunk_Alloc(iqmmodel->numVertexes * sizeof(mnormal_t));
	iqmmodel->animatetangent = (mtangent_t*)Hunk_Alloc(iqmmodel->numVertexes * sizeof(mtangent_t));

	for(i=0; i<iqmmodel->numVertexes; i++){
		VectorSet(iqmmodel->vertexes[i].position,
					LittleFloat(vposition[0]),
					LittleFloat(vposition[1]),
					LittleFloat(vposition[2]));

		VectorSet(iqmmodel->normal[i].dir,
					LittleFloat(vnormal[0]),
					LittleFloat(vnormal[1]),
					LittleFloat(vnormal[2]));

		Vector4Set(iqmmodel->tangent[i].dir,
					LittleFloat(vtangent[0]),
					LittleFloat(vtangent[1]),
					LittleFloat(vtangent[2]),
					LittleFloat(vtangent[3]));

		vposition	+=3;
		vnormal		+=3;
		vtangent	+=4;
	}
}

qboolean IQM_ReadSkinFile(char skin_file[MAX_OSPATH], char *skinpath)
{
	FILE *fp;
	int length;
	char *buffer;
	char *s;

	if((fp = fopen(skin_file, "rb" )) == NULL)
	{
		return qfalse;
	}
	else
	{
		size_t sz;
		fseek(fp, 0, SEEK_END);
		length = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		buffer = malloc( length + 1 );
		sz = fread( buffer, length, 1, fp );
		buffer[length] = 0;
	}
	s = buffer;

	strcpy( skinpath, COM_Parse( &s ) );
	skinpath[length] = 0; //clear any possible garbage

	if ( fp != 0 )
	{
		fclose(fp);
		free( buffer );
	}
	else
	{
		FS_FreeFile( buffer );
	}
	return qtrue;
}

qboolean Mod_INTERQUAKEMODEL_Load(model_t *mod, void *buffer)
{
	iqmheader_t *header;
	int i, j, k;
	const int *inelements;
	float *vposition = NULL, *vtexcoord = NULL, *vnormal = NULL, *vtangent = NULL;
	unsigned char *vblendweights = NULL;
	unsigned char *pbase;
	iqmjoint_t *joint = NULL;
	iqmjoint2_t *joint2 = NULL;
	matrix3x4_t	*inversebaseframe;
	iqmpose_t *poses;
	iqmpose2_t *poses2;
	iqmbounds_t *bounds;
	iqmvertexarray_t *va;
	unsigned short *framedata;
	char *text;
	char skinname[MAX_QPATH], shortname[MAX_QPATH];
	char *pskinpath_buffer;
	int skinpath_buffer_length;
	char *parse_string;

	pbase = (unsigned char *)buffer;
	header = (iqmheader_t *)buffer;
	if (memcmp(header->id, "INTERQUAKEMODEL", 16))
	{
		Com_Printf ("Mod_INTERQUAKEMODEL_Load: %s is not an Inter-Quake Model", mod->name);
		return qfalse;
	}

	if (LittleLong(header->version) != 1 && LittleLong(header->version) != 2)
	{
		Com_Printf ("Mod_INTERQUAKEMODEL_Load: only version 1 or 2 models are currently supported (name = %s)", mod->name);
		return qfalse;
	}

	mod->type = mod_iqm;

	// byteswap header
	header->version = LittleLong(header->version);
	header->filesize = LittleLong(header->filesize);
	header->flags = LittleLong(header->flags);
	header->num_text = LittleLong(header->num_text);
	header->ofs_text = LittleLong(header->ofs_text);
	header->num_meshes = LittleLong(header->num_meshes);
	header->ofs_meshes = LittleLong(header->ofs_meshes);
	header->num_vertexarrays = LittleLong(header->num_vertexarrays);
	header->num_vertexes = LittleLong(header->num_vertexes);
	header->ofs_vertexarrays = LittleLong(header->ofs_vertexarrays);
	header->num_triangles = LittleLong(header->num_triangles);
	header->ofs_triangles = LittleLong(header->ofs_triangles);
	header->ofs_neighbors = LittleLong(header->ofs_neighbors);
	header->num_joints = LittleLong(header->num_joints);
	header->ofs_joints = LittleLong(header->ofs_joints);
	header->num_poses = LittleLong(header->num_poses);
	header->ofs_poses = LittleLong(header->ofs_poses);
	header->num_anims = LittleLong(header->num_anims);
	header->ofs_anims = LittleLong(header->ofs_anims);
	header->num_frames = LittleLong(header->num_frames);
	header->num_framechannels = LittleLong(header->num_framechannels);
	header->ofs_frames = LittleLong(header->ofs_frames);
	header->ofs_bounds = LittleLong(header->ofs_bounds);
	header->num_comment = LittleLong(header->num_comment);
	header->ofs_comment = LittleLong(header->ofs_comment);
	header->num_extensions = LittleLong(header->num_extensions);
	header->ofs_extensions = LittleLong(header->ofs_extensions);

	if (header->num_triangles < 1 || header->num_vertexes < 3 || header->num_vertexarrays < 1 || header->num_meshes < 1)
	{
		Com_Printf("%s has no geometry\n", mod->name);
		return qfalse;
	}
	if (header->num_frames < 1 || header->num_anims < 1)
	{
		Com_Printf("%s has no animations\n", mod->name);
		return qfalse;
	}

	mod->extraData = Hunk_Begin (0x300000, mod->name);

	va = (iqmvertexarray_t *)(pbase + header->ofs_vertexarrays);
	for (i = 0;i < (int)header->num_vertexarrays;i++)
	{
		va[i].type = LittleLong(va[i].type);
		va[i].flags = LittleLong(va[i].flags);
		va[i].format = LittleLong(va[i].format);
		va[i].size = LittleLong(va[i].size);
		va[i].offset = LittleLong(va[i].offset);
		switch (va[i].type)
		{
		case IQM_POSITION:
			if (va[i].format == IQM_FLOAT && va[i].size == 3)
				vposition = (float *)(pbase + va[i].offset);
			break;
		case IQM_TEXCOORD:
			if (va[i].format == IQM_FLOAT && va[i].size == 2)
				vtexcoord = (float *)(pbase + va[i].offset);
			break;
		case IQM_NORMAL:
			if (va[i].format == IQM_FLOAT && va[i].size == 3)
				vnormal = (float *)(pbase + va[i].offset);
			break;
		case IQM_TANGENT:
			if (va[i].format == IQM_FLOAT && va[i].size == 4)
				vtangent = (float *)(pbase + va[i].offset);
			break;
		case IQM_BLENDINDEXES:
			if (va[i].format == IQM_UBYTE && va[i].size == 4)
			{
				mod->blendindexes = (unsigned char *)Hunk_Alloc(header->num_vertexes * 4 * sizeof(unsigned char));
				memcpy(mod->blendindexes, (unsigned char *)(pbase + va[i].offset), header->num_vertexes * 4 * sizeof(unsigned char));
			}
			break;
		case IQM_BLENDWEIGHTS:
			if (va[i].format == IQM_UBYTE && va[i].size == 4)
			{
				vblendweights = (unsigned char *)Hunk_Alloc(header->num_vertexes * 4 * sizeof(unsigned char));
				memcpy(vblendweights, (unsigned char *)(pbase + va[i].offset), header->num_vertexes * 4 * sizeof(unsigned char));
			}
			break;
		}
	}
	if (!vposition || !vtexcoord || !mod->blendindexes || !vblendweights)
	{
		Com_Printf("%s is missing vertex array data\n", mod->name);
		return qfalse;
	}

	text = header->num_text && header->ofs_text ? (char *)(pbase + header->ofs_text) : "";

	mod->jointname = (char *)Hunk_Alloc(header->num_text * sizeof(char *));
	memcpy(mod->jointname, text, header->num_text * sizeof(char *));

	mod->version = header->version;
	mod->numFrames = header->num_anims;
	mod->num_joints = header->num_joints;
	mod->num_poses = header->num_frames;
	mod->numVertexes = header->num_vertexes;
	mod->num_triangles = header->num_triangles;

	// load the joints
	if( header->version == 1 )
	{
		joint = (iqmjoint_t *) (pbase + header->ofs_joints);
		mod->joints = (iqmjoint_t*)Hunk_Alloc (header->num_joints * sizeof(iqmjoint_t));
		for (i = 0;i < mod->num_joints;i++)
		{
			mod->joints[i].name = LittleLong(joint[i].name);
			mod->joints[i].parent = LittleLong(joint[i].parent);
			for (j = 0;j < 3;j++)
			{
				mod->joints[i].origin[j] = LittleFloat(joint[i].origin[j]);
				mod->joints[i].rotation[j] = LittleFloat(joint[i].rotation[j]);
				mod->joints[i].scale[j] = LittleFloat(joint[i].scale[j]);
			}
		}
	}
	else
	{
		joint2 = (iqmjoint2_t *) (pbase + header->ofs_joints);
		mod->joints2 = (iqmjoint2_t*)Hunk_Alloc (header->num_joints * sizeof(iqmjoint2_t));
		for (i = 0;i < mod->num_joints;i++)
		{
			mod->joints2[i].name = LittleLong(joint2[i].name);
			mod->joints2[i].parent = LittleLong(joint2[i].parent);
			for (j = 0;j < 3;j++)
			{
				mod->joints2[i].origin[j] = LittleFloat(joint2[i].origin[j]);
				mod->joints2[i].rotation[j] = LittleFloat(joint2[i].rotation[j]);
				mod->joints2[i].scale[j] = LittleFloat(joint2[i].scale[j]);
			}
			mod->joints2[i].rotation[3] = LittleFloat(joint2[i].rotation[3]);
		}
	}
	//these don't need to be a part of mod - remember to free them
	mod->baseframe = (matrix3x4_t*)Hunk_Alloc (header->num_joints * sizeof(matrix3x4_t));
	inversebaseframe = (matrix3x4_t*)malloc (header->num_joints * sizeof(matrix3x4_t));

	if( header->version == 1 )
	{
		for(i = 0; i < (int)header->num_joints; i++)
		{
			vec3_t rot;
			vec4_t q_rot;
			iqmjoint_t j = mod->joints[i];

			//first need to make a vec4 quat from our rotation vec
			VectorSet(rot, j.rotation[0], j.rotation[1], j.rotation[2]);
			Vector4Set(q_rot, j.rotation[0], j.rotation[1], j.rotation[2], -sqrt(max(1.0 - pow(VectorLength(rot),2), 0.0)));

			Matrix3x4_FromQuatAndVectors(&mod->baseframe[i], q_rot, j.origin, j.scale);
			Matrix3x4_Invert(&inversebaseframe[i], mod->baseframe[i]);

			if(j.parent >= 0)
			{
				matrix3x4_t temp;
				Matrix3x4_Multiply(&temp, mod->baseframe[j.parent], mod->baseframe[i]);
				mod->baseframe[i] = temp;
				Matrix3x4_Multiply(&temp, inversebaseframe[i], inversebaseframe[j.parent]);
				inversebaseframe[i] = temp;
			}
		}
	}
	else
	{
		for(i = 0; i < (int)header->num_joints; i++)
		{
			iqmjoint2_t j = mod->joints2[i];

			Matrix3x4_FromQuatAndVectors(&mod->baseframe[i], j.rotation, j.origin, j.scale);
			Matrix3x4_Invert(&inversebaseframe[i], mod->baseframe[i]);

			assert(j.parent < (int)header->num_joints);

			if(j.parent >= 0)
			{
				matrix3x4_t temp;
				Matrix3x4_Multiply(&temp, mod->baseframe[j.parent], mod->baseframe[i]);
				mod->baseframe[i] = temp;
				Matrix3x4_Multiply(&temp, inversebaseframe[i], inversebaseframe[j.parent]);
				inversebaseframe[i] = temp;
			}
		}
	}

	if( header->version == 1 )
	{
		poses = (iqmpose_t *) (pbase + header->ofs_poses);
		mod->frames = (matrix3x4_t*)Hunk_Alloc (header->num_frames * header->num_poses * sizeof(matrix3x4_t));
		framedata = (unsigned short *) (pbase + header->ofs_frames);

		for(i = 0; i < header->num_frames; i++)
		{
			for(j = 0; j < header->num_poses; j++)
			{
				iqmpose_t p = poses[j];
				vec3_t translate, rotate, scale;
				vec4_t q_rot;
				matrix3x4_t m, temp;

				p.parent = LittleLong(p.parent);
                                p.channelmask = LittleLong(p.channelmask);
                                for(k = 0; k < 9; k++)
                                {
                                        p.channeloffset[k] = LittleFloat(p.channeloffset[k]);
                                        p.channelscale[k] = LittleFloat(p.channelscale[k]);
                                }

				translate[0] = p.channeloffset[0]; if(p.channelmask&0x01) translate[0] += (unsigned short)LittleShort(*framedata++) * p.channelscale[0];
				translate[1] = p.channeloffset[1]; if(p.channelmask&0x02) translate[1] += (unsigned short)LittleShort(*framedata++) * p.channelscale[1];
				translate[2] = p.channeloffset[2]; if(p.channelmask&0x04) translate[2] += (unsigned short)LittleShort(*framedata++) * p.channelscale[2];
				rotate[0] = p.channeloffset[3]; if(p.channelmask&0x08) rotate[0] += (unsigned short)LittleShort(*framedata++) * p.channelscale[3];
				rotate[1] = p.channeloffset[4]; if(p.channelmask&0x10) rotate[1] += (unsigned short)LittleShort(*framedata++) * p.channelscale[4];
				rotate[2] = p.channeloffset[5]; if(p.channelmask&0x20) rotate[2] += (unsigned short)LittleShort(*framedata++) * p.channelscale[5];
				scale[0] = p.channeloffset[6]; if(p.channelmask&0x40) scale[0] += (unsigned short)LittleShort(*framedata++) * p.channelscale[6];
				scale[1] = p.channeloffset[7]; if(p.channelmask&0x80) scale[1] += (unsigned short)LittleShort(*framedata++) * p.channelscale[7];
				scale[2] = p.channeloffset[8]; if(p.channelmask&0x100) scale[2] += (unsigned short)LittleShort(*framedata++) * p.channelscale[8];
				// Concatenate each pose with the inverse base pose to avoid doing this at animation time.
				// If the joint has a parent, then it needs to be pre-concatenated with its parent's base pose.
				// Thus it all negates at animation time like so:
				//   (parentPose * parentInverseBasePose) * (parentBasePose * childPose * childInverseBasePose) =>
				//   parentPose * (parentInverseBasePose * parentBasePose) * childPose * childInverseBasePose =>
				//   parentPose * childPose * childInverseBasePose

				Vector4Set(q_rot, rotate[0], rotate[1], rotate[2], -sqrt(max(1.0 - pow(VectorLength(rotate),2), 0.0)));

				Matrix3x4_FromQuatAndVectors(&m, q_rot, translate, scale);

				if(p.parent >= 0)
				{
					Matrix3x4_Multiply(&temp, mod->baseframe[p.parent], m);
					Matrix3x4_Multiply(&mod->frames[i*header->num_poses+j], temp, inversebaseframe[j]);
				}
				else
					Matrix3x4_Multiply(&mod->frames[i*header->num_poses+j], m, inversebaseframe[j]);
			}
		}
	}
	else
	{
		poses2 = (iqmpose2_t *) (pbase + header->ofs_poses);
		mod->frames = (matrix3x4_t*)Hunk_Alloc (header->num_frames * header->num_poses * sizeof(matrix3x4_t));
		framedata = (unsigned short *) (pbase + header->ofs_frames);

		for(i = 0; i < header->num_frames; i++)
		{
			for(j = 0; j < header->num_poses; j++)
			{
				iqmpose2_t p = poses2[j];
				vec3_t translate, scale;
				vec4_t rotate;
				matrix3x4_t m, temp;

				p.parent = LittleLong(p.parent);
				p.channelmask = LittleLong(p.channelmask);
				for(k = 0; k < 10; k++)
				{
					p.channeloffset[k] = LittleFloat(p.channeloffset[k]);
					p.channelscale[k] = LittleFloat(p.channelscale[k]);
				}

				translate[0] = p.channeloffset[0]; if(p.channelmask&0x01) translate[0] += (unsigned short)LittleShort(*framedata++) * p.channelscale[0];
				translate[1] = p.channeloffset[1]; if(p.channelmask&0x02) translate[1] += (unsigned short)LittleShort(*framedata++) * p.channelscale[1];
				translate[2] = p.channeloffset[2]; if(p.channelmask&0x04) translate[2] += (unsigned short)LittleShort(*framedata++) * p.channelscale[2];
				rotate[0] = p.channeloffset[3]; if(p.channelmask&0x08) rotate[0] += (unsigned short)LittleShort(*framedata++) * p.channelscale[3];
				rotate[1] = p.channeloffset[4]; if(p.channelmask&0x10) rotate[1] += (unsigned short)LittleShort(*framedata++) * p.channelscale[4];
				rotate[2] = p.channeloffset[5]; if(p.channelmask&0x20) rotate[2] += (unsigned short)LittleShort(*framedata++) * p.channelscale[5];
				rotate[3] = p.channeloffset[6]; if(p.channelmask&0x40) rotate[3] += (unsigned short)LittleShort(*framedata++) * p.channelscale[6];
				scale[0] = p.channeloffset[7]; if(p.channelmask&0x80) scale[0] += (unsigned short)LittleShort(*framedata++) * p.channelscale[7];
				scale[1] = p.channeloffset[8]; if(p.channelmask&0x100) scale[1] += (unsigned short)LittleShort(*framedata++) * p.channelscale[8];
				scale[2] = p.channeloffset[9]; if(p.channelmask&0x200) scale[2] += (unsigned short)LittleShort(*framedata++) * p.channelscale[9];
				// Concatenate each pose with the inverse base pose to avoid doing this at animation time.
				// If the joint has a parent, then it needs to be pre-concatenated with its parent's base pose.
				// Thus it all negates at animation time like so:
				//   (parentPose * parentInverseBasePose) * (parentBasePose * childPose * childInverseBasePose) =>
				//   parentPose * (parentInverseBasePose * parentBasePose) * childPose * childInverseBasePose =>
				//   parentPose * childPose * childInverseBasePose

				Matrix3x4_FromQuatAndVectors(&m, rotate, translate, scale);

				if(p.parent >= 0)
				{
					Matrix3x4_Multiply(&temp, mod->baseframe[p.parent], m);
					Matrix3x4_Multiply(&mod->frames[i*header->num_poses+j], temp, inversebaseframe[j]);
				}
				else
					Matrix3x4_Multiply(&mod->frames[i*header->num_poses+j], m, inversebaseframe[j]);
			}
		}
	}

	mod->outframe = (matrix3x4_t *)Hunk_Alloc(mod->num_joints * sizeof(matrix3x4_t));

	// load bounding box data
	if (header->ofs_bounds)
	{
		float xyradius = 0, radius = 0;
		bounds = (iqmbounds_t *) (pbase + header->ofs_bounds);
		VectorClear(mod->mins);
		VectorClear(mod->maxs);
		for (i = 0; i < (int)header->num_frames;i++)
		{
			bounds[i].mins[0] = LittleFloat(bounds[i].mins[0]);
			bounds[i].mins[1] = LittleFloat(bounds[i].mins[1]);
			bounds[i].mins[2] = LittleFloat(bounds[i].mins[2]);
			bounds[i].maxs[0] = LittleFloat(bounds[i].maxs[0]);
			bounds[i].maxs[1] = LittleFloat(bounds[i].maxs[1]);
			bounds[i].maxs[2] = LittleFloat(bounds[i].maxs[2]);
			bounds[i].xyradius = LittleFloat(bounds[i].xyradius);
			bounds[i].radius = LittleFloat(bounds[i].radius);

			if (mod->mins[0] > bounds[i].mins[0]) mod->mins[0] = bounds[i].mins[0];
			if (mod->mins[1] > bounds[i].mins[1]) mod->mins[1] = bounds[i].mins[1];
			if (mod->mins[2] > bounds[i].mins[2]) mod->mins[2] = bounds[i].mins[2];
			if (mod->maxs[0] < bounds[i].maxs[0]) mod->maxs[0] = bounds[i].maxs[0];
			if (mod->maxs[1] < bounds[i].maxs[1]) mod->maxs[1] = bounds[i].maxs[1];
			if (mod->maxs[2] < bounds[i].maxs[2]) mod->maxs[2] = bounds[i].maxs[2];

			if (bounds[i].xyradius > xyradius)
				xyradius = bounds[i].xyradius;
			if (bounds[i].radius > radius)
				radius = bounds[i].radius;
		}

		mod->radius = radius;
	}

	//compute a full bounding box
	for ( i = 0; i < 8; i++ )
	{
		vec3_t   tmp;

		if ( i & 1 )
			tmp[0] = mod->mins[0];
		else
			tmp[0] = mod->maxs[0];

		if ( i & 2 )
			tmp[1] = mod->mins[1];
		else
			tmp[1] = mod->maxs[1];

		if ( i & 4 )
			tmp[2] = mod->mins[2];
		else
			tmp[2] = mod->maxs[2];

		VectorCopy( tmp, mod->bbox[i] );
	}

	// load triangle data
	inelements = (const int *) (pbase + header->ofs_triangles);

	mod->tris = (iqmtriangle_t*)Hunk_Alloc(header->num_triangles * sizeof(iqmtriangle_t));

	for (i = 0;i < (int)header->num_triangles;i++)
	{
		mod->tris[i].vertex[0] = LittleLong(inelements[0]);
		mod->tris[i].vertex[1] = LittleLong(inelements[1]);
		mod->tris[i].vertex[2] = LittleLong(inelements[2]);
		inelements += 3;
	}

	//load triangle neighbors
	if (header->ofs_neighbors)
	{
		inelements = (const int *) (pbase + header->ofs_neighbors);

		mod->neighbours = Hunk_Alloc(header->num_triangles * sizeof(neighbors_t));

		for (i = 0;i < (int)header->num_triangles;i++)
		{
			mod->neighbours[i].n[0] = LittleLong(inelements[0]);
			mod->neighbours[i].n[1] = LittleLong(inelements[1]);
			mod->neighbours[i].n[2] = LittleLong(inelements[2]);
			inelements += 3;
		}
	}

	// load vertex data
	IQM_LoadVertexArrays(mod, vposition, vnormal, vtangent);

	// load texture coodinates
    mod->stCoords = (fstvert_t*)Hunk_Alloc (header->num_vertexes * sizeof(fstvert_t));
	mod->blendweights = (float *)Hunk_Alloc(header->num_vertexes * 4 * sizeof(float));

	for (i = 0;i < (int)header->num_vertexes;i++)
	{
		mod->stCoords[i].s = LittleFloat(vtexcoord[0]);
		mod->stCoords[i].t = LittleFloat(vtexcoord[1]);

		vtexcoord+=2;

		for(j = 0; j < 4; j++)
			mod->blendweights[i*4+j] = vblendweights[j]/255.0f;

		vblendweights+=4;
	}

	/*
	 * get skin pathname from <model>.skin file and initialize skin
	 */
	COM_StripExtension( mod->name, shortname );
	strcat( shortname, ".skin" );
	skinpath_buffer_length = FS_LoadFile( shortname, (void**)&pskinpath_buffer );
		// note: FS_LoadFile handles upper/lowercase, nul-termination,
		//  and path search sequence

	if ( skinpath_buffer_length > 0 )
	{ // <model>.skin file found and read,
		// data is in Z_Malloc'd buffer pointed to by pskin_buffer

		// get relative image pathname for model's skin from .skin file data
		parse_string = pskinpath_buffer;
		strcpy( skinname, COM_Parse( &parse_string ) );
		Z_Free( pskinpath_buffer ); // free Z_Malloc'd read buffer

		// get image file for skin
		mod->skins[0] = GL_FindImage( skinname, it_skin );
	}

	//free temp non hunk mem
	if(inversebaseframe)
		free(inversebaseframe);

	return qtrue;
}

void IQM_AnimateFrame(float curframe, int nextframe)
{
	int i, j;

    int frame1 = (int)floor(curframe),
        frame2 = nextframe;
    float frameoffset = curframe - frame1;
	frame1 %= currentmodel->num_poses;
	frame2 %= currentmodel->num_poses;

	{
		matrix3x4_t *mat1 = &currentmodel->frames[frame1 * currentmodel->num_joints],
			*mat2 = &currentmodel->frames[frame2 * currentmodel->num_joints];

		// Interpolate matrixes between the two closest frames and concatenate with parent matrix if necessary.
		// Concatenate the result with the inverse of the base pose.
		// You would normally do animation blending and inter-frame blending here in a 3D engine.

		for(i = 0; i < currentmodel->num_joints; i++)
		{
			matrix3x4_t mat, rmat, temp;
			vec3_t rot;
			Matrix3x4_Scale(&mat, mat1[i], 1-frameoffset);
			Matrix3x4_Scale(&temp, mat2[i], frameoffset);

			Matrix3x4_Add(&mat, mat, temp);

			if(currentmodel->version == 1)
			{
				if(currentmodel->joints[i].parent >= 0)
					Matrix3x4_Multiply(&currentmodel->outframe[i], currentmodel->outframe[currentmodel->joints[i].parent], mat);
				else
					Matrix3x4_Copy(&currentmodel->outframe[i], mat);
			}
			else
			{
				if(currentmodel->joints2[i].parent >= 0)
					Matrix3x4_Multiply(&currentmodel->outframe[i], currentmodel->outframe[currentmodel->joints2[i].parent], mat);
				else
					Matrix3x4_Copy(&currentmodel->outframe[i], mat);
			}

			//bend the model at the waist for player pitch
			if(currentmodel->version == 1)
			{
				if(!strcmp(&currentmodel->jointname[currentmodel->joints[i].name], "Spine")||
					!strcmp(&currentmodel->jointname[currentmodel->joints[i].name], "Spine.001"))
				{
					vec3_t basePosition, oldPosition, newPosition;
					VectorSet(rot, 0, 1, 0); //remember .iqm's are 90 degrees rotated from reality, so this is the pitch axis
					Matrix3x4GenRotate(&rmat, modelpitch, rot);

					// concatenate the rotation with the bone
					Matrix3x4_Multiply(&temp, rmat, currentmodel->outframe[i]);

					// get the position of the bone in the base frame
					VectorSet(basePosition, currentmodel->baseframe[i].a[3], currentmodel->baseframe[i].b[3], currentmodel->baseframe[i].c[3]);

					// add in the correct old bone position and subtract off the wrong new bone position to get the correct rotation pivot
					VectorSet(oldPosition,  DotProduct(basePosition, currentmodel->outframe[i].a) + currentmodel->outframe[i].a[3],
						 DotProduct(basePosition, currentmodel->outframe[i].b) + currentmodel->outframe[i].b[3],
						 DotProduct(basePosition, currentmodel->outframe[i].c) + currentmodel->outframe[i].c[3]);

					VectorSet(newPosition, DotProduct(basePosition, temp.a) + temp.a[3],
	   					 DotProduct(basePosition, temp.b) + temp.b[3],
						 DotProduct(basePosition, temp.c) + temp.c[3]);

					temp.a[3] += oldPosition[0] - newPosition[0];
					temp.b[3] += oldPosition[1] - newPosition[1];
					temp.c[3] += oldPosition[2] - newPosition[2];

					// replace the old matrix with the rotated one
					Matrix3x4_Copy(&currentmodel->outframe[i], temp);
				}
				//now rotate the legs back
				if(!strcmp(&currentmodel->jointname[currentmodel->joints[i].name], "hip.l")||
					!strcmp(&currentmodel->jointname[currentmodel->joints[i].name], "hip.r"))
				{
					vec3_t basePosition, oldPosition, newPosition;
					VectorSet(rot, 0, 1, 0);
					Matrix3x4GenRotate(&rmat, -modelpitch, rot);

					// concatenate the rotation with the bone
					Matrix3x4_Multiply(&temp, rmat, currentmodel->outframe[i]);

					// get the position of the bone in the base frame
					VectorSet(basePosition, currentmodel->baseframe[i].a[3], currentmodel->baseframe[i].b[3], currentmodel->baseframe[i].c[3]);

					// add in the correct old bone position and subtract off the wrong new bone position to get the correct rotation pivot
					VectorSet(oldPosition,  DotProduct(basePosition, currentmodel->outframe[i].a) + currentmodel->outframe[i].a[3],
						 DotProduct(basePosition, currentmodel->outframe[i].b) + currentmodel->outframe[i].b[3],
						 DotProduct(basePosition, currentmodel->outframe[i].c) + currentmodel->outframe[i].c[3]);

					VectorSet(newPosition, DotProduct(basePosition, temp.a) + temp.a[3],
	   					 DotProduct(basePosition, temp.b) + temp.b[3],
						 DotProduct(basePosition, temp.c) + temp.c[3]);

					temp.a[3] += oldPosition[0] - newPosition[0];
					temp.b[3] += oldPosition[1] - newPosition[1];
					temp.c[3] += oldPosition[2] - newPosition[2];

					// replace the old matrix with the rotated one
					Matrix3x4_Copy(&currentmodel->outframe[i], temp);
				}
			}
			else
			{
				if(!strcmp(&currentmodel->jointname[currentmodel->joints2[i].name], "Spine")||
				!strcmp(&currentmodel->jointname[currentmodel->joints2[i].name], "Spine.001"))
				{
					vec3_t basePosition, oldPosition, newPosition;
					VectorSet(rot, 0, 1, 0); //remember .iqm's are 90 degrees rotated from reality, so this is the pitch axis
					Matrix3x4GenRotate(&rmat, modelpitch, rot);

					// concatenate the rotation with the bone
					Matrix3x4_Multiply(&temp, rmat, currentmodel->outframe[i]);

					// get the position of the bone in the base frame
					VectorSet(basePosition, currentmodel->baseframe[i].a[3], currentmodel->baseframe[i].b[3], currentmodel->baseframe[i].c[3]);

					// add in the correct old bone position and subtract off the wrong new bone position to get the correct rotation pivot
					VectorSet(oldPosition,  DotProduct(basePosition, currentmodel->outframe[i].a) + currentmodel->outframe[i].a[3],
						 DotProduct(basePosition, currentmodel->outframe[i].b) + currentmodel->outframe[i].b[3],
						 DotProduct(basePosition, currentmodel->outframe[i].c) + currentmodel->outframe[i].c[3]);

					VectorSet(newPosition, DotProduct(basePosition, temp.a) + temp.a[3],
	   					 DotProduct(basePosition, temp.b) + temp.b[3],
						 DotProduct(basePosition, temp.c) + temp.c[3]);

					temp.a[3] += oldPosition[0] - newPosition[0];
					temp.b[3] += oldPosition[1] - newPosition[1];
					temp.c[3] += oldPosition[2] - newPosition[2];

					// replace the old matrix with the rotated one
					Matrix3x4_Copy(&currentmodel->outframe[i], temp);
				}
				//now rotate the legs back
				if(!strcmp(&currentmodel->jointname[currentmodel->joints2[i].name], "hip.l")||
					!strcmp(&currentmodel->jointname[currentmodel->joints2[i].name], "hip.r"))
				{
					vec3_t basePosition, oldPosition, newPosition;
					VectorSet(rot, 0, 1, 0);
					Matrix3x4GenRotate(&rmat, -modelpitch, rot);

					// concatenate the rotation with the bone
					Matrix3x4_Multiply(&temp, rmat, currentmodel->outframe[i]);

					// get the position of the bone in the base frame
					VectorSet(basePosition, currentmodel->baseframe[i].a[3], currentmodel->baseframe[i].b[3], currentmodel->baseframe[i].c[3]);

					// add in the correct old bone position and subtract off the wrong new bone position to get the correct rotation pivot
					VectorSet(oldPosition,  DotProduct(basePosition, currentmodel->outframe[i].a) + currentmodel->outframe[i].a[3],
						 DotProduct(basePosition, currentmodel->outframe[i].b) + currentmodel->outframe[i].b[3],
						 DotProduct(basePosition, currentmodel->outframe[i].c) + currentmodel->outframe[i].c[3]);

					VectorSet(newPosition, DotProduct(basePosition, temp.a) + temp.a[3],
	   					 DotProduct(basePosition, temp.b) + temp.b[3],
						 DotProduct(basePosition, temp.c) + temp.c[3]);

					temp.a[3] += oldPosition[0] - newPosition[0];
					temp.b[3] += oldPosition[1] - newPosition[1];
					temp.c[3] += oldPosition[2] - newPosition[2];

					// replace the old matrix with the rotated one
					Matrix3x4_Copy(&currentmodel->outframe[i], temp);
				}
			}
		}
	}
	// to do - this entire section could be handled in a glsl shader, saving huge amounts of cpu overhead
	// The actual vertex generation based on the matrixes follows...
	{
		const mvertex_t *srcpos = (const mvertex_t *)currentmodel->vertexes;
		const mnormal_t *srcnorm = (const mnormal_t *)currentmodel->normal;
		const mtangent_t *srctan = (const mtangent_t *)currentmodel->tangent;

		mvertex_t *dstpos = (mvertex_t *)currentmodel->animatevertexes;
		mnormal_t *dstnorm = (mnormal_t *)currentmodel->animatenormal;
		mtangent_t *dsttan = (mtangent_t *)currentmodel->animatetangent;

		const unsigned char *index = currentmodel->blendindexes;

		float *weight = currentmodel->blendweights;

		for(i = 0; i < currentmodel->numVertexes; i++)
		{
			matrix3x4_t mat;

			// Blend matrixes for this vertex according to its blend weights.
			// the first index/weight is always present, and the weights are
			// guaranteed to add up to 255. So if only the first weight is
			// presented, you could optimize this case by skipping any weight
			// multiplies and intermediate storage of a blended matrix.
			// There are only at most 4 weights per vertex, and they are in
			// sorted order from highest weight to lowest weight. Weights with
			// 0 values, which are always at the end, are unused.

			Matrix3x4_Scale(&mat, currentmodel->outframe[index[0]], weight[0]);

			for(j = 1; j < 4 && weight[j]; j++)
			{
				Matrix3x4_ScaleAdd (&mat, &currentmodel->outframe[index[j]], weight[j], &mat);
			}

			// Transform attributes by the blended matrix.
			// Position uses the full 3x4 transformation matrix.
			// Normals and tangents only use the 3x3 rotation part
			// of the transformation matrix.

			Matrix3x4_Transform(dstpos, mat, *srcpos);

			// Note that if the matrix includes non-uniform scaling, normal vectors
			// must be transformed by the inverse-transpose of the matrix to have the
			// correct relative scale. Note that invert(mat) = adjoint(mat)/determinant(mat),
			// and since the absolute scale is not important for a vector that will later
			// be renormalized, the adjoint-transpose matrix will work fine, which can be
			// cheaply generated by 3 cross-products.
			//
			// If you don't need to use joint scaling in your models, you can simply use the
			// upper 3x3 part of the position matrix instead of the adjoint-transpose shown
			// here.

			Matrix3x4_TransformNormal(dstnorm, mat, *srcnorm);

			// Note that input tangent data has 4 coordinates,
			// so only transform the first 3 as the tangent vector.

			Matrix3x4_TransformTangent(dsttan, mat, *srctan);

			srcpos++;
			srcnorm++;
			srctan++;
			dstpos++;
			dstnorm++;
			dsttan++;

			index += 4;
			weight += 4;
		}
	}
}

static qboolean IQM_CullModel( void )
{
	int i;
	vec3_t	vectors[3];
	vec3_t  angles;
	vec3_t	dist;
	vec3_t bbox[8];

	VectorSubtract(r_origin, currententity->origin, dist);

	/*
	** rotate the bounding box
	*/
	VectorCopy( currententity->angles, angles );
	angles[YAW] = -angles[YAW];
	AngleVectors( angles, vectors[0], vectors[1], vectors[2] );

	for ( i = 0; i < 8; i++ )
	{
		vec3_t tmp;

		VectorCopy( currentmodel->bbox[i], tmp );

		bbox[i][0] = DotProduct( vectors[0], tmp );
		bbox[i][1] = -DotProduct( vectors[1], tmp );
		bbox[i][2] = DotProduct( vectors[2], tmp );

		VectorAdd( currententity->origin, bbox[i], bbox[i] );
	}

	{
		int p, f, aggregatemask = ~0;

		for ( p = 0; p < 8; p++ )
		{
			int mask = 0;

			for ( f = 0; f < 4; f++ )
			{
				float dp = DotProduct( frustum[f].normal, bbox[p] );

				if ( ( dp - frustum[f].dist ) < 0 )
				{
					mask |= ( 1 << f );
				}
			}
			aggregatemask &= mask;
		}

		if ( aggregatemask && (VectorLength(dist) > 150)) //so shadows don't blatantly disappear when out of frustom
		{
			return qtrue;
		}

		return qfalse;
	}
}

//Can these next two be replaced with some type of animation grouping from the model?
qboolean IQM_InAnimGroup(int frame, int oldframe)
{
	//check if we are in a player anim group that is commonly looping
	if(frame >= 0 && frame <= 39 && oldframe >=0 && oldframe <= 39)
		return qtrue; //standing, or 40 frame static mesh
	else if(frame >= 40 && frame <=45 && oldframe >= 40 && oldframe <=45)
		return qtrue; //running
	else if(frame >= 66 && frame <= 71 && oldframe >= 66 && oldframe <= 71)
		return qtrue; //jumping
	else if(frame >= 0 && frame <= 23 && oldframe >= 0 && oldframe <= 23)
		return qtrue; //static meshes are 24 frames
	else
		return qfalse;
}

int IQM_NextFrame(int frame)
{
	int outframe;

	//just for now
	if(currententity->flags & RF_WEAPONMODEL)
	{
		outframe = frame + 1;
		return outframe;
	}

	switch(frame)
	{
		//map models can be 24 or 40 frames
		case 23:
			if(currentmodel->num_poses > 24)
				outframe = frame + 1;
			else
				outframe = 0;
			break;
		//player standing
		case 39:
			outframe = 0;
			break;
		//player running
		case 45:
			outframe = 40;
			break;
		//player shooting
		case 53:
			outframe = 46;
			break;
		//player jumping
		case 71:
			outframe = 0;
			break;
		//player crouched
		case 153:
			outframe = 135;
			break;
		//player crouched walking
		case 159:
			outframe = 154;
			break;
		case 168:
			outframe = 160;
			break;
		//deaths
		case 219:
			outframe = 219;
			break;
		case 237:
			outframe = 237;
			break;
		case 257:
			outframe = 257;
			break;
		default:
			outframe = frame + 1;
			break;
	}
	return outframe;
}
/*
=================
R_DrawINTERQUAKEMODEL
=================
*/
static vec3_t shadelight;

void IQM_DrawMesh()
{
	int     i, j;
	int     index_xyz, index_st;
	int     idx = 0;
	vec3_t	vertexArray[3 * MAX_TRIANGLES];
/*	vec2_t	texArray[2 * MAX_TRIANGLES];
	image_t	*skin, *glowskin;
	float	alphaShift;

	alphaShift = sin(ref_realtime * 5.666);
	alphaShift = (alphaShift + 1) * 0.5f;
	alphaShift = clamp(alphaShift, 0.3, 1.0);

	// select skin
	if (currententity->skin) {
		skin = currententity->skin;
	}
	else
	{
		skin = currentmodel->skins[0];
	}
	if (!skin)
		skin = r_notexture;	// fallback...

	glowskin = currentmodel->glowtexture[currententity->skinnum];

	if (!glowskin)
		glowskin = r_notexture;
*/

	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertexArray);

//	qglEnableVertexAttribArray(ATT_TEX0);
//	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texArray);

	GL_BindProgram(nullProgram, 0);
	qglUniformMatrix4fv(null_mvp, 1, qfalse, (const float *)currententity->orMatrix);

	for (i = 0; i<currentmodel->num_triangles; i++)
	{
		for (j = 0; j < 3; j++)
		{
			index_xyz = index_st = currentmodel->tris[i].vertex[j];

			vertexArray[i * 3 + j][0] = currentmodel->animatevertexes[index_xyz].position[0];
			vertexArray[i * 3 + j][1] = currentmodel->animatevertexes[index_xyz].position[1];
			vertexArray[i * 3 + j][2] = currentmodel->animatevertexes[index_xyz].position[2];

		//	texArray[i * 3 + j][0] = currentmodel->stCoords[index_st].s;
		//	texArray[i * 3 + j][1] = currentmodel->stCoords[index_st].t;
			idx++;
		}
	}

	qglDrawArrays(GL_TRIANGLES, 0, idx);

	qglDisableVertexAttribArray(ATT_POSITION);
//	qglDisableVertexAttribArray(ATT_TEX0);
	GL_BindNullProgram();
}

double degreeToRadian(double degree)
{
	double radian = 0;
	radian = degree * (PI / 180);
	return radian;
}

void IQM_DrawModels(void)
{
	float		frame, time;

	if (currententity->flags & (RF_SHELL_HALF_DAM | RF_SHELL_GREEN | RF_SHELL_RED | RF_SHELL_BLUE | RF_SHELL_DOUBLE)) //no shells
		return;

	if (IQM_CullModel())
		return;

	//modelpitch = 0.52 * sinf(rs_realtime); //use this for testing only
	modelpitch = degreeToRadian(currententity->angles[PITCH]);


	currententity->angles[PITCH] = currententity->angles[ROLL] = 0;
	R_SetupEntityMatrix(currententity);

	//frame interpolation
	time = (Sys_Milliseconds() - currententity->iqmFrameTime) / 100;
	if (time > 1.0)
		time = 1.0;

	if ((currententity->frame == currententity->oldframe) && !IQM_InAnimGroup(currententity->frame, currententity->oldframe))
		time = 0;

	//Check for stopped death anims
	if (currententity->frame == 257 || currententity->frame == 237 || currententity->frame == 219)
		time = 0;

	frame = currententity->frame + time;

	IQM_AnimateFrame(frame, IQM_NextFrame(currententity->frame));

	IQM_DrawMesh();
}
