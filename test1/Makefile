CC = g++
CXXFLAGS = -v -g -O2 -Wall -std=c++11 -I$(CURDIR)/.. -I$(CURDIR)/../libs
LDFLAGS = -L$(CURDIR)/.. -ljucpp -lpthread -ldl

OUT_NAME = $(CURDIR)/../hello_http
BUILD_DIR = $(CURDIR)/../build/hello_http

all: $(OUT_NAME)

$(OUT_NAME): main.cpp
	$(CC) $(CXXFLAGS) main.cpp $(LDFLAGS) -o $@

clean:
	rm -f $(OUT_NAME)

distclean:
	rm -f $(OUT_NAME)


