/*
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
*/
#include <cutils/trace.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <time.h>
#include <utils/Timers.h>

#define IMG_WIDTH 4096
#define IMG_HEIGHT 4096

void testGpuMaxBw() {
	long img_size;
	int texid[2] = {201,202};
	int loop = 200;
	void* buf0 = malloc(img_size*4);
	void* buf1 = malloc(img_size*4);
	uint32_t* tmp_buf0 = (uint32_t*)buf0;
	uint32_t* tmp_buf1 = (uint32_t*)buf1;
	img_size = IMG_WIDTH*IMG_HEIGHT;

	//create texture array with upspecified image contentes
    glBindTexture(GL_TEXTURE_2D, texid[0]);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IMG_WIDTH, IMG_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, texid[1]);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IMG_WIDTH, IMG_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	
	srand((unsigned)time(NULL));	
	for (int i = 0; i < img_size; i++)
	{	
		memset(tmp_buf0+i, 2*i, 1);
		memset(tmp_buf1+i, 2*i+1, 1);
	}  
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IMG_WIDTH, IMG_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp_buf0);
	free(tmp_buf0); 
   
	glBindTexture(GL_TEXTURE_2D, 1);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IMG_WIDTH, IMG_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp_buf1);
	free(tmp_buf1);
   
    glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable(GL_TEXTURE_2D);
   
    glDisable(GL_BLEND);

    glViewport(0, 0, IMG_WIDTH, IMG_HEIGHT);

    glFinish();
    nsecs_t now = systemTime();
    for (int i=0 ; i<loop ; i++) {
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, 1);
        glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, texid[i%2], 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glColor4ub(i, 0, 0, 0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glFinish();
    nsecs_t t = systemTime() - now;
    printf("gpu max BW test: %dx%d:  time=%fms  BW=%f GB/s\n", IMG_WIDTH, IMG_HEIGHT, t/1000000.0f, img_size*4*2*loop/1024.0f/1024.0f/(t/1000000.0f));
	
}
int main(int argc, char** argv)
{
    EGLDisplay dpy;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLConfig config;
    EGLint numConfigs = -1;
    EGLSurface surface;
    EGLContext context;
    EGLint w = 320, h = 480;

    dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(dpy, &majorVersion, &minorVersion);

    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT|EGL_WINDOW_BIT,
        EGL_RED_SIZE,      5,
        EGL_GREEN_SIZE,    6,
        EGL_BLUE_SIZE,     5,
        EGL_NONE
    };
    eglChooseConfig(dpy, configAttribs, &config, 1, &numConfigs);
    EGLint surfaceAttribs[] = { EGL_WIDTH, w, EGL_HEIGHT, h, EGL_NONE };
    surface = eglCreatePbufferSurface(dpy, config, surfaceAttribs);
    context = eglCreateContext(dpy, config, NULL, NULL);
    eglMakeCurrent(dpy, surface, surface, context);
 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_BLEND);
    glDisable(GL_DITHER);

    const GLfloat vertices[4][2] = {
           { -1, -1 },
           { -1,  1 },
           {  1,  1 },
           {  1, -1 }
    };

    const GLfloat texCoords[4][2] = {
           { 0,  0 },
           { 1,  0 },
           { 1,  1 },
           { 0,  1 }
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

	testGpuMaxBw();
      
    eglTerminate(dpy);

    while(1);    
    return 0;

}
