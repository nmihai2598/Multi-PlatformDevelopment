CC = cl
L = link 

all: build

build: tema1.c
	$(CC) /MD tema1.c /link /out:so-cpp.exe

.IGNORE:
clean:
	del /Q /S /F *.obj so-cpp.exe
