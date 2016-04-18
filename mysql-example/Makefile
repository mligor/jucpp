CC = g++
CXXFLAGS = -v -g -O2 -Wall -std=gnu++11 -I$(CURDIR)/.. -I$(CURDIR)/../libs

include ../jucpp/DetectArchitecture.mk


LDFLAGS = -L$(CURDIR)/.. -L$(CURDIR)/../libs/mysql/$(PLATFORM)-$(ARCH)/lib  -ljucpp -lpthread -ldl -lmysqlclient -lrt

OUT_NAME = $(CURDIR)/mysql-example
BUILD_DIR = $(CURDIR)/../build/example

all: $(OUT_NAME)

$(OUT_NAME): mysql-example.cpp Makefile
	$(CC) $(CXXFLAGS) mysql-example.cpp $(LDFLAGS) -o $@

clean:
	rm -f $(OUT_NAME)

distclean:
	rm -f $(OUT_NAME)


