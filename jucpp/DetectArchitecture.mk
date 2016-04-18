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

