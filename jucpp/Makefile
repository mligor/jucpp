include DetectArchitecture.mk

CC = g++
C = gcc

CXXFLAGS = -g -O2 -Wall -std=gnu++11  -I$(CURDIR)/.. -I$(CURDIR)/../libs -I$(CURDIR)/../libs/mysql/$(PLATFORM)-$(ARCH)/include -DMG_ENABLE_THREADS
CFLAGS = -g -O2 -Wall -I$(CURDIR)/.. -I$(CURDIR)/../libs
LDFLAGS = 

OUT_NAME = $(CURDIR)/../libjucpp.a
OUT_NAME_NO_MYSQL = $(CURDIR)/../libjucpp-nomysql.a
BUILD_DIR = $(CURDIR)/../build

JUCPP_FILES := http.cpp jucpp.cpp sql.cpp session.cpp mail.cpp base64.cpp angular.cpp jucpp_mysql.cpp
JUCPP_OBJ_FILES := $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp, %.obj, $(JUCPP_FILES)))
JUCPP_OBJ_FILES_NO_MYSQL := $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp, %-nomysql.obj, $(JUCPP_FILES)))

CPP_FILES := $(JUCPP_FILES) 

STANDARD_OBJ_FILES := $(BUILD_DIR)/json_lib/json_writer.obj $(BUILD_DIR)/json_lib/json_reader.obj $(BUILD_DIR)/json_lib/json_value.obj $(BUILD_DIR)/mongoose_lib.obj $(BUILD_DIR)/sqlite_lib.obj

OBJ_FILES := $(JUCPP_OBJ_FILES) $(STANDARD_OBJ_FILES)

OBJ_FILES_NO_MYSQL := $(JUCPP_OBJ_FILES_NO_MYSQL) $(STANDARD_OBJ_FILES)

all: $(OUT_NAME) $(OUT_NAME_NO_MYSQL)

$(OUT_NAME): $(JUCPP_OBJ_FILES) $(OBJ_FILES) Makefile
	mkdir -p $(dir $@)
	ar rcvs $@ $^

$(OUT_NAME_NO_MYSQL): $(JUCPP_OBJ_FILES_NO_MYSQL) $(OBJ_FILES_NO_MYSQL) Makefile
	mkdir -p $(dir $@)
	ar rcvs $@ $^

$(BUILD_DIR)/json_lib/json_writer.obj : ../libs/json/json_writer.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/json_lib/json_reader.obj : ../libs/json/json_reader.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/json_lib/json_value.obj : ../libs/json/json_value.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/sqlite_lib.obj : ../libs/sqlite/sqlite3.c
	mkdir -p $(dir $@)
	$(C) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/mongoose_lib.obj: ../libs/mongoose/mongoose.c
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%-nomysql.obj: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -DJUCPP_NO_MYSQL -c -o $@ $<

$(BUILD_DIR)/%.obj: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OUT_NAME) $(OBJ_FILES) $(OUT_NAME_NO_MYSQL) $(OBJ_FILES_NO_MYSQL)
	rm -rf $(BUILD_DIR)

distclean:
	rm -f $(OUT_NAME) $(OBJ_FILES) $(OUT_NAME_NO_MYSQL) $(OBJ_FILES_NO_MYSQL)
	rm -rf $(BUILD_DIR)


