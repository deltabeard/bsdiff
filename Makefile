EXE := bstool
SRCS := bstool.c bsdiff.c bspatch.c

ifdef VSCMD_VER
	# Visual Studio is special.
	CC := cl
	OUTCMD := /Fe
	CFLAGS := /nologo /W4 /O2 /GL /D_CRT_SECURE_NO_WARNINGS
else
	OUTCMD := -o
	CFLAGS := -O2 -std=c99 -pedantic -Wall -Wextra
endif

all: $(EXE)
$(EXE): $(SRCS)
	$(CC) $(OUTCMD)$@ $^ $(CFLAGS)