CC = g++
CXXFLAGS = -v -g -O2 -Wall -std=gnu++11 -I$(CURDIR)/.. -I$(CURDIR)/../libs
LDFLAGS = -L$(CURDIR)/.. -ljucpp-nomysql -lpthread -ldl

OUT_NAME = $(CURDIR)/angularExample
BUILD_DIR = $(CURDIR)/../build/angularExample

all: $(OUT_NAME)

$(OUT_NAME): main.cpp Makefile
	$(CC) $(CXXFLAGS) main.cpp $(LDFLAGS) -o $@

clean:
	rm -f $(OUT_NAME)

distclean:
	rm -f $(OUT_NAME)


