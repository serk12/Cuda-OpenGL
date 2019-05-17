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

./build/main.exe: ./build/main.o ./build/kernel.o
	$(NVCC) $^ -o $@ $(LDFLAGS)

./build/main.o: ./src/main.cpp ./header/kernel.h ./header/interactions.h
	$(NVCC) $(NVCC_FLAGS) -c $< -o $@

./build/kernel.o: ./src/kernel.cu ./header/kernel.h
	$(NVCC) $(NVCC_FLAGS) -c $< -o $@

./build:
	mkdir ./build

clean:
	rm -rf ./build/*
