#ifndef TEXTUREKERNEL_H
#define TEXTUREKERNEL_H

struct uchar4;
struct int2;
void textureKernelLauncher(uchar4 *d_out, int w, int h, int2 pos);

#endif // ifndef TEXTUREKERNEL_H
