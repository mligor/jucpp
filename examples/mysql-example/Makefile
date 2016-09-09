include ../../setenv.mk

CC = g++
LD = g++
CXXFLAGS = $(EXTRA_CFLAGS) -std=gnu++11
LDFLAGS = -lpthread -ldl $(BUILD_DIR)libjucpp-mysql.a $(ROOT_DIR)libs/mysql/$(PLATFORM)-$(ARCH)/lib/libmysqlclient.a

ifeq ($(PLATFORM),linux)
	LDFLAGS +=  -lrt
endif

OUT_NAME = $(BIN_DIR)mysql-example

all: $(OUT_NAME)

$(BUILD_DIR)mysql-example.obj: mysql-example.cpp Makefile
	$(CC) $(CXXFLAGS) mysql-example.cpp -c -o $@

$(OUT_NAME): $(BUILD_DIR)mysql-example.obj
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $(BUILD_DIR)mysql-example.obj -o $@

clean:
	rm -f $(OUT_NAME)

distclean:
	rm -f $(OUT_NAME)


