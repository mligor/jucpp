CC = g++
CXXFLAGS = -g -O2 -Wall -std=c++11 -stdlib=libc++ -I$(CURDIR)/.. -I$(CURDIR)/../libs
LDFLAGS = 

OUT_NAME = $(CURDIR)/../hello_http
BUILD_DIR = $(CURDIR)/../build/hello_http

all: $(OUT_NAME)

$(OUT_NAME): main.cpp
	$(CC) $(CXXFLAGS) $(LDFLAGS) $< ../jucpp.a -o $@

clean:
	rm -f $(OUT_NAME)

distclean:
	rm -f $(OUT_NAME)


