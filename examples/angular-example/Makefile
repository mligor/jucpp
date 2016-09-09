include ../../setenv.mk

CC = g++
LD = g++
CXXFLAGS = $(EXTRA_CFLAGS) -Wall -std=gnu++11
LDFLAGS = -lpthread -ldl $(BUILD_DIR)libjucpp.a

OUT_NAME = $(BIN_DIR)angular-example

all: $(OUT_NAME)

$(BUILD_DIR)angular-example.obj: angular-example.cpp Makefile
	$(CC) $(CXXFLAGS) angular-example.cpp -c -o $@

$(OUT_NAME): $(BUILD_DIR)angular-example.obj
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $(BUILD_DIR)angular-example.obj -o $@

clean:
	rm -f $(OUT_NAME)

distclean:
	rm -f $(OUT_NAME)


