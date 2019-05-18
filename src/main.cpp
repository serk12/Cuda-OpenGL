#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif // ifdef _WIN32

#ifdef __APPLE__
#include <GLUT/glut.h>
#else // ifdef __APPLE__
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif // ifdef __APPLE__

#include <cuda_runtime.h>
#include <cuda_gl_interop.h>

#include "../header/textureKernel.h"
#include "../header/vertexKernel.h"
#include "../header/interactions.h"

// texture and pixel objects
GLuint pbo = 0; // OpenGL pixel buffer object
GLuint tex = 0; // OpenGL texture object
GLuint vbo = 0; // OpenGL vertex buffer object

struct cudaGraphicsResource *cuda_pbo_resource;
struct cudaGraphicsResource *cuda_vbo_resource;

const unsigned int mesh_width  = 256;
const unsigned int mesh_height = 256;

int numVertices = mesh_width * mesh_height;
GLfloat xRotated, yRotated, zRotated;
float   g_fAnim = 0.0;
void initGLUT(int *argc, char **argv) {
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(600, 600);
    glutCreateWindow(TITLE_STRING);
#ifndef __APPLE__
    glewInit();
#endif // ifndef __APPLE__
    glClearColor(0, 0, 0, 0);
}

void initTextureBuffer() {
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 4 * W * H * sizeof(GLubyte), 0,
                 GL_STREAM_DRAW);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    cudaGraphicsGLRegisterBuffer(&cuda_pbo_resource, pbo,
                                 cudaGraphicsMapFlagsWriteDiscard);
}

void initVertexBuffer() {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, numVertices * 4 * sizeof(float), 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    cudaGraphicsGLRegisterBuffer(&cuda_vbo_resource, vbo, cudaGraphicsMapFlagsWriteDiscard);
}

void initBuffer() {
    initTextureBuffer();
    initVertexBuffer();
}

void renderTexture() {
    uchar4 *d_out = 0;
    cudaGraphicsMapResources(1, &cuda_pbo_resource, 0);
    cudaGraphicsResourceGetMappedPointer((void **)&d_out, NULL, cuda_pbo_resource);

    textureKernelLauncher(d_out, W, H, loc);

    cudaGraphicsUnmapResources(1, &cuda_pbo_resource, 0);
}

void renderVertex() {
    float4 *ptr = 0;
    cudaGraphicsMapResources(1, &cuda_vbo_resource, 0);
    size_t num_bytes;
    cudaGraphicsResourceGetMappedPointer((void **)&ptr, &num_bytes, cuda_vbo_resource);

    vertexKernelLauncher(ptr, mesh_width, mesh_height, g_fAnim);

    cudaGraphicsUnmapResources(1, &cuda_vbo_resource, 0);
}

void render() {
    renderTexture();
    renderVertex();
}

void drawTexture() {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-W / 100, -H / 100, -7);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-W / 100,  H / 100, -7);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(W / 100,  H / 100, -7);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(W / 100, -H / 100, -7);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void drawVbo() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexPointer(4, GL_FLOAT, 0, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glColor3f(1.0, 0.0, 0.0);
    glDrawArrays(GL_POINTS, 0, numVertices);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void draw() {
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.5);
    drawTexture();
    drawVbo();
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
    if (vbo) {
        cudaGraphicsUnregisterResource(cuda_vbo_resource);
        glDeleteBuffers(1, &vbo);
    }
}

void reshape(int x, int y) {
    if ((y == 0) || (x == 0)) return;  // Nothing is visible then, so return

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0, (GLdouble)x / (GLdouble)y, 0.5, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, x, y); // Use the whole window for rendering
}

void animation(void) {
    yRotated += 0.4;
    xRotated += 0.4;
    g_fAnim  += 0.01;
    display();
}

int main(int argc, char **argv) {
    printInstructions();

    // OpenGL init
    initGLUT(&argc, argv);

    // events init
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(handleSpecialKeypress);
    glutPassiveMotionFunc(mouseMove);
    glutMotionFunc(mouseDrag);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(animation);
    atexit(exitfunc);

    // Cuda init
    initBuffer();

    // main loop
    glutMainLoop();

    return 0;
}
