
ROOT_DIR = $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))

JUCPP_INCLUDES = -I$(ROOT_DIR) -I$(ROOT_DIR)libs

ifeq ($(mode),release)
   EXTRA_CFLAGS = -O2 $(JUCPP_INCLUDES)
else
   mode = debug
   EXTRA_CFLAGS = -g -O0 -DDEBUG $(JUCPP_INCLUDES)
endif

MAKELIB = ar rcsv

ifeq ($(OS),Windows_NT)
    PLATFORM = windows
    ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
        ARCH = win32
    endif
    ifeq ($(PROCESSOR_ARCHITECTURE),x86)
        ARCH = x64
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        PLATFORM = linux
    endif
    ifeq ($(UNAME_S),Darwin)
        PLATFORM = osx
        MAKELIB = libtool -static -v -o
    endif
    UNAME_M := $(shell uname -m)
    ifeq ($(UNAME_M),x86_64)
        ARCH = x86_64
    endif
    ifneq ($(filter %86,$(UNAME_M)),)
        ARCH = x86
    endif
    ifneq ($(filter arm%,$(UNAME_M)),)
        ARCH = arm
    endif
endif


BUILD_DIR = $(ROOT_DIR)build/
BIN_DIR = $(ROOT_DIR)bin/

