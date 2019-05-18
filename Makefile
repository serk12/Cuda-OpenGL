UNAME_S := $(shell uname)

ifeq ($(UNAME_S), Darwin)
    LDFLAGS = -Xlinker -framework,OpenGL -Xlinker -framework,GLUT
else
    LDFLAGS += -L/usr/local/cuda/samples/common/lib/linux/x86_64
    LDFLAGS += -lglut -lGL -lGLU -lGLEW
endif
LDFLAGS += -Wno-deprecated-gpu-targets
NVCC = /usr/local/cuda/bin/nvcc
NVCC_FLAGS = -g -G -Xcompiler "-Wall -Wno-deprecated-declarations" -Wno-deprecated-gpu-targets

all: ./build ./build/main.exe

./build/main.exe: ./build/main.o ./build/vertexKernel.o ./build/textureKernel.o
	$(NVCC) $^ -o $@ $(LDFLAGS)

./build/main.o: ./src/main.cpp ./header/vertexKernel.h ./header/textureKernel.h  ./header/interactions.h
	$(NVCC) $(NVCC_FLAGS) -c $< -o $@

./build/vertexKernel.o: ./src/vertexKernel.cu ./header/vertexKernel.h
	$(NVCC) $(NVCC_FLAGS) -c $< -o $@

./build/textureKernel.o: ./src/textureKernel.cu ./header/textureKernel.h
	$(NVCC) $(NVCC_FLAGS) -c $< -o $@

./build:
	mkdir ./build

clean:
	rm -rf ./build/*
