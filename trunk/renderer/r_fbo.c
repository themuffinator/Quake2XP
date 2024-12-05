/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/

// r_fbo.c

#include "r_local.h"

static void R_FB_Check() {
	const char* s;
	GLenum		code;

	code = qglCheckFramebufferStatus(GL_FRAMEBUFFER);

	// an error occured
	switch (code) {
	case GL_FRAMEBUFFER_COMPLETE:
		Com_Printf(S_COLOR_WHITE"succeeded\n");
		return;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		s = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		s = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		s = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		s = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		s = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		s = "GL_FRAMEBUFFER_UNSUPPORTED";
		break;
	case GL_FRAMEBUFFER_UNDEFINED:
		s = "GL_FRAMEBUFFER_UNDEFINED";
		break;
	}
	Com_Printf(S_COLOR_RED"Failed!:" S_COLOR_MAGENTA " %s\n", s);
}

void R_FboListing_f(void) {
	rbo_t	*rbo;
	fbo_t	*fbo;
	int i;

	Com_Printf(S_COLOR_YELLOW"RBO List:\n");
	for (i = 0, rbo = rb.r_rbo; i < rb.r_numRbos; i++, rbo++) {
		Com_Printf(">" S_COLOR_GREEN "%s %ix%i\n", rbo->name, rbo->width, rbo->height);
	}
	Com_Printf(S_COLOR_YELLOW"FBO List:\n");
	for (i = 0, fbo = fb.r_fbo; i < fb.r_numFbos; i++, fbo++) {
		Com_Printf(">" S_COLOR_GREEN "%s\n", fbo->name);
	}
}

void R_ShotdownFBO(void) {
	rbo_t *rbo;
	fbo_t *fbo;
	int i;

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

	for (i = 0, rbo = rb.r_rbo; i < rb.r_numRbos; i++, rbo++) {
		qglDeleteRenderbuffers(1, &rbo->id);
		memset(rbo, 0, sizeof(*rbo));
	}
	for (i = 0, fbo = fb.r_fbo; i < fb.r_numFbos; i++, fbo++) {
		qglDeleteFramebuffers(1, &fbo->id);
		memset(fbo, 0, sizeof(*fbo));
	}
}

rbo_t *R_Create_RBO(const char *name, GLuint internalFormat, int width, int height) {
	rbo_t *rbo;
	int i;

	switch (internalFormat) {
	case GL_R8:
	case GL_R8I:
	case GL_R8UI:
	case GL_R16:
	case GL_R16I:
	case GL_R16UI:
	case GL_R16F:
	case GL_R32I:
	case GL_R32UI:
	case GL_R32F:

	case GL_RG8:
	case GL_RG8I:
	case GL_RG8UI:
	case GL_RG16:
	case GL_RG16I:
	case GL_RG16UI:
	case GL_RG16F:
	case GL_RG32I:
	case GL_RG32UI:
	case GL_RG32F:

	case GL_RGB8:
	case GL_RGB8I:
	case GL_RGB8UI:
	case GL_RGB10_A2:
	case GL_R11F_G11F_B10F:
	case GL_RGB16:
	case GL_RGB16I:
	case GL_RGB16UI:
	case GL_RGB16F:
	case GL_RGB32I:
	case GL_RGB32UI:
	case GL_RGB32F:

	case GL_RGBA8:
	case GL_RGBA8I:
	case GL_RGBA8UI:
	case GL_RGBA16:
	case GL_RGBA16I:
	case GL_RGBA16UI:
	case GL_RGBA16F:
	case GL_RGBA32I:
	case GL_RGBA32UI:
	case GL_RGBA32F:

	case GL_DEPTH_COMPONENT16:
	case GL_DEPTH_COMPONENT24:
	case GL_DEPTH_COMPONENT32F:

	case GL_DEPTH24_STENCIL8:
	case GL_DEPTH32F_STENCIL8:

	case GL_STENCIL_INDEX1:
	case GL_STENCIL_INDEX4:
	case GL_STENCIL_INDEX8:
	case GL_STENCIL_INDEX16:
		break;
	default:
		VID_Error(ERR_DROP, "R_Create_RBO: format 0x%x is non-renderable\n", internalFormat);
	}

	if (rb.r_numRbos == MAX_RBOS)
		VID_Error(ERR_DROP, "R_Create_RBO: MAX_RBOS hit");

	for (i = 0, rbo = rb.r_rbo; i < rb.r_numRbos; i++, rbo++) {
		if (!rbo->id)
			break;
	}

	if (i == rb.r_numRbos) {
		if (rb.r_numRbos == MAX_RBOS)
			VID_Error(ERR_DROP, "MAX_RBOS");
		rb.r_numRbos++;
	}

	strcpy(rbo->name, name);
	rbo->width = width;
	rbo->height = height;

	qglGenRenderbuffers(1, &rbo->id);
	qglBindRenderbuffer(GL_RENDERBUFFER, rbo->id);
	qglRenderbufferStorage(GL_RENDERBUFFER, internalFormat, rbo->width, rbo->height);
	qglObjectLabel(GL_RENDERBUFFER, rbo->id, strlen(rbo->name), rbo->name);
	qglBindRenderbuffer(GL_RENDERBUFFER, 0);

	return rbo;
}

static void R_AttachRBO(const rbo_t *rb, const GLenum attachment) {
	switch (attachment) {
	case GL_DEPTH_ATTACHMENT:
	case GL_STENCIL_ATTACHMENT:
	case GL_DEPTH_STENCIL_ATTACHMENT:
	case GL_COLOR_ATTACHMENT0:
	case GL_COLOR_ATTACHMENT1:
	case GL_COLOR_ATTACHMENT2:
	case GL_COLOR_ATTACHMENT3:
	case GL_COLOR_ATTACHMENT4:
	case GL_COLOR_ATTACHMENT5:
	case GL_COLOR_ATTACHMENT6:
	case GL_COLOR_ATTACHMENT7:
	case GL_COLOR_ATTACHMENT8:
	case GL_COLOR_ATTACHMENT9:
	case GL_COLOR_ATTACHMENT10:
	case GL_COLOR_ATTACHMENT11:
	case GL_COLOR_ATTACHMENT12:
	case GL_COLOR_ATTACHMENT13:
	case GL_COLOR_ATTACHMENT14:
	case GL_COLOR_ATTACHMENT15:
		break;
	default:
		VID_Error(ERR_DROP, "R_AttachRBO: invalid attachment point 0x%x\n", attachment);
	}
	qglFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rb->id);
}

void R_FB_AttachImage(const GLenum attachment, const image_t *image, const int index) {

	switch (attachment) {
	case GL_DEPTH_ATTACHMENT:
	case GL_STENCIL_ATTACHMENT:
	case GL_DEPTH_STENCIL_ATTACHMENT:
	case GL_COLOR_ATTACHMENT0:
	case GL_COLOR_ATTACHMENT1:
	case GL_COLOR_ATTACHMENT2:
	case GL_COLOR_ATTACHMENT3:
	case GL_COLOR_ATTACHMENT4:
	case GL_COLOR_ATTACHMENT5:
	case GL_COLOR_ATTACHMENT6:
	case GL_COLOR_ATTACHMENT7:
	case GL_COLOR_ATTACHMENT8:
	case GL_COLOR_ATTACHMENT9:
	case GL_COLOR_ATTACHMENT10:
	case GL_COLOR_ATTACHMENT11:
	case GL_COLOR_ATTACHMENT12:
	case GL_COLOR_ATTACHMENT13:
	case GL_COLOR_ATTACHMENT14:
	case GL_COLOR_ATTACHMENT15:
		break;
	default:
		VID_Error(ERR_DROP, "R_FB_AttachImage: invalid attachment point 0x%x\n", attachment);
	}

	switch (image->texType) {
	case GL_TEXTURE_2D:
	case GL_TEXTURE_RECTANGLE:
		qglFramebufferTexture2D(GL_FRAMEBUFFER, attachment, image->texType, image->texnum, 0);
		break;
	case GL_TEXTURE_CUBE_MAP:
		qglFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, image->texnum, 0);
		break;
	case GL_TEXTURE_3D:
		qglFramebufferTexture3D(GL_FRAMEBUFFER, attachment, image->texType, image->texnum, 0, index);
		break;
	}
}

fbo_t *R_Create_FBO(const char *name) {
	fbo_t *fbo;
	int i;
	
	if (rb.r_numRbos == MAX_FBOS)
		VID_Error(ERR_DROP, "R_Create_FBO: MAX_FBOS hit");

	for (i = 0, fbo = fb.r_fbo; i < fb.r_numFbos; i++, fbo++) {
		if (!fbo->id)
			break;
	}

	if (i == fb.r_numFbos) {
		if (fb.r_numFbos == MAX_FBOS)
			VID_Error(ERR_DROP, "MAX_FBOS");
		fb.r_numFbos++;
	}

	strcpy(fbo->name, name);

	qglGenFramebuffers	(1, &fbo->id);
	qglBindFramebuffer	(GL_FRAMEBUFFER, fbo->id);
	qglObjectLabel		(GL_FRAMEBUFFER, fbo->id, strlen(fbo->name), fbo->name);

	return fbo;
}

void R_InitPboBuffers() {

	Com_Printf("Initializing Pixel Buffers: " S_COLOR_GREEN "ok\n");
	
	// read from gpu
	qglGenBuffers(1, &pbo._fullScreen);
	qglBindBuffer(GL_PIXEL_PACK_BUFFER, pbo._fullScreen);
	qglBufferData(GL_PIXEL_PACK_BUFFER, vid.width * vid.height * 3 * sizeof(byte), 0, GL_STREAM_READ);

	qglGenBuffers(1, &pbo._fullScreenF);
	qglBindBuffer(GL_PIXEL_PACK_BUFFER, pbo._fullScreenF);
	qglBufferData(GL_PIXEL_PACK_BUFFER, vid.width * vid.height * 3 * sizeof(float), 0, GL_STREAM_READ);

	qglBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void R_InitFboBuffers() {

	int i;
	Com_Printf("Initializing Frame Buffers...\n\n");
// init fbo textures
	gi.hdrBase = R_CreateTexture("***hdrBase***", GL_TEXTURE_RECTANGLE, GL_RGBA16F, GL_RGBA,
		0, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		GL_FLOAT, NULL);

	gi.hdrBaseInterim = R_CreateTexture("***hdrBaseInterim***", GL_TEXTURE_RECTANGLE, GL_RGBA16F, GL_RGBA,
		0, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		GL_FLOAT, NULL);

	gi.rboDepth = R_CreateTexture("***rboDepth***", GL_TEXTURE_RECTANGLE, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT,
		0, vid.width, vid.height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST, GL_UNSIGNED_BYTE, NULL);

	gi.ldrBase = R_CreateTexture("***ldrBase***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB,
		0, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR,
		GL_FLOAT, NULL);

	gi.hdrInterim2D = R_CreateTexture("***hdrInterim2D***", GL_TEXTURE_2D, GL_RGBA16F, GL_RGBA,
		0, vid.width, vid.height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	gi.hdrLuminance = R_CreateTexture("***hdrLuminance***", GL_TEXTURE_2D, GL_RGB16F, GL_RGB,
		IF_MIPMAP, 128, 128, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_FLOAT, NULL);
	
	gi.lensFlareIn = R_CreateTexture("***lensFlareIn***", GL_TEXTURE_2D, GL_RGBA16F, GL_RGB, 0,
		vid.width * 0.25, vid.height * 0.25, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);
	gi.lensFlareInterim = R_CreateTexture("***lensFlareInterim***", GL_TEXTURE_2D, GL_R11F_G11F_B10F, GL_RGB, 0,
		vid.width * 0.25, vid.height * 0.25, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);
	gi.lensFlareOut = R_CreateTexture("***lensFlareOut***", GL_TEXTURE_2D, GL_R11F_G11F_B10F, GL_RGB, 0,
		vid.width * 0.25, vid.height * 0.25, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	gi.thermalIn = R_CreateTexture("***thermalIn***", GL_TEXTURE_RECTANGLE, GL_RGBA16F, GL_RGB, 0,
		vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
		GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);
	gi.thermalInterim = R_CreateTexture("***thermalInterim***", GL_TEXTURE_RECTANGLE, GL_R11F_G11F_B10F, GL_RGB, 0,
		vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
		GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);
	gi.thermalOut = R_CreateTexture("***thermalOut***", GL_TEXTURE_RECTANGLE, GL_R11F_G11F_B10F, GL_RGB, 0,
		vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
		GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	gi.bloomIn = R_CreateTexture("***bloomIn***", GL_TEXTURE_RECTANGLE, GL_RGBA16F, GL_RGBA, 0,
		vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	gi.bloomInterim = R_CreateTexture("***bloomInterim***", GL_TEXTURE_RECTANGLE, GL_R11F_G11F_B10F, GL_RGB, 0,
		vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	gi.bloomOut = R_CreateTexture("***bloomOut***", GL_TEXTURE_RECTANGLE, GL_R11F_G11F_B10F, GL_RGB, 0,
		vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	gi.linearDepth = R_CreateTexture("***linearDepth***", GL_TEXTURE_RECTANGLE, GL_R16F, GL_RED, 0,
		vid.width, vid.height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST, GL_FLOAT, NULL);

	gi.ssaoDepth = R_CreateTexture("***ssaoDepth***", GL_TEXTURE_RECTANGLE, GL_R16F, GL_RED, 0,
		vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
		GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);
	
	// 1024 512 256 128 64
	int shadowMapSize = 1024;
	for (i = 0; i < MAX_SHADOW_LODS; i++) {
		gi.shadowCube[i] = R_CreateTexture(va("***shadowCube[%i]***", i), GL_TEXTURE_CUBE_MAP, GL_R32F, GL_RED, 0, shadowMapSize, shadowMapSize,
			GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);
		shadowMapSize >>= 1;
	}

	for (i = 0; i < 2; i++)
		gi.ssaoColor[i] = R_CreateTexture("***ssaoColor***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB, 0,
			vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

// init fbo buffers
	Com_Printf("Load "S_COLOR_YELLOW "BASE FBO ");
	rb.rboDepth = R_Create_RBO("***rbo_depthBase***", GL_DEPTH_COMPONENT24, vid.width, vid.height);
	fb.hdrBase = R_Create_FBO("***hdrBase_fbo***");
	R_AttachRBO(rb.rboDepth, GL_DEPTH_ATTACHMENT);
	R_FB_AttachImage(GL_COLOR_ATTACHMENT0, gi.hdrBase, 0);
	R_FB_AttachImage(GL_COLOR_ATTACHMENT1, gi.hdrBaseInterim, 0);
	R_FB_AttachImage(GL_DEPTH_ATTACHMENT, gi.rboDepth, 0);
	R_FB_Check();

	Com_Printf("Load "S_COLOR_YELLOW "FINAL FBO ");
	fb.ldrBase = R_Create_FBO("***ldrBase_fbo***");
	R_FB_AttachImage(GL_COLOR_ATTACHMENT0, gi.ldrBase, 0);
	R_FB_Check();

	Com_Printf("Load "S_COLOR_YELLOW "BASE 2D FBO ");
	fb.hdrBase2D = R_Create_FBO("***hdrBase2D_fbo***");
	R_FB_AttachImage(GL_COLOR_ATTACHMENT0, gi.hdrInterim2D, 0);
	R_FB_Check();

	Com_Printf("Load "S_COLOR_YELLOW "HDR LUMINANCE FBO ");
	fb.hdrLum = R_Create_FBO("***hdrLum_fbo***");
	R_FB_AttachImage(GL_COLOR_ATTACHMENT0, gi.hdrLuminance, 0);
	R_FB_Check();


	Com_Printf("Load "S_COLOR_YELLOW "LENSFLARE FBO ");
	fb.lensFlare = R_Create_FBO("***lensflare_fbo***");
	R_FB_AttachImage(GL_COLOR_ATTACHMENT0, gi.lensFlareIn, 0);
	R_FB_AttachImage(GL_COLOR_ATTACHMENT1, gi.lensFlareInterim, 0);
	R_FB_AttachImage(GL_COLOR_ATTACHMENT2, gi.lensFlareOut, 0);
	R_FB_Check();

	Com_Printf("Load "S_COLOR_YELLOW "THERMAL FBO ");
	fb.thermal = R_Create_FBO("***thermal_fbo***");
	R_FB_AttachImage(GL_COLOR_ATTACHMENT0, gi.thermalIn, 0);
	R_FB_AttachImage(GL_COLOR_ATTACHMENT1, gi.thermalInterim, 0);
	R_FB_AttachImage(GL_COLOR_ATTACHMENT2, gi.thermalOut, 0);
	R_FB_Check();

	Com_Printf("Load "S_COLOR_YELLOW "BLOOM FBO ");
	fb.bloomCompute = R_Create_FBO("***comp_fbo***");
	R_FB_AttachImage(GL_COLOR_ATTACHMENT0, gi.bloomIn, 0);
	R_FB_AttachImage(GL_COLOR_ATTACHMENT1, gi.bloomInterim, 0);
	R_FB_AttachImage(GL_COLOR_ATTACHMENT2, gi.bloomOut, 0);
	R_FB_Check();

	Com_Printf("Load "S_COLOR_YELLOW "LINEAR DEPTH FBO ");
	fb.linearDepth = R_Create_FBO("***linearDepth_fbo***");
	R_FB_AttachImage(GL_COLOR_ATTACHMENT0, gi.linearDepth, 0);
	R_FB_Check();

	Com_Printf("Load "S_COLOR_YELLOW "SSAO FBO ");
	i_ssaoColorIndex = 0;
	fb.ssao = R_Create_FBO("***ssao_fbo***");
	R_FB_AttachImage(GL_COLOR_ATTACHMENT0, gi.ssaoColor[0], 0);
	R_FB_AttachImage(GL_COLOR_ATTACHMENT1, gi.ssaoColor[1], 0);
	R_FB_AttachImage(GL_COLOR_ATTACHMENT2, gi.ssaoDepth, 0);
	R_FB_Check();

	int rboSize = 1024;
	Com_Printf("Load "S_COLOR_YELLOW "SHADOWMAP FBO ");
	for (int l = 0; l < MAX_SHADOW_LODS; l++) {
		rb.depth[l] = R_Create_RBO(va("***rbo_depthShadowMap[%i]***", l), GL_DEPTH_COMPONENT24, rboSize, rboSize);
		rboSize >>= 1;
		fb.shadowMap[l] = R_Create_FBO(va("***shadowmap_fbo[%i]***", l));
		R_AttachRBO(rb.depth[l], GL_DEPTH_ATTACHMENT);
		for (i = 0; i < 6; i++)
			R_FB_AttachImage(GL_COLOR_ATTACHMENT0, gi.shadowCube[l], i);
	}
	R_FB_Check();

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

	Com_Printf("\n");
	R_InitPboBuffers();
	Com_Printf("\n");
}