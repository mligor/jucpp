CC = g++
CXXFLAGS = -g -O2 -Wall -std=c++11  -I$(CURDIR)/.. -I$(CURDIR)/../libs
LDFLAGS = 

OUT_NAME = $(CURDIR)/../libjucpp.a
BUILD_DIR = $(CURDIR)/../build

JUCPP_FILES := http.cpp jucpp.cpp
JUCPP_OBJ_FILES := $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp, %.obj, $(JUCPP_FILES)))

CPP_FILES := $(JUCPP_FILES) 

OBJ_FILES := $(JUCPP_OBJ_FILES) $(BUILD_DIR)/json_lib/json_writer.obj $(BUILD_DIR)/json_lib/json_reader.obj $(BUILD_DIR)/json_lib/json_value.obj $(BUILD_DIR)/mongoose_lib.obj $(BUILD_DIR)/tthread_lib.obj

all: $(OUT_NAME)

$(OUT_NAME): $(JUCPP_OBJ_FILES) $(OBJ_FILES)
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



$(BUILD_DIR)/mongoose_lib.obj: ../libs/mongoose/mongoose.c
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/tthread_lib.obj: ../libs/tthread/source/tinythread.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<
	
$(BUILD_DIR)/%.obj: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<
	
	

clean:
	rm -f $(OUT_NAME) $(OBJ_FILES)
	rm -rf $(BUILD_DIR)

distclean:
	rm -f $(OUT_NAME) $(OBJ_FILES)
	rm -rf $(BUILD_DIR)


