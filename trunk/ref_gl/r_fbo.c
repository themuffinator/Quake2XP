/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/

// r_fbo.c

#include "r_local.h"

/*
=============
R_FB_Check

Framebuffer must be bound.
=============
*/
static void FB_Check(const char* file, const int line) {
	const char* s;
	GLenum		code;

	code = qglCheckFramebufferStatus(GL_FRAMEBUFFER);

	// an error occured
	switch (code) {
	case GL_FRAMEBUFFER_COMPLETE:
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

	Com_Printf("R_FB_Check: %s, line %i: %s\n", file, line, s);
}

#define _R_FB_Check();		FB_Check(__FILE__, __LINE__);

void CreateSSAOBuffer(void) {
	qboolean statusOK;

	Com_Printf("Load "S_COLOR_YELLOW "SSAO FBO ");

	r_miniDepthTex = R_CreateTexture("***r_miniDepthTex***", GL_TEXTURE_RECTANGLE, GL_R16F, GL_RED, 
									0, vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, 
									GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	for (int i = 0; i < 2; i++)
		r_ssaoColorTex[i] = R_CreateTexture("***r_ssaoColorTex***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB, 
									0, vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
											GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	r_ssaoColorTexIndex = 0;

	qglGenFramebuffers(1, &fbo._ssao);
	qglBindFramebuffer(GL_FRAMEBUFFER, fbo._ssao);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, r_ssaoColorTex[0]->texnum, 0);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, r_ssaoColorTex[1]->texnum, 0);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_RECTANGLE, r_miniDepthTex->texnum, 0);

	statusOK = qglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!statusOK)
		Com_Printf(S_COLOR_RED"Failed!");
	else
		Com_Printf(S_COLOR_WHITE"succeeded\n");
	
	qglObjectLabel(GL_FRAMEBUFFER, fbo._ssao, strlen("***fbo_ssao***"), "***fbo_ssao***");

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void CreateLinearDepthBuffer(void) {
	qboolean statusOK;

	Com_Printf("Load "S_COLOR_YELLOW "LINEAR DEPTH FBO ");

	r_linearDepth = R_CreateTexture("***r_linearDepth***", GL_TEXTURE_RECTANGLE, GL_R16F, GL_RED, 0, vid.width, vid.height,
									GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
									GL_FLOAT, NULL);

	qglGenFramebuffers(1, &fbo._linearDepth);
	qglBindFramebuffer(GL_FRAMEBUFFER, fbo._linearDepth);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, r_linearDepth->texnum, 0);

	statusOK = qglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!statusOK)
		Com_Printf(S_COLOR_RED"Failed!");
	else
		Com_Printf(S_COLOR_WHITE"succeeded\n");
	qglObjectLabel(GL_FRAMEBUFFER, fbo._linearDepth, strlen("***fbo_linearDepth***"), "***fbo_linearDepth***");

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void CreateBloomBuffer(void) {
	qboolean statusOK;

	Com_Printf("Load "S_COLOR_YELLOW "BLOOM FBO ");

	r_hdrBloomImage = R_CreateTexture("***r_hdrBloomImage***", GL_TEXTURE_RECTANGLE, GL_R11F_G11F_B10F, GL_RGB, 0,
	vid.width * 0.25, vid.height * 0.25, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	qglGenFramebuffers(1, &fbo._bloom);
	qglBindFramebuffer(GL_FRAMEBUFFER, fbo._bloom);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, r_hdrBloomImage->texnum, 0);


	statusOK = qglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!statusOK)
		Com_Printf(S_COLOR_RED"Failed!");
	else
		Com_Printf(S_COLOR_WHITE"succeeded\n");
	qglObjectLabel(GL_FRAMEBUFFER, fbo._bloom, strlen("***fbo_bloom***"), "***fbo_bloom***");

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void CreateThermalBuffer(void) {
	qboolean statusOK;

	Com_Printf("Load "S_COLOR_YELLOW "THERMAL FBO ");

	r_thermalImage = R_CreateTexture("***r_thermalImage***", GL_TEXTURE_RECTANGLE, GL_R11F_G11F_B10F, GL_RGB, 0,
									vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, 
									GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);
	
	qglGenFramebuffers(1, &fbo._thermal);
	qglBindFramebuffer(GL_FRAMEBUFFER, fbo._thermal);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, r_thermalImage->texnum, 0);

	statusOK = qglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!statusOK)
		Com_Printf(S_COLOR_RED"Failed!");
	else
		Com_Printf(S_COLOR_WHITE"succeeded\n");
	qglObjectLabel(GL_FRAMEBUFFER, fbo._thermal, strlen("***fbo_thermalVision***"), "***fbo_thermalVision***");
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void R_CreateScreenFbo() {
	uint rbId, rbcId;
	qboolean statusOK;

	Com_Printf("Load "S_COLOR_YELLOW "HDR FBO ");

//	qglGenRenderbuffers(1, &rbcId);
//	qglBindRenderbuffer(GL_RENDERBUFFER, rbcId);
//	qglRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB16F, vid.width, vid.height);

	qglGenRenderbuffers(1, &rbId);
	qglBindRenderbuffer(GL_RENDERBUFFER, rbId);
	qglRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, vid.width, vid.height);
//	qglRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, vid.width, vid.height);

	qglObjectLabel(GL_RENDERBUFFER, rbId, strlen("***rbo_HdrRenderBuffer***"), "***rbo_HdrRenderBuffer***");

	qglBindRenderbuffer(GL_RENDERBUFFER, 0);

	r_hdrScreen = R_CreateTexture("***r_hdrScreen***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB,
		0, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		GL_FLOAT, NULL);

	r_hdrScreenCopy = R_CreateTexture("***r_hdrScreenCopy***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB,
		0, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		GL_FLOAT, NULL);

	r_depthStencilTexture = R_CreateTexture("***r_depthStencilTexture***", GL_TEXTURE_RECTANGLE, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL,
		0, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		GL_UNSIGNED_INT_24_8, NULL);
	
	qglGenFramebuffers(1, &fbo._hdr);
	qglBindFramebuffer(GL_FRAMEBUFFER, fbo._hdr);

//	qglFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbcId);
//	qglFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH24_STENCIL8, GL_RENDERBUFFER, rbId);

	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE,			r_hdrScreen->texnum, 0);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE,			r_hdrScreenCopy->texnum, 0);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE,	r_depthStencilTexture->texnum, 0);

	statusOK = qglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!statusOK)
		Com_Printf(S_COLOR_RED"Failed!\n");
	else
		Com_Printf(S_COLOR_WHITE"succeeded\n");
	qglObjectLabel(GL_FRAMEBUFFER, fbo._hdr, strlen("***fbo_HdrScreen***"), "***fbo_HdrScreen***");
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

}
void R_FboFinal() {
	qboolean statusOK;

	Com_Printf("Load "S_COLOR_YELLOW "FINAL FBO ");

	qglGenFramebuffers(1, &fbo._final);
	qglBindFramebuffer(GL_FRAMEBUFFER, fbo._final);

	r_finalScreen = R_CreateTexture("***r_finalScreen***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB,
									0, vid.width, vid.height,
									GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR,
									GL_FLOAT, NULL);

	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, r_finalScreen->texnum, 0);

	statusOK = qglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!statusOK)
		Com_Printf(S_COLOR_RED"Failed!\n");
	else
		Com_Printf(S_COLOR_WHITE"succeeded\n");
	qglObjectLabel(GL_FRAMEBUFFER, fbo._final, strlen("***fbo_HdrFinal***"), "***fbo_HdrFinal***");
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void R_Tex2dFbo() {
	qboolean statusOK;

	Com_Printf("Load "S_COLOR_YELLOW "FXAA FBO ");

	qglGenFramebuffers(1, &fbo._tex2d);
	qglBindFramebuffer(GL_FRAMEBUFFER, fbo._tex2d);

	r_hdrScreenCopy2d = R_CreateTexture("***r_hdrScreenCopy2d***", GL_TEXTURE_2D, GL_RGB16F, GL_RGB,
						0, vid.width, vid.height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r_hdrScreenCopy2d->texnum, 0);

	statusOK = qglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!statusOK)
		Com_Printf(S_COLOR_RED"Failed!\n");
	else
		Com_Printf(S_COLOR_WHITE"succeeded\n");

	qglObjectLabel(GL_FRAMEBUFFER, fbo._tex2d, strlen("***fbo_HdrTexture2D***"), "***fbo_HdrTexture2D***");
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void R_HdrLumFbo() {
	qboolean statusOK;
	int i, texSize = 64;

	Com_Printf("Load "S_COLOR_YELLOW "HDR LUMINANCE FBO ");

	for (i = 0; i < 2; i++) {

		qglGenFramebuffers(1, &fbo._hdrLum[i]);
		qglBindFramebuffer(GL_FRAMEBUFFER, fbo._hdrLum[i]);

		r_hdrLuminance[i] = R_CreateTexture(va("***fbo_hdrLuminance_%i***", i), GL_TEXTURE_2D, GL_RGB16F, GL_RGB,
		IF_MIPMAP, texSize, texSize, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_FLOAT, NULL);

		qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r_hdrLuminance[i]->texnum, 0);
		qglObjectLabel(GL_FRAMEBUFFER, fbo._hdrLum[i], strlen(va("***fbo_hdrLuminance_%i***", i)), va("***fbo_hdrLuminance_%i***", i));
	}

	statusOK = qglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!statusOK)
		Com_Printf(S_COLOR_RED"Failed!\n");
	else
		Com_Printf(S_COLOR_WHITE"succeeded\n");

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void R_PboInit() {

	Com_Printf("Initializing Pixel Buffers: " S_COLOR_GREEN "ok\n");
	
	// read from gpu
	qglGenBuffers(2, pbo._luma);
	qglBindBuffer(GL_PIXEL_PACK_BUFFER, pbo._luma[0]);
	qglBufferData(GL_PIXEL_PACK_BUFFER, 64 * 64 * 3 * sizeof(float), 0, GL_STREAM_COPY);

	qglBindBuffer(GL_PIXEL_PACK_BUFFER, pbo._luma[1]);
	qglBufferData(GL_PIXEL_PACK_BUFFER, 64 * 64 * 3 * sizeof(float), 0, GL_STREAM_COPY);

	qglGenBuffers(1, &pbo._fullScreen);
	qglBindBuffer(GL_PIXEL_PACK_BUFFER, pbo._fullScreen);
	qglBufferData(GL_PIXEL_PACK_BUFFER, vid.width * vid.height * 3 * sizeof(byte), 0, GL_STREAM_READ);

	qglGenBuffers(1, &pbo._fullScreenF);
	qglBindBuffer(GL_PIXEL_PACK_BUFFER, pbo._fullScreenF);
	qglBufferData(GL_PIXEL_PACK_BUFFER, vid.width * vid.height * 3 * sizeof(float), 0, GL_STREAM_READ);


	qglBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}