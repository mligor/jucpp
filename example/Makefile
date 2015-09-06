CC = g++
CXXFLAGS = -v -g -O2 -Wall -std=gnu++11 -I$(CURDIR)/.. -I$(CURDIR)/../libs
LDFLAGS = -L$(CURDIR)/.. -ljucpp -lpthread -ldl

OUT_NAME = $(CURDIR)/example
BUILD_DIR = $(CURDIR)/../build/example

all: $(OUT_NAME)

$(OUT_NAME): example.cpp
	$(CC) $(CXXFLAGS) example.cpp $(LDFLAGS) -o $@

clean:
	rm -f $(OUT_NAME)

distclean:
	rm -f $(OUT_NAME)


