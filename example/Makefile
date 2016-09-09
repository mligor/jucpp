include ../prepare.mk

CC = g++
LD = g++
CXXFLAGS = $(EXTRA_CFLAGS) -Wall -std=gnu++11
LDFLAGS = -lpthread -ldl $(BUILD_DIR)libjucpp.a

OUT_NAME = $(BIN_DIR)example

all: $(OUT_NAME)

$(BUILD_DIR)example.obj: example.cpp Makefile
	$(CC) $(CXXFLAGS) example.cpp -c -o $@

$(OUT_NAME): $(BUILD_DIR)example.obj
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $(BUILD_DIR)example.obj -o $@

clean:
	rm -f $(OUT_NAME) $(OUT_NAME_DEBUG)

distclean:
	rm -f $(OUT_NAME) $(OUT_NAME_DEBUG)


