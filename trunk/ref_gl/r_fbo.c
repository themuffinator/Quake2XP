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

	r_miniDepthTex = R_CreateTexture("***r_miniDepthTex***", GL_TEXTURE_RECTANGLE, GL_R16F, GL_RED, it_pic, vid.width / 2, vid.height / 2, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, qfalse, NULL);

	for (int i = 0; i < 2; i++)
		r_ssaoColorTex[i] = R_CreateTexture("***r_ssaoColorTex***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB, it_pic, vid.width / 2, vid.height / 2, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, qfalse, NULL);

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

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void CreateBloomBuffer(void) {
	qboolean statusOK;

	Com_Printf("Load "S_COLOR_YELLOW "BLOOM FBO ");

	r_bloomImage = R_CreateTexture("***r_bloomImage***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB, it_pic, vid.width * 0.25, vid.height * 0.25, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_FLOAT, qfalse, NULL);

	qglGenFramebuffers(1, &fbo._bloom);
	qglBindFramebuffer(GL_FRAMEBUFFER, fbo._bloom);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, r_bloomImage->texnum, 0);

	statusOK = qglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!statusOK)
		Com_Printf(S_COLOR_RED"Failed!");
	else
		Com_Printf(S_COLOR_WHITE"succeeded\n");

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void CreateThermalBuffer(void) {
	qboolean statusOK;

	Com_Printf("Load "S_COLOR_YELLOW "THERMAL FBO ");

	r_thermalImage = R_CreateTexture("***r_thermalImage***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB, it_pic, 
									vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, 
									GL_LINEAR, GL_LINEAR, GL_FLOAT, qfalse, NULL);
	
	qglGenFramebuffers(1, &fbo._thermal);
	qglBindFramebuffer(GL_FRAMEBUFFER, fbo._thermal);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, r_thermalImage->texnum, 0);

	statusOK = qglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!statusOK)
		Com_Printf(S_COLOR_RED"Failed!");
	else
		Com_Printf(S_COLOR_WHITE"succeeded\n");

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void R_CreateScreenFbo() {
	uint rb;//, cb;
	qboolean statusOK;

	Com_Printf("Load "S_COLOR_YELLOW "HDR FBO ");

	//	qglGenRenderbuffers(1, &cb);
	//	qglBindRenderbuffer(GL_RENDERBUFFER, cb);
	//	qglRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB16F, vid.width, vid.height);

	qglGenRenderbuffers(1, &rb);
	qglBindRenderbuffer(GL_RENDERBUFFER, rb);
	qglRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, vid.width, vid.height);
	//	qglRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, vid.width, vid.height);
	qglBindRenderbuffer(GL_RENDERBUFFER, 0);

	r_hdrScreen = R_CreateTexture("***r_hdrScreen***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB,
		it_pic, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR,
		GL_FLOAT, qfalse, NULL);

	r_hdrScreenCopy = R_CreateTexture("***r_hdrScreenCopy***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB,
		it_pic, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR,
		GL_FLOAT, qfalse, NULL);

	r_hdrScreenCopy2d = R_CreateTexture("***r_hdrScreenCopy2d***", GL_TEXTURE_2D, GL_RGB16F, GL_RGB,
		it_pic, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR,
		GL_FLOAT, qtrue, NULL);

	r_fxaaTex = R_CreateTexture("***r_fxaaTex***", GL_TEXTURE_2D, GL_RGB16F, GL_RGB,
		it_pic, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
		GL_LINEAR, GL_LINEAR, GL_FLOAT, qfalse, NULL);

	r_fixFovTex = R_CreateTexture("***r_fixFovTex***", GL_TEXTURE_2D, GL_RGB16F, GL_RGB,
		it_pic, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
		GL_LINEAR, GL_LINEAR, GL_FLOAT, qfalse, NULL);


	r_depthStencilTexture = R_CreateTexture("***r_depthStencilTexture***", GL_TEXTURE_RECTANGLE, GL_DEPTH24_STENCIL8, GL_UNSIGNED_INT_24_8,
		it_pic, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		GL_UNSIGNED_INT_24_8, qfalse, NULL);


	qglGenFramebuffers(1, &fbo._hdr);
	qglBindFramebuffer(GL_FRAMEBUFFER, fbo._hdr);

	//	qglFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, cb);
	qglFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH24_STENCIL8, GL_RENDERBUFFER, rb);


	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, r_hdrScreen->texnum, 0);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, r_hdrScreenCopy->texnum, 0);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, r_hdrScreenCopy2d->texnum, 0);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, r_fxaaTex->texnum, 0);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, r_fixFovTex->texnum, 0);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE, r_depthStencilTexture->texnum, 0);

	statusOK = qglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!statusOK)
		Com_Printf(S_COLOR_RED"Failed!\n");
	else
		Com_Printf(S_COLOR_WHITE"succeeded\n");

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

}
void R_FboFinal() {
	qboolean statusOK;

	Com_Printf("Load "S_COLOR_YELLOW "FINAL FBO ");

	qglGenFramebuffers(1, &fbo._final);
	qglBindFramebuffer(GL_FRAMEBUFFER, fbo._final);

	r_finalScreen = R_CreateTexture("***r_finalScreen***", GL_TEXTURE_RECTANGLE, GL_RGB16F, GL_RGB,
		it_pic, vid.width, vid.height,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR,
		GL_FLOAT, qfalse, NULL);

	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, r_finalScreen->texnum, 0);

	statusOK = qglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!statusOK)
		Com_Printf(S_COLOR_RED"Failed!\n");
	else
		Com_Printf(S_COLOR_WHITE"succeeded\n");

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
}