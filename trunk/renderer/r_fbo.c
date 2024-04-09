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

void GL_BindFB(fbObject_t *fb) {

	if (!fb) {
		if (gl_state.fboId) {
			qglBindFramebuffer(GL_FRAMEBUFFER, 0);
			gl_state.fboId = 0;
		}
		return;
	}

	if (gl_state.fboId != fb->id) {
		qglBindFramebuffer(GL_FRAMEBUFFER, fb->id);
		gl_state.fboId = fb->id;
	}
}

void R_ShotdownFBO(void) {
	rbObject_t *rb;
	fbObject_t *fb;
	int i;

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

	for (i = 0, rb = r_rbo; i < r_numRbos; i++, rb++) {
		qglDeleteRenderbuffers(1, &rb->id);
		memset(rb, 0, sizeof(*rb));
	}
	for (i = 0, fb = r_fbo; i < r_numFbos; i++, fb++) {
		qglDeleteFramebuffers(1, &fb->id);
		memset(fb, 0, sizeof(*fb));
	}
}

rbObject_t *R_Create_RBO(const char *name, GLuint internalFormat, int width, int height) {
	rbObject_t *rb;
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

	if (r_numRbos == MAX_RBOS)
		VID_Error(ERR_DROP, "R_Create_RBO: MAX_RBOS hit");

	for (i = 0, rb = r_rbo; i < r_numRbos; i++, rb++) {
		if (!rb->id)
			break;
	}

	if (i == r_numRbos) {
		if (r_numRbos == MAX_RBOS)
			VID_Error(ERR_DROP, "MAX_RBOS");
		r_numRbos++;
	}

	strcpy(rb->name, name);
	rb->width = width;
	rb->height = height;

	qglGenRenderbuffers(1, &rb->id);
	qglBindRenderbuffer(GL_RENDERBUFFER, rb->id);
	qglRenderbufferStorage(GL_RENDERBUFFER, internalFormat, rb->width, rb->height);
	qglObjectLabel(GL_RENDERBUFFER, rb->id, strlen(rb->name), rb->name);
	qglBindRenderbuffer(GL_RENDERBUFFER, 0);

	return rb;
}

static void R_AttachRBO(const rbObject_t *rb, const GLenum attachment) {
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

void R_AttachImage(const GLenum attachment, const image_t *image, const int index) {

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
		VID_Error(ERR_DROP, "R_AttachImage: invalid attachment point 0x%x\n", attachment);
	}
	qglFramebufferTexture2D(GL_FRAMEBUFFER, attachment, image->texType, image->texnum, 0);
}

fbObject_t *R_Create_FBO(const char *name) {
	fbObject_t *fb;
	int i;

	if (r_numRbos == MAX_FBOS)
		VID_Error(ERR_DROP, "R_Create_FBO: MAX_FBOS hit");

	for (i = 0, fb = r_fbo; i < r_numFbos; i++, fb++) {
		if (!fb->id)
			break;
	}

	if (i == r_numFbos) {
		if (r_numFbos == MAX_FBOS)
			VID_Error(ERR_DROP, "MAX_FBOS");
		r_numFbos++;
	}

	strcpy(fb->name, name);

	qglGenFramebuffers	(1, &fb->id);
	qglBindFramebuffer	(GL_FRAMEBUFFER, fb->id);
	qglObjectLabel		(GL_FRAMEBUFFER, fb->id, strlen(fb->name), fb->name);

	return fb;
}

void CreateSSAOBuffer(void) {

	Com_Printf("Load "S_COLOR_YELLOW "SSAO FBO ");

	r_miniDepthTex = R_CreateTexture("***r_miniDepthTex***", GL_TEXTURE_RECTANGLE, GL_R16F, GL_RED, 
									0, vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, 
									GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	for (int i = 0; i < 2; i++)
		r_ssaoColorTex[i] = R_CreateTexture("***r_ssaoColorTex***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB, 
									0, vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
											GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	r_ssaoColorTexIndex = 0;
	fbo._ssao = R_Create_FBO("***ssai_fbo***");
	R_AttachImage(GL_COLOR_ATTACHMENT0, r_ssaoColorTex[0],	0);
	R_AttachImage(GL_COLOR_ATTACHMENT1, r_ssaoColorTex[1],	0);
	R_AttachImage(GL_COLOR_ATTACHMENT2, r_miniDepthTex,		0);
	R_FB_Check();
}

void CreateLinearDepthBuffer(void) {

	Com_Printf("Load "S_COLOR_YELLOW "LINEAR DEPTH FBO ");

	r_linearDepth = R_CreateTexture("***r_linearDepth***", GL_TEXTURE_RECTANGLE, GL_R16F, GL_RED, 0, vid.width, vid.height,
									GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
									GL_FLOAT, NULL);
	
	fbo._linearDepth = R_Create_FBO("***linearDepth_fbo***");
	R_AttachImage(GL_COLOR_ATTACHMENT0, r_linearDepth, 0);
	R_FB_Check();
}

void CreateBloomBuffer(void) {
	
	Com_Printf("Load "S_COLOR_YELLOW "BLOOM FBO ");
	
	r_compIn = R_CreateTexture("***r_compIn***", GL_TEXTURE_RECTANGLE, GL_RGBA16F, GL_RGBA, 0,
		vid.width, vid.height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	r_compInterim = R_CreateTexture("***r_compInterim***", GL_TEXTURE_RECTANGLE, GL_R11F_G11F_B10F, GL_RGB, 0,
		vid.width, vid.height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	r_compOut = R_CreateTexture("***r_compOut***", GL_TEXTURE_RECTANGLE, GL_R11F_G11F_B10F, GL_RGB, 0,
		vid.width, vid.height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);
	
	fbo._comp = R_Create_FBO("***comp_fbo***");
	R_AttachImage(GL_COLOR_ATTACHMENT0, r_compIn, 0);
	R_AttachImage(GL_COLOR_ATTACHMENT1, r_compInterim, 0);
	R_AttachImage(GL_COLOR_ATTACHMENT2, r_compOut, 0);
	R_FB_Check();
}

void CreateGlareBuffer(void) {

	Com_Printf("Load "S_COLOR_YELLOW "GLARE FBO ");

	r_hdrGlareImage = R_CreateTexture("***r_hdrGlareImage***", GL_TEXTURE_RECTANGLE, GL_R11F_G11F_B10F, GL_RGB, 0,
		vid.width * 0.25, vid.height * 0.25, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	fbo._glare = R_Create_FBO("***glare_fbo***");
	R_AttachImage(GL_COLOR_ATTACHMENT0, r_hdrGlareImage, 0);
	R_FB_Check();
}


void CreateThermalBuffer(void) {

	Com_Printf("Load "S_COLOR_YELLOW "THERMAL FBO ");

	r_thermalImage = R_CreateTexture("***r_thermalImage***", GL_TEXTURE_RECTANGLE, GL_R11F_G11F_B10F, GL_RGB, 0,
									vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, 
									GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);
	fbo._thermal = R_Create_FBO("***thermal_fbo***");
	R_AttachImage(GL_COLOR_ATTACHMENT0, r_thermalImage, 0);
	R_FB_Check();
}

void R_CreateScreenFbo() {

	Com_Printf("Load "S_COLOR_YELLOW "SCREEN FBO ");

	r_hdrScreen = R_CreateTexture("***r_hdrScreen***", GL_TEXTURE_RECTANGLE, GL_RGBA16F, GL_RGBA,
		0, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		GL_FLOAT, NULL);

	r_hdrScreenCopy = R_CreateTexture("***r_hdrScreenCopy***", GL_TEXTURE_RECTANGLE, GL_RGBA16F, GL_RGBA,
		0, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		GL_FLOAT, NULL);

	r_depthStencilTexture = R_CreateTexture("***r_depthStencilTexture***", GL_TEXTURE_RECTANGLE, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL,
		0, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		GL_UNSIGNED_INT_24_8, NULL);

	rbo.depthStencil	= R_Create_RBO("***rbo_depth_stencil***", GL_DEPTH24_STENCIL8, vid.width, vid.height);
	fbo._screen			= R_Create_FBO("***screen_fbo***");

	R_AttachRBO(rbo.depthStencil, GL_DEPTH_STENCIL_ATTACHMENT);

	R_AttachImage(GL_COLOR_ATTACHMENT0,			r_hdrScreen, 0);
	R_AttachImage(GL_COLOR_ATTACHMENT1,			r_hdrScreenCopy, 0);
	R_AttachImage(GL_DEPTH_STENCIL_ATTACHMENT,	r_depthStencilTexture, 0);
	R_FB_Check();
}

void R_FboFinal() {

	Com_Printf("Load "S_COLOR_YELLOW "FINAL FBO ");

	r_finalScreen = R_CreateTexture("***r_finalScreen***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB,
									0, vid.width, vid.height,
									GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR,
									GL_FLOAT, NULL);

	fbo._final = R_Create_FBO("***final_fbo***");
	R_AttachImage(GL_COLOR_ATTACHMENT0, r_finalScreen, 0);
	R_FB_Check();
}

void R_Tex2dFbo() {

	Com_Printf("Load "S_COLOR_YELLOW "SCREEN 2D FBO ");

	r_hdrScreenCopy2d = R_CreateTexture("***r_hdrScreenCopy2d***", GL_TEXTURE_2D, GL_RGBA16F, GL_RGBA,
						0, vid.width, vid.height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	fbo._tex2d = R_Create_FBO("***tex2d_fbo***");
	R_AttachImage(GL_COLOR_ATTACHMENT0, r_hdrScreenCopy2d, 0);
	R_FB_Check();
}

void R_HdrLumFbo() {
	int texSize = 128;

	Com_Printf("Load "S_COLOR_YELLOW "HDR LUMINANCE FBO ");

	r_hdrLuminance = R_CreateTexture("***fbo_hdrLuminance***", GL_TEXTURE_2D, GL_RGB16F, GL_RGB,
	IF_MIPMAP, texSize, texSize, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_FLOAT, NULL);
	
	fbo._hdrLum = R_Create_FBO("***hdrLum_fbo***");
	R_AttachImage(GL_COLOR_ATTACHMENT0, r_hdrLuminance, 0);
	R_FB_Check();
}

void R_PboInit() {

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

	Com_Printf("Initializing FBOs...\n\n");
	R_CreateScreenFbo();
	R_FboFinal();
	R_Tex2dFbo();
	CreateLinearDepthBuffer();
	CreateSSAOBuffer();
	CreateBloomBuffer();
	CreateGlareBuffer();
	CreateThermalBuffer();
	R_HdrLumFbo();
	qglBindFramebuffer(GL_FRAMEBUFFER, 0); // reset

	Com_Printf("\n");
	R_PboInit();
	Com_Printf("\n");
}