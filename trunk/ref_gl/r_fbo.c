#include "r_local.h"


static GLenum drawbuffer[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};

void R_CheckFBO()
{
const char	*s;
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

	Com_Printf(S_COLOR_RED, "R_CheckFBO: %s\n", s);
}

void Create_FBO_Tex0(void){

	
	int		i;
	char	name[17] = "***fbo_color0***";
	image_t	*image;

	// find a free image_t
	for (i=0, image=gltextures ; i<numgltextures ; i++,image++)
	{
		if (!image->texnum)
			break;
	}
	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			VID_Error (ERR_FATAL, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];
	
	strcpy (image->name, name);

	image->width = vid.width;
	image->height = vid.height;
	image->upload_width = vid.width;
	image->upload_height = vid.height;
	image->type = it_pic;
	image->texnum = TEXNUM_IMAGES + (image - gltextures);

	gl_state.fbo_color0 = image;

    qglBindTexture			(GL_TEXTURE_RECTANGLE_ARB, gl_state.fbo_color0->texnum);
	qglTexImage2D		    (GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, vid.width, vid.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	qglFramebufferTexture2D	(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB, gl_state.fbo_color0->texnum, 0);
	qglDrawBuffers			(1, drawbuffer);
}

void Create_FBO(void){

	qglGenRenderbuffers				(1, &gl_state.rbo_depthStencil);
	qglBindRenderbuffer				(GL_RENDERBUFFER, gl_state.rbo_depthStencil);
	qglRenderbufferStorage			(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, vid.width, vid.height);
	qglBindRenderbuffer				(GL_RENDERBUFFER, 0);

	qglGenFramebuffers				(1, &gl_state.fbo_base);
	qglBindFramebuffer				(GL_FRAMEBUFFER, gl_state.fbo_base);
	// attach depth render buffer
	qglFramebufferRenderbuffer		(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gl_state.rbo_depthStencil);
	// attach stencil render buffer
	qglFramebufferRenderbuffer		(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gl_state.rbo_depthStencil);
	
	// attach color to FBO as Color0
	Create_FBO_Tex0					();
	R_CheckFBO						();
	qglBindFramebuffer				(GL_FRAMEBUFFER, 0);

}


void CreateWeaponRect(void){

	
	int		i;
	char	name[17] = "***weaponHack***";
	image_t	*image;

	// find a free image_t
	for (i=0, image=gltextures ; i<numgltextures ; i++,image++)
	{
		if (!image->texnum)
			break;
	}
	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			VID_Error (ERR_FATAL, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];
	
	strcpy (image->name, name);

	image->width = vid.width;
	image->height = vid.height;
	image->upload_width = vid.width;
	image->upload_height = vid.height;
	image->type = it_pic;
	image->texnum = TEXNUM_IMAGES + (image - gltextures);

	weaponHack = image;

    qglBindTexture			(GL_TEXTURE_RECTANGLE_ARB, weaponHack->texnum);
	qglTexImage2D		    (GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, vid.width, vid.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	qglGenFramebuffers		(1, &gl_state.fbo_weaponMask);
	qglBindFramebuffer		(GL_FRAMEBUFFER, gl_state.fbo_weaponMask);
	qglFramebufferTexture2D	(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB, weaponHack->texnum, 0);
	qglDrawBuffers			(1, drawbuffer);
	qglBindFramebuffer		(GL_FRAMEBUFFER, 0);
	R_CheckFBO();

}