include ../prepare.mk

.PHONY:all jucpp jucpp_mysql

CC = g++
C = gcc

CXXFLAGS = $(EXTRA_CFLAGS) -Wall -std=gnu++11 -I$(ROOT_DIR)libs/mysql/$(PLATFORM)-$(ARCH)/include -DMG_ENABLE_THREADS -Wno-deprecated-register
CFLAGS =  $(EXTRA_CFLAGS) -Wall -Wno-unused-const-variable -Wno-deprecated-register
LDFLAGS =

OUT_NAME = $(BUILD_DIR)libjucpp.a
OUT_NAME_MYSQL = $(BUILD_DIR)libjucpp-mysql.a


JUCPP_FILES := http.cpp jucpp.cpp sql.cpp session.cpp mail.cpp base64.cpp angular.cpp
JUCPP_MYSQL_FILES := $(JUCPP_FILES) jucpp_mysql.cpp
JUCPP_OBJ_FILES := $(addprefix $(BUILD_DIR), $(patsubst %.cpp, %.obj, $(JUCPP_FILES)))
JUCPP_OBJ_FILES_MYSQL := $(addprefix $(BUILD_DIR), $(patsubst %.cpp, %-mysql.obj, $(JUCPP_MYSQL_FILES)))

STANDARD_OBJ_FILES := $(BUILD_DIR)json_lib/json_writer.obj $(BUILD_DIR)json_lib/json_reader.obj $(BUILD_DIR)json_lib/json_value.obj $(BUILD_DIR)mongoose_lib.obj $(BUILD_DIR)sqlite_lib.obj

OBJ_FILES := $(JUCPP_OBJ_FILES) $(STANDARD_OBJ_FILES)

OBJ_FILES_MYSQL := $(JUCPP_OBJ_FILES_MYSQL) $(STANDARD_OBJ_FILES)

jucpp_mysql: $(OUT_NAME_MYSQL)

jucpp: $(OUT_NAME)

all: $(OUT_NAME) $(OUT_NAME_MYSQL)

$(OUT_NAME): $(JUCPP_OBJ_FILES) $(OBJ_FILES) Makefile
	mkdir -p $(dir $@)
	ar rsc $@ $(JUCPP_OBJ_FILES) $(OBJ_FILES)

$(OUT_NAME_MYSQL): $(JUCPP_OBJ_FILES_MYSQL) $(OBJ_FILES_MYSQL) Makefile
	mkdir -p $(dir $@)
	$(MAKELIB) $@ $(JUCPP_OBJ_FILES_MYSQL) $(OBJ_FILES_MYSQL)

$(BUILD_DIR)json_lib/json_writer.obj : ../libs/json/json_writer.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)json_lib/json_reader.obj : ../libs/json/json_reader.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)json_lib/json_value.obj : ../libs/json/json_value.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)sqlite_lib.obj : ../libs/sqlite/sqlite3.c
	mkdir -p $(dir $@)
	$(C) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)mongoose_lib.obj: ../libs/mongoose/mongoose.c
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)%-mysql.obj: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -DJUCPP_MYSQL -c -o $@ $<

$(BUILD_DIR)%.obj: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OUT_NAME) $(OBJ_FILES) $(OUT_NAME_MYSQL) $(OBJ_FILES_MYSQL)
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)

distclean:
	rm -f $(OUT_NAME) $(OBJ_FILES) $(OUT_NAME_MYSQL) $(OBJ_FILES_MYSQL)
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)

test:
	echo BIN_DIR=$(BUILD_DIR), BIN_DIR=$(BIN_DIR), mode=$(mode), ARCH=$(ARCH) 