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

GLfloat xRotated, yRotated, zRotated;

void initGLUT(int *argc, char **argv) {
        glutInit(argc, argv);
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
        glutInitWindowPosition(100, 100);
        glutInitWindowSize(600, 600);
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
        glColor3f(1.0f,1.0f,1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-W/100, -H/100,-7);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-W/100,  H/100,-7);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(W/100,  H/100, -7);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(W/100, -H/100, -7);
        glEnd();
        glDisable(GL_TEXTURE_2D);
}

void drawCube() {
        glRotatef(xRotated,1.0,0.0,0.0);
        glRotatef(yRotated,0.0,1.0,0.0);
        glRotatef(zRotated,0.0,0.0,1.0);

        glBegin(GL_QUADS);  // Draw The Cube Using quads
        glColor3f(1.0f,0.0f,0.0f);
        glVertex3f( 1.0f, 1.0f,-1.0f);  // Top Right Of The Quad (Top)
        glVertex3f(-1.0f, 1.0f,-1.0f);  // Top Left Of The Quad (Top)
        glVertex3f(-1.0f, 1.0f, 1.0f);  // Bottom Left Of The Quad (Top)
        glVertex3f( 1.0f, 1.0f, 1.0f);  // Bottom Right Of The Quad (Top)
        glColor3f(0.0f,1.0f,0.0f);
        glVertex3f( 1.0f,-1.0f, 1.0f);  // Top Right Of The Quad (Bottom)
        glVertex3f(-1.0f,-1.0f, 1.0f);  // Top Left Of The Quad (Bottom)
        glVertex3f(-1.0f,-1.0f,-1.0f);  // Bottom Left Of The Quad (Bottom)
        glVertex3f( 1.0f,-1.0f,-1.0f);  // Bottom Right Of The Quad (Bottom)
        glColor3f(0.0f,0.0f,0.5f);
        glVertex3f( 1.0f, 1.0f, 1.0f);  // Top Right Of The Quad (Front)
        glVertex3f(-1.0f, 1.0f, 1.0f);  // Top Left Of The Quad (Front)
        glVertex3f(-1.0f,-1.0f, 1.0f);  // Bottom Left Of The Quad (Front)
        glVertex3f( 1.0f,-1.0f, 1.0f);  // Bottom Right Of The Quad (Front)
        glColor3f(1.0f,1.0f,0.0f);
        glVertex3f( 1.0f,-1.0f,-1.0f);  // Top Right Of The Quad (Back)
        glVertex3f(-1.0f,-1.0f,-1.0f);  // Top Left Of The Quad (Back)
        glVertex3f(-1.0f, 1.0f,-1.0f);  // Bottom Left Of The Quad (Back)
        glVertex3f( 1.0f, 1.0f,-1.0f);  // Bottom Right Of The Quad (Back)
        glColor3f(0.0f,1.0f,1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);  // Top Right Of The Quad (Left)
        glVertex3f(-1.0f, 1.0f,-1.0f);  // Top Left Of The Quad (Left)
        glVertex3f(-1.0f,-1.0f,-1.0f);  // Bottom Left Of The Quad (Left)
        glVertex3f(-1.0f,-1.0f, 1.0f);  // Bottom Right Of The Quad (Left)
        glColor3f(1.0f,0.0f,1.0f);
        glVertex3f( 1.0f, 1.0f,-1.0f);  // Top Right Of The Quad (Right)
        glVertex3f( 1.0f, 1.0f, 1.0f);  // Top Left Of The Quad (Right)
        glVertex3f( 1.0f,-1.0f, 1.0f);  // Bottom Left Of The Quad (Right)
        glVertex3f( 1.0f,-1.0f,-1.0f);  // Bottom Right Of The Quad (Right)
        glEnd();        // End Drawing The Cube
}

void draw() {
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

void exitfunc() {
        if (pbo) {
                cudaGraphicsUnregisterResource(cuda_pbo_resource);
                glDeleteBuffers(1, &pbo);
                glDeleteTextures(1, &tex);
        }
}

void reshape(int x, int y) {
        if (y == 0 || x == 0) return;  //Nothing is visible then, so return
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(40.0,(GLdouble)x/(GLdouble)y,0.5,20.0);
        glMatrixMode(GL_MODELVIEW);
        glViewport(0,0,x,y); //Use the whole window for rendering
}

void animation(void) {
        yRotated += 0.4;
        xRotated += 0.4;
        display();
}

int main(int argc, char** argv) {
        printInstructions();

        //OpenGL init
        initGLUT(&argc, argv);

        //events init
        glutKeyboardFunc(keyboard);
        glutSpecialFunc(handleSpecialKeypress);
        glutPassiveMotionFunc(mouseMove);
        glutMotionFunc(mouseDrag);
        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glutIdleFunc(animation);
        atexit(exitfunc);

        //Cuda init
        initPixelBuffer();

        //main loop
        glutMainLoop();

        return 0;
}
