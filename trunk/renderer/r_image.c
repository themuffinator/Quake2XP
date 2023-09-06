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

#include "r_local.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_TGA
#include "stb/stb_image.h"

// original Q2 palette
static const byte r_originalPalette[] = {
  0,   0,   0,  15,  15,  15,  31,  31,  31,  47,  47,  47, 
 63,  63,  63,  75,  75,  75,  91,  91,  91, 107, 107, 107, 
123, 123, 123, 139, 139, 139, 155, 155, 155, 171, 171, 171, 
187, 187, 187, 203, 203, 203, 219, 219, 219, 235, 235, 235, 
 99,  75,  35,  91,  67,  31,  83,  63,  31,  79,  59,  27, 
 71,  55,  27,  63,  47,  23,  59,  43,  23,  51,  39,  19, 
 47,  35,  19,  43,  31,  19,  39,  27,  15,  35,  23,  15, 
 27,  19,  11,  23,  15,  11,  19,  15,   7,  15,  11,   7, 
 95,  95, 111,  91,  91, 103,  91,  83,  95,  87,  79,  91, 
 83,  75,  83,  79,  71,  75,  71,  63,  67,  63,  59,  59, 
 59,  55,  55,  51,  47,  47,  47,  43,  43,  39,  39,  39, 
 35,  35,  35,  27,  27,  27,  23,  23,  23,  19,  19,  19, 
143, 119,  83, 123,  99,  67, 115,  91,  59, 103,  79,  47, 
207, 151,  75, 167, 123,  59, 139, 103,  47, 111,  83,  39, 
235, 159,  39, 203, 139,  35, 175, 119,  31, 147,  99,  27, 
119,  79,  23,  91,  59,  15,  63,  39,  11,  35,  23,   7, 
167,  59,  43, 159,  47,  35, 151,  43,  27, 139,  39,  19, 
127,  31,  15, 115,  23,  11, 103,  23,   7,  87,  19,   0, 
 75,  15,   0,  67,  15,   0,  59,  15,   0,  51,  11,   0, 
 43,  11,   0,  35,  11,   0,  27,   7,   0,  19,   7,   0, 
123,  95,  75, 115,  87,  67, 107,  83,  63, 103,  79,  59, 
 95,  71,  55,  87,  67,  51,  83,  63,  47,  75,  55,  43, 
 67,  51,  39,  63,  47,  35,  55,  39,  27,  47,  35,  23, 
 39,  27,  19,  31,  23,  15,  23,  15,  11,  15,  11,   7, 
111,  59,  23,  95,  55,  23,  83,  47,  23,  67,  43,  23, 
 55,  35,  19,  39,  27,  15,  27,  19,  11,  15,  11,   7, 
179,  91,  79, 191, 123, 111, 203, 155, 147, 215, 187, 183, 
203, 215, 223, 179, 199, 211, 159, 183, 195, 135, 167, 183, 
115, 151, 167,  91, 135, 155,  71, 119, 139,  47, 103, 127, 
 23,  83, 111,  19,  75, 103,  15,  67,  91,  11,  63,  83, 
  7,  55,  75,   7,  47,  63,   7,  39,  51,   0,  31,  43, 
  0,  23,  31,   0,  15,  19,   0,   7,  11,   0,   0,   0, 
139,  87,  87, 131,  79,  79, 123,  71,  71, 115,  67,  67, 
107,  59,  59,  99,  51,  51,  91,  47,  47,  87,  43,  43, 
 75,  35,  35,  63,  31,  31,  51,  27,  27,  43,  19,  19, 
 31,  15,  15,  19,  11,  11,  11,   7,   7,   0,   0,   0, 
151, 159, 123, 143, 151, 115, 135, 139, 107, 127, 131,  99, 
119, 123,  95, 115, 115,  87, 107, 107,  79,  99,  99,  71, 
 91,  91,  67,  79,  79,  59,  67,  67,  51,  55,  55,  43, 
 47,  47,  35,  35,  35,  27,  23,  23,  19,  15,  15,  11, 
159,  75,  63, 147,  67,  55, 139,  59,  47, 127,  55,  39, 
119,  47,  35, 107,  43,  27,  99,  35,  23,  87,  31,  19, 
 79,  27,  15,  67,  23,  11,  55,  19,  11,  43,  15,   7, 
 31,  11,   7,  23,   7,   0,  11,   0,   0,   0,   0,   0, 
119, 123, 207, 111, 115, 195, 103, 107, 183,  99,  99, 167, 
 91,  91, 155,  83,  87, 143,  75,  79, 127,  71,  71, 115, 
 63,  63, 103,  55,  55,  87,  47,  47,  75,  39,  39,  63, 
 35,  31,  47,  27,  23,  35,  19,  15,  23,  11,   7,   7, 
155, 171, 123, 143, 159, 111, 135, 151,  99, 123, 139,  87, 
115, 131,  75, 103, 119,  67,  95, 111,  59,  87, 103,  51, 
 75,  91,  39,  63,  79,  27,  55,  67,  19,  47,  59,  11, 
 35,  47,   7,  27,  35,   0,  19,  23,   0,  11,  15,   0, 
  0, 255,   0,  35, 231,  15,  63, 211,  27,  83, 187,  39, 
 95, 167,  47,  95, 143,  51,  95, 123,  51, 255, 255, 255, 
255, 255, 211, 255, 255, 167, 255, 255, 127, 255, 255,  83, 
255, 255,  39, 255, 235,  31, 255, 215,  23, 255, 191,  15, 
255, 171,   7, 255, 147,   0, 239, 127,   0, 227, 107,   0, 
211,  87,   0, 199,  71,   0, 183,  59,   0, 171,  43,   0, 
155,  31,   0, 143,  23,   0, 127,  15,   0, 115,   7,   0, 
 95,   0,   0,  71,   0,   0,  47,   0,   0,  27,   0,   0, 
239,   0,   0,  55,  55, 255, 255,   0,   0,   0,   0, 255, 
 43,  43,  35,  27,  27,  23,  19,  19,  15, 235, 151, 127, 
195, 115,  83, 159,  87,  51, 123,  63,  27, 235, 211, 199, 
199, 171, 155, 167, 139, 119, 135, 107,  87, 159,  91,  83
};

image_t* GL_LoadPic(char* name, byte* pic, int width, int height, imagetype_t type, int bits, uint _hash);

static image_t *r_imageHashTable[IMAGE_HASH_SIZE];
static byte intensitytable[256];
static uchar gammatable[256];
uint d_8to24table[256];
float d_8to24tablef[256][3];
int upload_width, upload_height;
qboolean uploaded_paletted;

qboolean STB_LoadTexture(const char* name, byte** pic, int* width, int* height){
	int		w, h, bbp;
	byte*	buffer = NULL;
	byte*	data = NULL;

	int len = FS_LoadFile(name, (void**)&buffer);
	if (buffer == NULL){
		Com_DPrintf("%s couldn't read image form %s\n", __func__, name);
		return qfalse;
	}

	data = stbi_load_from_memory(buffer, len, &w, &h, &bbp, STBI_rgb_alpha);
	if (data == NULL)
	{
		Com_DPrintf("%s couldn't load data from %s: %s!\n", __func__, name, stbi_failure_reason());
		FS_FreeFile(buffer);
		return qfalse;
	}

	FS_FreeFile(buffer);

	Com_DPrintf("%s() loaded: %s\n", __func__, name);

	*pic = data;
	*width = w;
	*height = h;
	return qtrue;
}

image_t* R_LoadDDS(char* texName, uint type) {

	ddsFileHeader_t			*header;
	ddsFileHeaderDXT10_t	*headerDXT10;
	uint					len, i, width, height, skipMip;
	uint					format, intFormat, blockSize = 16, mipLevel, texSize, hdrBitsCount = 0, dataType;
	image_t					*image;
	qboolean				compressed, hdr;
	byte					*buf, *imagedata;
	uint					hash = Com_HashKey(texName);

	if (!texName)
		return NULL;

	len = strlen(texName);
	
	if (len < 5)
		return NULL;

	for (i = 0, image = gltextures; i < numgltextures; i++, image++){

		if (image->hash == hash){

			if (!b_stricmp(image->name, texName)) {

				image->registration_sequence = registration_sequence;
				return image;
			}
		}
	}

	len = FS_LoadFile(texName, (void**)&buf);

	if (!buf)
		return NULL;

	if (len <= sizeof(ddsFileHeader_t) + 4){

		FS_FreeFile(buf);
		Com_Printf("R_LoadDDS: file too short (%s)\n", texName);
		return NULL;
	}

	if (strncmp(buf, "DDS ", 4) != 0){

		FS_FreeFile(buf);
		Com_Printf("R_LoadDDS: not a direct draw surface file (%s)\n", texName);
		return NULL;
	}

	header = (ddsFileHeader_t*)(buf + 4);

	compressed = qfalse;
	hdr = qfalse;

	if (header->ddspf.dwFlags & DDSF_FOURCC){

		compressed = qtrue;

		switch (header->ddspf.dwFourCC){

	/*	case 113: // D3DFMT_A16B16G16R16F        
			intFormat = GL_RGBA16F;
			format = GL_RGBA;
			compressed = qfalse;
			hdr = qtrue;
			hdrBitsCount = 64;
			break;
	*/	
		case 116: // D3DFMT_A32B32G32R32F
			intFormat		= GL_RGBA32F;
			format			= GL_RGBA;
			compressed		= qfalse;
			hdr				= qtrue;
			hdrBitsCount	= 128;
			break;

		case DDS_MAKEFOURCC('D', 'X', 'T', '1'):
		
			intFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			blockSize = 8;
			break;
		
		case DDS_MAKEFOURCC('D', 'X', 'T', '3'):

			intFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			break;

		case DDS_MAKEFOURCC('D', 'X', 'T', '5'):

			intFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;

		case DDS_MAKEFOURCC('D', 'X', '1', '0'):

			headerDXT10 = (ddsFileHeaderDXT10_t*)(buf + 4 + sizeof(ddsFileHeader_t));

			if (headerDXT10->dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) {
				intFormat		= GL_RGBA32F;
				format			= GL_RGBA;
				compressed		= qfalse;
				hdr				= qtrue;
				hdrBitsCount	= 128;
			}
			if (headerDXT10->dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM) {
				intFormat		= GL_RGBA8;
				format			= GL_BGRA;
				compressed		= qfalse;
			}			

			if (headerDXT10->dxgiFormat == DXGI_FORMAT_BC1_UNORM) {
				intFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				blockSize = 8;			
			}
			if (headerDXT10->dxgiFormat == DXGI_FORMAT_BC2_UNORM)
				intFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;

			if (headerDXT10->dxgiFormat == DXGI_FORMAT_BC3_UNORM)
				intFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

			if (headerDXT10->dxgiFormat == DXGI_FORMAT_BC7_UNORM)
				intFormat = GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
			
			if (headerDXT10->dxgiFormat == DXGI_FORMAT_BC6H_UF16)
				intFormat = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB;

			if (headerDXT10->dxgiFormat == DXGI_FORMAT_BC6H_SF16)
				intFormat = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB;
 
			if ((headerDXT10->dxgiFormat != DXGI_FORMAT_BC1_UNORM) && //dxt1
				(headerDXT10->dxgiFormat != DXGI_FORMAT_BC2_UNORM) && //dxt3
				(headerDXT10->dxgiFormat != DXGI_FORMAT_BC3_UNORM) && //dxt5
				(headerDXT10->dxgiFormat != DXGI_FORMAT_BC7_UNORM) && //bc7
				(headerDXT10->dxgiFormat != DXGI_FORMAT_BC6H_UF16) && //bc6u
				(headerDXT10->dxgiFormat != DXGI_FORMAT_BC6H_SF16) && //bc6s
				(headerDXT10->dxgiFormat != DXGI_FORMAT_R32G32B32A32_FLOAT)&& //rgba32f
				(headerDXT10->dxgiFormat != DXGI_FORMAT_B8G8R8A8_UNORM)) { //rgba8

				Com_Printf("R_LoadDDS: incorrect 'headerDXT10->dxgiFormat' = %i (%s)\n", headerDXT10->dxgiFormat, texName);
				return NULL;
			}
			if (headerDXT10->resourceDimension != D3D10_RESOURCE_DIMENSION_TEXTURE2D){

				Com_Printf("R_LoadDDS: incorrect 'headerDXT10->resourceDimension' = %i (supported 3 'Texture2D') (%s)\n", headerDXT10->resourceDimension, texName);
				return NULL;
			}
			break;

		default:
			FS_FreeFile(buf);
			Com_Printf("R_LoadDDS: invalid compressed internal format (supported DXT1, DXT3, DXT5, BPTC) (%s)\n", texName);
			return NULL;
		}
	}
	else if ((header->ddspf.dwFlags & DDSF_RGBA) && header->ddspf.dwRGBBitCount == 32){

		intFormat	= GL_RGBA8;
		format		= GL_BGRA;
	}
	else if ((header->ddspf.dwFlags & DDSF_RGB) && header->ddspf.dwRGBBitCount == 24){

		intFormat	= GL_RGB8;
		format		= GL_BGR;
	}
	else{

		FS_FreeFile(buf);
		Com_Printf("R_LoadDDS: invalid uncompressed internal format (%s) header (%i) dwRGBBitCount (%i)\n", texName, header->ddspf.dwFlags, header->ddspf.dwRGBBitCount);
		return NULL;
	}

	// find a free image_t
	for (i = 0, image = gltextures; i < numgltextures; i++, image++){
		if (!image->texnum)
			break;
	}
	if (i == numgltextures){
		if (numgltextures == MAX_GLTEXTURES)
			VID_Error(ERR_FATAL, "MAX_GLTEXTURES");
		numgltextures++;
	}

	width = header->dwWidth;
	height = header->dwHeight;
	
	image = &gltextures[i];
	strcpy(image->name, texName);

	image->width = width;
	image->height = height;
	image->upload_width = width;
	image->upload_height = height;
	image->type = type;
	image->hash = hash;
	image->floatTex = hdr;
	image->compressed = compressed;
	image->has_alpha = qtrue;
	image->paletted = qfalse;
	image->intFormat = intFormat;

	if (image->type == it_pic){

		byte *data, *pal;
		char s[MAX_QPATH];
		int pcx_w, pcx_h;
		strcpy(s, texName);
		s[strlen(s) - 4] = 0;
		strcat(s, ".pcx");
		LoadPCX(s, &data, &pal, &pcx_w, &pcx_h);

		image->picScale_w = 1.0;
		image->picScale_h = 1.0;

		if (pcx_w > 0 && pcx_h > 0){

			image->picScale_w = (float)pcx_w / image->width;
			image->picScale_h = (float)pcx_h / image->height;

			if (data)
				free(data);

			if (pal)
				free(pal);
		}
	}

	image->sl = 0;
	image->sh = 1;
	image->tl = 0;
	image->th = 1;

	imagedata = buf + sizeof(ddsFileHeader_t) + 4;

	if (header->ddspf.dwFourCC == DDS_MAKEFOURCC('D', 'X', '1', '0'))
		imagedata += sizeof(ddsFileHeaderDXT10_t);

	if (!compressed) {
		if (!hdr) {
			blockSize = header->ddspf.dwRGBBitCount / 8;
			dataType = GL_UNSIGNED_BYTE;
			image->dataType = dataType;
		}
		else {
			blockSize = hdrBitsCount / 8;
			dataType = GL_FLOAT;
			image->dataType = dataType;
		}
	} else
		image->dataType = GL_COMPRESSED_RGBA;

	if (header->dwCaps2 & DDSCAPS2_CUBEMAP) {
		glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &image->texnum);
		image->texType = GL_TEXTURE_CUBE_MAP_ARRAY;
	}
	else {
		glCreateTextures(GL_TEXTURE_2D, 1, &image->texnum);
		image->texType = GL_TEXTURE_2D;
	}
	int maxSize = max(width, height);

	if (image->type != it_part && image->type != it_sky && maxSize >= 1024)
		skipMip = min(r_textureQuality->integer, header->dwMipMapCount);
	else
		skipMip = 0;
	
	if (header->dwFlags & DDSF_MIPMAPCOUNT) {

		image->numMips = header->dwMipMapCount;

		glTextureParameteri(image->texnum, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(image->texnum, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(image->texnum, GL_TEXTURE_BASE_LEVEL, skipMip);
		glTextureParameteri(image->texnum, GL_TEXTURE_MAX_LEVEL, image->numMips - 1);
		glTextureParameterf(image->texnum, GL_TEXTURE_LOD_BIAS, r_textureLodBias->value);
		glTextureParameterf(image->texnum, GL_TEXTURE_MAX_ANISOTROPY, r_textureAnisotropy->value);
	}
	else {
		image->numMips = 1;
		glTextureParameteri(image->texnum, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(image->texnum, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	if (header->dwCaps2 & DDSCAPS2_CUBEMAP){

		// fix tex size for uncompressed cubemaps
		image->width = width * 6;
		image->upload_width = width * 6;

		glTextureParameteri(image->texnum, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTextureParameteri(image->texnum, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		
		uint numLayers = header->dwDepth;

		glTextureStorage3D(image->texnum, image->numMips, intFormat, width, height, 6);
	
		int faceOffset = 0;

		for (int face = 0; face < 6; face++){

			mipLevel = 0;
			width = header->dwWidth;
			height = header->dwHeight;

			for (i = 0; i < image->numMips; i++){

				texSize = 0;
				
				if (compressed){
					texSize = ((width + 3) >> 2) * ((height + 3) >> 2) * blockSize;
					glCompressedTextureSubImage3D(image->texnum, mipLevel, 0, 0, face, width, height, 1, intFormat, texSize, imagedata + faceOffset);
				}
				else{
					texSize = width * height * blockSize;
					glTextureSubImage3D(image->texnum, mipLevel, 0, 0, face, width, height, 1, format, dataType, imagedata + faceOffset);
				}

				width >>= 1;
				height >>= 1;

				if (width < 1)
					width = 1;

				if (height < 1)
					height = 1;

				faceOffset += texSize;
				mipLevel++;
			}
		}
	}
	else{

		if (image->type == it_part){
			glTextureParameteri(image->texnum, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(image->texnum, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else{
			glTextureParameteri(image->texnum, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(image->texnum, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		glTextureStorage2D(image->texnum, image->numMips, intFormat, width, height);

		mipLevel = 0;
		for (i = 0; i < image->numMips; i++){

			texSize = 0;

			if (compressed){
				texSize = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
				glCompressedTextureSubImage2D(image->texnum, mipLevel, 0, 0, width, height, intFormat, texSize, imagedata);
			}
			else{
				texSize = width * height * blockSize;
				glTextureSubImage2D(image->texnum, mipLevel, 0, 0, width, height, format, dataType, imagedata);

			}

			width >>= 1;
			height >>= 1;

			if (width < 1)
				width = 1;

			if (height < 1)
				height = 1;

			imagedata += texSize;
			mipLevel++;
		}
	}

	image->handle = glGetTextureHandleARB(image->texnum);
	glMakeTextureHandleResidentARB(image->handle);

	qglObjectLabel(GL_TEXTURE, image->texnum, strlen(image->name), image->name);

	FS_FreeFile(buf);

	return image;

}

int CalcMipmapCount(int w, int h)
{
	int width, height, mipcount;

	// mip-maps can't exceeds 16
	for (mipcount = 0; mipcount < 16; mipcount++)
	{
		width = max(1, (w >> mipcount));
		height = max(1, (h >> mipcount));
		if (width == 1 && height == 1)
			break;
	}

	return mipcount + 1;
}

void R_CaptureColorBuffer(){
		
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	glCopyTextureSubImage2D(r_hdrScreenCopy->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);
}


/*
===============
GL_ImageList_f
===============
*/

void GL_ImageList_f(void)
{
	int i, comptexSize = 0, level, texSize, dataSize = sizeof(byte);
	uint totalTexturesSize = 0, numChannels = 4;
	image_t *image;

	Com_Printf("------------------\n");

	for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
		
		if (image->texnum <= 0)
			continue;

		if (image->compressed) {

			for (level = 0; level < image->numMips; level++) {
				glGetTextureLevelParameteriv(image->texnum, level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &comptexSize);
				totalTexturesSize += comptexSize;
			}

		}
		else {
			texSize = image->upload_width * image->upload_height;
			if ((image->type != it_pic) && (image->type != it_screen)) {
				texSize *= 4; // + mipmaps size
				texSize /= 3;
			}

			switch (image->intFormat) {

			case GL_R16F:
			case GL_R32F:
				numChannels = 1;
				break;

			case GL_RG16F:
			case GL_RG32F:
			case GL_DEPTH24_STENCIL8:
				numChannels = 2;
				break;

			case GL_RGB8:
			case GL_RGB16F:
			case GL_RGB32F:
				numChannels = 3;
				break;
			}

			switch (image->intFormat) {
			case GL_R16F:
			case GL_RG16F:
			case GL_RGB16F:
			case GL_RGBA16F:
			case GL_R32F:
			case GL_RG32F:
			case GL_RGB32F:
			case GL_RGBA32F:
			case GL_R11F_G11F_B10F:
				dataSize = sizeof(float);
			case GL_DEPTH24_STENCIL8:
				dataSize = sizeof(int);
				break;
			}

			totalTexturesSize += texSize * numChannels * dataSize;
		}

		switch (image->texType) {
		case GL_TEXTURE_2D:
			Com_Printf("2D    ");
			break;
		case GL_TEXTURE_CUBE_MAP:
			Com_Printf("CUBE  ");
			break;
		case GL_TEXTURE_CUBE_MAP_ARRAY:
			Com_Printf("CUBEA ");
			break;
		case GL_TEXTURE_RECTANGLE:
			Com_Printf("RECT  ");
			break;
		}

		switch (image->intFormat) {
		case GL_SRGB8:
			Com_Printf("SRGB8  ");
			break;
		case GL_RGBA8:
			Com_Printf("RGBA8  ");
			break;
		case GL_RGB8:
			Com_Printf("RGB8   ");
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			Com_Printf("DXT1   ");
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			Com_Printf("DXT3   ");
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			Com_Printf("DXT5   ");
			break;
		case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB:
			Com_Printf("BC6S   ");
			break;
		case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB:
			Com_Printf("BC6U   ");
			break;
		case GL_COMPRESSED_RGBA_BPTC_UNORM_ARB:
			Com_Printf("BC7    ");
			break;
		case GL_RG16F:
			Com_Printf("RG16F  ");
			break;
		case GL_R16F:
			Com_Printf("R16F   ");
			break;
		case GL_R32F:
			Com_Printf("R32F   ");
			break;
		case GL_DEPTH24_STENCIL8:
			Com_Printf("D24S8   ");
			break;
		case GL_RGBA16F:
			Com_Printf("RGBA16F ");
			break;
		case GL_RGB16F:
			Com_Printf("RGB16F  ");
			break;
		case GL_R11F_G11F_B10F:
			Com_Printf("RG11B10F ");
		}

		switch (image->dataType) {
		case GL_UNSIGNED_BYTE:
			Com_Printf("GL_UBYTE ");
			break;
		case GL_FLOAT:
			Com_Printf("GL_FLOAT ");
			break;
		case GL_COMPRESSED_RGBA:
			Com_Printf("GL_COMPR ");
			break;
		case GL_UNSIGNED_INT_24_8:
			Com_Printf("GL_UINT24_8 ");
		}

		Com_Printf("%ix%i mips: %i %s\n", image->upload_width, image->upload_height, image->numMips, image->name);
	}
	Com_Printf("%i MB total image memory\n",totalTexturesSize>>20);
}


/*
==============
LoadPCX
==============
*/
void LoadPCX(char *filename, byte ** pic, byte ** palette, int *width, int *height)
{
	byte *raw;
	pcx_t *pcx;
	int x, y;
	int len;
	int dataByte, runLength;
	byte *out, *pix;

	*pic = NULL;
	*palette = NULL;

	// 
	// load the file
	// 
	len = FS_LoadFile(filename, (void **) &raw);
	if (!raw) {
		Con_Printf(PRINT_DEVELOPER, "Bad pcx file %s\n", filename);
		return;
	}

	if (!len) {
		FS_FreeFile(raw);
		Con_Printf(PRINT_DEVELOPER, "Bad pcx file %s\n", filename);
		return;
	}
	// 
	// parse the PCX file
	// 
	pcx = (pcx_t *) raw;

	pcx->xmin = LittleShort(pcx->xmin);
	pcx->ymin = LittleShort(pcx->ymin);
	pcx->xmax = LittleShort(pcx->xmax);
	pcx->ymax = LittleShort(pcx->ymax);
	pcx->hres = LittleShort(pcx->hres);
	pcx->vres = LittleShort(pcx->vres);
	pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
	pcx->palette_type = LittleShort(pcx->palette_type);

	raw = &pcx->data;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640 || pcx->ymax >= 480) {
		Com_Printf("Bad pcx file %s\n", filename);
		return;
	}

	out = malloc((pcx->ymax + 1) * (pcx->xmax + 1));

	*pic = out;

	pix = out;

	if (palette) {
		*palette = malloc(768);
		Q_memcpy(*palette, (byte *) pcx + len - 768, 768);
	}

	if (width)
		*width = pcx->xmax + 1;
	if (height)
		*height = pcx->ymax + 1;

	for (y = 0; y <= pcx->ymax; y++, pix += pcx->xmax + 1) {
		for (x = 0; x <= pcx->xmax;) {
			dataByte = *raw++;

			if ((dataByte & 0xC0) == 0xC0) {
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			} else
				runLength = 1;

			while (runLength-- > 0)
				pix[x++] = dataByte;
		}

	}

	if (raw - (byte *) pcx > len) {
		Con_Printf(PRINT_DEVELOPER, "PCX file %s was malformed", filename);
		free(*pic);
		*pic = NULL;
	}

	FS_FreeFile(pcx);
}

/*
================
GL_LoadWal
================
*/
image_t *GL_LoadWal(char *name)
{
	miptex_t *mt;
	int width, height, ofs;
	image_t *image;

	FS_LoadFile(name, (void **)&mt);

	if (!mt) {
		Com_Printf("GL_FindImage: can't load %s\n", name);
		return r_missingTexture;
	}

	width = LittleLong(mt->width);
	height = LittleLong(mt->height);
	ofs = LittleLong(mt->offsets[0]);


	image = GL_LoadPic(name, (byte *)mt + ofs, width, height, it_wall, 8, 0);

	FS_FreeFile((void *)mt);

	return image;
}

/*
===============
GL_Upload32

Returns has_alpha
===============
*/
qboolean GL_Upload32(uint texnum, unsigned *data, int width, int height, qboolean mipmap, uint ClampMode){

	int		samples, c, i;
	byte	*scan;

	// scan the texture for any non-255 alpha
	c = width * height;
	scan = ((byte *) data) + 3;
	samples = 3;

	for (i = 0; i < c; i++, scan += 4) {
		if (*scan != 255) {
			samples = 4;
			break;
		}
	}

	int numMips = CalcMipmapCount(width, height);
	glTextureStorage2D(texnum, numMips, GL_RGBA8, width, height);
	glTextureSubImage2D(texnum, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTextureParameteri(texnum, GL_TEXTURE_WRAP_S, ClampMode);
	glTextureParameteri(texnum, GL_TEXTURE_WRAP_T, ClampMode);

	if (mipmap){
		glGenerateTextureMipmap(texnum);
		glTextureParameterf(texnum, GL_TEXTURE_MAX_ANISOTROPY,	r_textureAnisotropy->value);
		glTextureParameterf(texnum, GL_TEXTURE_LOD_BIAS,		r_textureLodBias->value);
		glTextureParameteri(texnum, GL_TEXTURE_MIN_FILTER,		GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(texnum, GL_TEXTURE_MAG_FILTER,		GL_LINEAR);
		glTextureParameteri(texnum, GL_TEXTURE_BASE_LEVEL,		0);
		glTextureParameteri(texnum, GL_TEXTURE_MAX_LEVEL,		numMips-1);
	}
	else{
		glTextureParameteri(texnum, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(texnum, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	return (samples == 4);
}


/*
===============
GL_Upload8

Returns has_alpha
===============
*/

qboolean GL_Upload8(uint texnum, byte * data, int width, int height, qboolean mipmap)
{
	static unsigned trans[512 * 256];
	int i, s;
	int p;

	s = width * height;

	if (s > sizeof(trans) / 4)
		VID_Error(ERR_DROP, "GL_Upload8: too large %d width %d height.", width, height);

	for (i = 0; i < s; i++) {
		p = data[i];
		trans[i] = d_8to24table[p];

		if (p == 255) {			// transparent, so scan around for another 
								// color
			// to avoid alpha fringes
			// FIXME: do a full flood fill so mips work...
			if (i > width && data[i - width] != 255)
				p = data[i - width];
			else if (i < s - width && data[i + width] != 255)
				p = data[i + width];
			else if (i > 0 && data[i - 1] != 255)
				p = data[i - 1];
			else if (i < s - 1 && data[i + 1] != 255)
				p = data[i + 1];
			else
				p = 0;
			// copy rgb components
			((byte *) & trans[i])[0] = ((byte *) & d_8to24table[p])[0];
			((byte *) & trans[i])[1] = ((byte *) & d_8to24table[p])[1];
			((byte *) & trans[i])[2] = ((byte *) & d_8to24table[p])[2];
		}
	}


	return GL_Upload32(texnum, trans, width, height, mipmap, GL_REPEAT);
}


// Knightmare - free single pic
void R_FreePic(char* name)
{
	int		i;
	image_t* image;

	for (i = 0, image = gltextures; i < numgltextures; i++, image++)
	{
		if (!image->registration_sequence)
			continue;		// free image_t slot
		
		if (image->type != it_pic)
			continue;		// only free pics
		
		if (!b_stricmp(name, image->bare_name)){
			// free it
			glMakeTextureHandleNonResidentARB(image->handle);
			qglDeleteTextures(1, &image->texnum);
			memset(image, 0, sizeof(*image));
			return;
		}
	}
}

/*
================
GL_LoadPic

This is also used as an entry point for the generated r_blackTexture1x1
================
*/

image_t* GL_LoadPic(char* name, byte* pic, int width, int height, imagetype_t type, int bits, uint _hash)
{
	image_t* image;

	int i;
	int len;
	char s[128];

	// find a free image_t
	for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
		if (!image->texnum)
			break;
	}

	if (i == numgltextures) {
		if (numgltextures == MAX_GLTEXTURES)
			VID_Error(ERR_DROP, "MAX_GLTEXTURES");
		numgltextures++;
	}

	image = &gltextures[i];

	if (strlen(name) >= sizeof(image->name))
		VID_Error(ERR_DROP, "Draw_LoadPic: \"%s\" is too long", name);
	strcpy(image->name, name);
	image->registration_sequence = registration_sequence;

	image->width = width;
	image->height = height;
	image->picScale_w = 1.0;
	image->picScale_h = 1.0;
	image->type = type;
	image->floatTex = qfalse;
	image->compressed = qfalse;
	
	image->dataType = GL_UNSIGNED_BYTE;
	image->texType = GL_TEXTURE_2D;
	image->intFormat = GL_RGBA8;
	if (_hash)
		image->hash = _hash;
	else
		image->hash = Com_HashKey(name);

	len = strlen(name);
	strcpy(s, name);

	if (image->type == it_pic && bits >= 24) //Scale hi-res pics
	{
		byte* pics, * palettes;
		int pcx_w, pcx_h;
		strcpy(s, name);
		s[strlen(s) - 4] = 0;
		strcat(s, ".pcx");
		LoadPCX(s, &pics, &palettes, &pcx_w, &pcx_h);

		image->picScale_w = 1.0;
		image->picScale_h = 1.0;

		if (pcx_w > 0 && pcx_h > 0) {

			image->picScale_w = (float)pcx_w / image->width;
			image->picScale_h = (float)pcx_h / image->height;

			if (pics)
				free(pics);
			if (palettes)
				free(palettes);
		}
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &image->texnum);
	
	uint ClampMode = GL_REPEAT;
	if (image->type == it_part)
		ClampMode = GL_CLAMP_TO_EDGE;

		if (bits == 8)
			image->has_alpha = GL_Upload8(image->texnum, pic, width, height, image->type != it_pic);
		else 									
			image->has_alpha = GL_Upload32(image->texnum,(unsigned *) pic, width, height, image->type != it_pic, ClampMode);
					
		image->upload_width = width;	
		image->upload_height = height;

		image->paletted = uploaded_paletted;
		image->sl = 0;
		image->sh = 1;
		image->tl = 0;
		image->th = 1;

		image->handle = glGetTextureHandleARB(image->texnum);
		glMakeTextureHandleResidentARB(image->handle);
		
	return image;
}


/*
===============
GL_FindImage

Finds or loads the given image
===============
*/
char override = 0;
image_t *GL_FindImage(char *name, imagetype_t type)
{
	image_t *image;
	int i, len, width, height;
	uint hash = Com_HashKey(name);
	byte *pic, *palette;

	if (!name)
		return NULL;	

	len = strlen(name);

	if (len < 5)
		return NULL;			

	// look for it
	for (i = 0, image = gltextures; i < numgltextures; i++, image++){

		if (image->hash == hash)
		{
			if (!b_stricmp(image->name, name)){

				image->registration_sequence = registration_sequence;
				return image;
			}
		}
	}
	// 
	// load the pic from disk
	// 
	pic = NULL;
	palette = NULL;
	
	if (type == it_mipmap)
		goto next;

	if (strcmp(name + len - 4, ".jpg") && strcmp(name + len - 4, ".tga") && !override) {

		char s[128];
		override = 1;
		strcpy(s, name);
		s[strlen(s) - 4] = 0;
		strcat(s, ".tga");

		image = GL_FindImage(s, type);
		if (image) {
			override = 0;
			return image;
		}
	}
	if (strcmp(name + len - 4, ".jpg") && strcmp(name + len - 4, ".tga") && !override) {

		char s[128];
		override = 1;
		strcpy(s, name);
		s[strlen(s) - 4] = 0;
		strcat(s, ".jpg");

		image = GL_FindImage(s, type);
		if (image) {
			override = 0;
			return image;
		}
	}

	override = 0;
	if (!strcmp(name + len - 4, ".pcx")) {
		LoadPCX(name, &pic, &palette, &width, &height);

		if (!pic)
			return NULL;
	
	image = GL_LoadPic(name, pic, width, height, type, 8, Com_HashKey(name));

	} else if (!strcmp(name + len - 4, ".wal")) {

		image = GL_LoadWal(name);

	} else if (!strcmp(name + len - 4, ".tga")) {
		STB_LoadTexture(name, &pic, &width, &height);
		if (!pic)
			return NULL;

		image = GL_LoadPic(name, pic, width, height, type, 32, Com_HashKey(name));
		
	}

	else if (!strcmp(name + len - 4, ".jpg")) {
	next:
		STB_LoadTexture(name, &pic, &width, &height);
		if (!pic)
			return NULL;

		image = GL_LoadPic(name, pic, width, height, it_pic, 24, Com_HashKey(name));
	} 
	else 
		return NULL;


	if (pic)
		free(pic);

	if (palette)
		free(palette);

	return image;
}

image_t* GL_FindImage2(char* name, imagetype_t type)
{
	image_t	*image;
	int		i, len, width, height;
	byte	*pic = NULL;

	if (!name)
		return NULL;

	len = strlen(name);

	if (len < 5)
		return NULL;

	for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
		if (image->type != type)
			continue;
		if (!strcmp(name, image->name)) {
			image->registration_sequence = registration_sequence;
			return image;
		}
	}

	STB_LoadTexture(name, &pic, &width, &height);
	
	if (!pic)
		return NULL;

	image = GL_LoadPic(name, pic, width, height, type, 24, Com_HashKey(name));
	
	if (!image)
		image = r_missingTexture;

	if (pic)
		free(pic);

	return image;
}




/*
===============
R_RegisterSkin
===============
*/

struct image_s *R_RegisterSkin(char *name){

	image_t *img;
	char gl[128];

	strcpy(gl, name);
	gl[strlen(gl) - 4] = 0;
	strcat(gl, ".dds");
	img = R_LoadDDS(gl, it_skin);

	if (!img)
		img = r_missingTexture;
	
	return img;
}

struct image_s *R_RegisterPlayerBump (char *name){

	image_t	*img;
	char	gl[48];
	
	strcpy(gl, name);
	gl[strlen(gl) - 4] = 0;
	strcat(gl, "_bump.dds");
	img = R_LoadDDS(gl, it_skin);

	if(!img)
		img = r_defBump;

	return img;
}

/*
================
GL_FreeUnusedImages

Any image that was not touched on this registration sequence
will be freed.
================
*/

void GL_FreeUnusedImages(void)
{
	int i;
	image_t *image;

	// image cache
	//=========================
	r_blackTexture1x1->registration_sequence = registration_sequence;
	r_missingTexture->registration_sequence = registration_sequence;

	for (i = 0; i < MAX_CAUSTICS; i++) {
		r_caustic[i]->registration_sequence = registration_sequence;
	}

	for (i = 0; i < MAX_WATER_NORMALS; i++) {
		r_waterNormals[i]->registration_sequence = registration_sequence;
	}


	for (i = 0; i < MAX_FLY; i++) {
		fly[i]->registration_sequence = registration_sequence;
	}

	for (i = 0; i < MAX_FLAMEANIM; i++) {
		flameanim[i]->registration_sequence = registration_sequence;
	}

	for (i = 0; i < MAX_BLOOD; i++) {
		r_blood[i]->registration_sequence = registration_sequence;
	}
	
	for (i = 0; i < MAX_xBLOOD; i++) {
		r_xblood[i]->registration_sequence = registration_sequence;
	}


	for (i = 0; i < MAX_EXPLODE; i++) {
		r_explode[i]->registration_sequence = registration_sequence;
	}

	for (i = 0; i < MAX_BFG_EXPL; i++) {
		r_bfg_expl[i]->registration_sequence = registration_sequence;
	}

	for (i = 0; i < DECAL_MAX; i++) {
		r_decalTexture[i]->registration_sequence = registration_sequence;
	}
	
	for (i = 0; i < PT_MAX; i++) {
		r_particleTexture[i]->registration_sequence = registration_sequence;
	}
	
	for (i = 0; i < MAX_SHELLS; i++){
		r_texshell[i]->registration_sequence = registration_sequence;
	}
	
	for(i=0; i<MAX_GLOBAL_FILTERS; i++)
			r_lightCubeMap[i]->registration_sequence = registration_sequence;

	r_distort->registration_sequence = registration_sequence;
	r_defBump->registration_sequence = registration_sequence;
	r_conBump->registration_sequence = registration_sequence;
	r_envTex->registration_sequence = registration_sequence;
	r_whiteMap->registration_sequence = registration_sequence;
	skinBump->registration_sequence = registration_sequence;
	r_laser_normal->registration_sequence = registration_sequence;
	r_lensDirt->registration_sequence = registration_sequence;

	for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
		if (image->registration_sequence == registration_sequence)
			continue;			// used this sequence

		if (!image->registration_sequence)
			continue;			// free image_t slot

		if (image->type == it_pic)
			continue;			// don't free pics

		if (image->type == it_screen)
			continue;			// don't free fbos images

		// free it
		glMakeTextureHandleNonResidentARB(image->handle);
		qglDeleteTextures(1, &image->texnum);
		memset(image, 0, sizeof(*image));
	}
}


/*
===============
Draw_GetPalette
===============
*/
int Draw_GetPalette(void)
{
	int i;
	int r, g, b;
	unsigned v;
	byte  *pal;

	// load the palette
	pal = (byte*)r_originalPalette;

	for (i = 0; i < 256; i++) {
		r = pal[i * 3 + 0];
		g = pal[i * 3 + 1];
		b = pal[i * 3 + 2];

		v = (255 << 24) + (r << 0) + (g << 8) + (b << 16);
		d_8to24table[i] = LittleLong(v);

		d_8to24tablef[i][0] = r * 0.003921568627450980392156862745098f;
		d_8to24tablef[i][1] = g * 0.003921568627450980392156862745098f;
		d_8to24tablef[i][2] = b * 0.003921568627450980392156862745098f;

	}

	d_8to24table[255] &= LittleLong(0xffffff);	// 255 is transparent

	return 0;
}

/*
===============
GL_InitImages
===============
*/
void GL_InitImages(void)
{
	registration_sequence = 1;
	Draw_GetPalette();

}

/*
===============
GL_ShutdownImages

===============
*/
void GL_ShutdownImages(void) {
	int i;
	image_t *image;

	for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
	//	if (!image->registration_sequence)
		//	continue;			// free image_t slot

		glMakeTextureHandleNonResidentARB(image->handle);
		// free it
		qglDeleteTextures(1, (GLuint*)&image->texnum);
		memset(image, 0, sizeof(*image));
	}
	numgltextures = 0;

	if (gl_lms.handle) {
		glMakeTextureHandleNonResidentARB(gl_lms.handle[0]);
		glMakeTextureHandleNonResidentARB(gl_lms.handle[1]);
		glMakeTextureHandleNonResidentARB(gl_lms.handle[2]);
	}
	qglDeleteTextures(1, &gl_lms.texnum[0]);
	qglDeleteTextures(1, &gl_lms.texnum[1]);
	qglDeleteTextures(1, &gl_lms.texnum[2]);

}
