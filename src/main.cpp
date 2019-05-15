#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

#include <cuda_runtime.h>
#include <cuda_gl_interop.h>

#include "../header/kernel.h"
#include "../header/interactions.h"

// texture and pixel objects
GLuint pbo = 0;     // OpenGL pixel buffer object
GLuint tex = 0;     // OpenGL texture object
struct cudaGraphicsResource *cuda_pbo_resource;

void render() {
        uchar4 *d_out = 0;
        cudaGraphicsMapResources(1, &cuda_pbo_resource, 0);
        cudaGraphicsResourceGetMappedPointer((void **)&d_out, NULL,
                                             cuda_pbo_resource);
        kernelLauncher(d_out, W, H, loc);
        cudaGraphicsUnmapResources(1, &cuda_pbo_resource, 0);
}

void drawTexture() {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, NULL);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 3);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(3, 3);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(3, 0);
        glEnd();
        glDisable(GL_TEXTURE_2D);
}

void drawCube() {
        // glRotatef(xRotated,1.0,0.0,0.0);
        // // rotation about Y axis
        // glRotatef(yRotated,0.0,1.0,0.0);
        // // rotation about Z axis
        // glRotatef(zRotated,0.0,0.0,1.0);
        glBegin(GL_QUADS);  // Draw The Cube Using quads
        glColor3f(0.0f,1.0f,0.0f);  // Color Blue
        glVertex3f( 1.0f, 1.0f,-1.0f);  // Top Right Of The Quad (Top)
        glVertex3f(-1.0f, 1.0f,-1.0f);  // Top Left Of The Quad (Top)
        glVertex3f(-1.0f, 1.0f, 1.0f);  // Bottom Left Of The Quad (Top)
        glVertex3f( 1.0f, 1.0f, 1.0f);  // Bottom Right Of The Quad (Top)
        glColor3f(1.0f,0.5f,0.0f);  // Color Orange
        glVertex3f( 1.0f,-1.0f, 1.0f);  // Top Right Of The Quad (Bottom)
        glVertex3f(-1.0f,-1.0f, 1.0f);  // Top Left Of The Quad (Bottom)
        glVertex3f(-1.0f,-1.0f,-1.0f);  // Bottom Left Of The Quad (Bottom)
        glVertex3f( 1.0f,-1.0f,-1.0f);  // Bottom Right Of The Quad (Bottom)
        glColor3f(1.0f,0.0f,0.0f);  // Color Red
        glVertex3f( 1.0f, 1.0f, 1.0f);  // Top Right Of The Quad (Front)
        glVertex3f(-1.0f, 1.0f, 1.0f);  // Top Left Of The Quad (Front)
        glVertex3f(-1.0f,-1.0f, 1.0f);  // Bottom Left Of The Quad (Front)
        glVertex3f( 1.0f,-1.0f, 1.0f);  // Bottom Right Of The Quad (Front)
        glColor3f(1.0f,1.0f,0.0f);  // Color Yellow
        glVertex3f( 1.0f,-1.0f,-1.0f);  // Top Right Of The Quad (Back)
        glVertex3f(-1.0f,-1.0f,-1.0f);  // Top Left Of The Quad (Back)
        glVertex3f(-1.0f, 1.0f,-1.0f);  // Bottom Left Of The Quad (Back)
        glVertex3f( 1.0f, 1.0f,-1.0f);  // Bottom Right Of The Quad (Back)
        glColor3f(0.0f,0.0f,1.0f);  // Color Blue
        glVertex3f(-1.0f, 1.0f, 1.0f);  // Top Right Of The Quad (Left)
        glVertex3f(-1.0f, 1.0f,-1.0f);  // Top Left Of The Quad (Left)
        glVertex3f(-1.0f,-1.0f,-1.0f);  // Bottom Left Of The Quad (Left)
        glVertex3f(-1.0f,-1.0f, 1.0f);  // Bottom Right Of The Quad (Left)
        glColor3f(1.0f,0.0f,1.0f);  // Color Violet
        glVertex3f( 1.0f, 1.0f,-1.0f);  // Top Right Of The Quad (Right)
        glVertex3f( 1.0f, 1.0f, 1.0f);  // Top Left Of The Quad (Right)
        glVertex3f( 1.0f,-1.0f, 1.0f);  // Bottom Left Of The Quad (Right)
        glVertex3f( 1.0f,-1.0f,-1.0f);  // Bottom Right Of The Quad (Right)
        glEnd();        // End Drawing The Cube
}

void draw() {
        glMatrixMode(GL_MODELVIEW);
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        glTranslatef(0.0,0.0,-10.5);
        drawTexture();
        drawCube();
}

void display() {
        render();
        draw();
        glutSwapBuffers();
}

void initGLUT(int *argc, char **argv) {
        glutInit(argc, argv);
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
        glutInitWindowPosition(100, 100);
        glutInitWindowSize(W, H);
        glutCreateWindow(TITLE_STRING);
#ifndef __APPLE__
        glewInit();
#endif
        glClearColor(0,0,0,0);
}

void initPixelBuffer() {
        glGenBuffers(1, &pbo);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, 4*W*H*sizeof(GLubyte), 0,
                     GL_STREAM_DRAW);
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        cudaGraphicsGLRegisterBuffer(&cuda_pbo_resource, pbo,
                                     cudaGraphicsMapFlagsWriteDiscard);
}

void exitfunc() {
        if (pbo) {
                cudaGraphicsUnregisterResource(cuda_pbo_resource);
                glDeleteBuffers(1, &pbo);
                glDeleteTextures(1, &tex);
        }
}

void reshape(int x, int y)
{
        if (y == 0 || x == 0) return;  //Nothing is visible then, so return
        //Set a new projection matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        //Angle of view:40 degrees
        //Near clipping plane distance: 0.5
        //Far clipping plane distance: 20.0

        gluPerspective(40.0,(GLdouble)x/(GLdouble)y,0.5,20.0);
        glMatrixMode(GL_MODELVIEW);
        glViewport(0,0,x,y); //Use the whole window for rendering
}

int main(int argc, char** argv) {
        printInstructions();
        initGLUT(&argc, argv);
        glutKeyboardFunc(keyboard);
        glutSpecialFunc(handleSpecialKeypress);
        glutPassiveMotionFunc(mouseMove);
        glutMotionFunc(mouseDrag);
        glutDisplayFunc(display);
        initPixelBuffer();

        glutReshapeFunc(reshape);

        glutMainLoop();
        atexit(exitfunc);
        return 0;
}
