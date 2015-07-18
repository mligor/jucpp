CC = g++
CXXFLAGS = -g -O2 -Wall -std=c++11 -stdlib=libc++ -I$(CURDIR)/.. -I$(CURDIR)/../libs
LDFLAGS = 

OUT_NAME = $(CURDIR)/../jucpp.a
BUILD_DIR = $(CURDIR)/../build

JUCPP_FILES := http.cpp jucpp.cpp
JUCPP_OBJ_FILES := $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp, %.obj, $(JUCPP_FILES)))

JSON_FILES := ../libs/json/json_reader.cpp ../libs/json/json_value.cpp ../libs/json/json_writer.cpp 
MONGOOSE_FILES := ../libs/mongoose/mongoose.c
TTHREAD_FILES := ../libs/tthread/source/tinythread.cpp

CPP_FILES := $(JUCPP_FILES) 

OBJ_FILES := $(JUCPP_OBJ_FILES) $(BUILD_DIR)/json_lib.obj $(BUILD_DIR)/mongoose_lib.obj $(BUILD_DIR)/tthread_lib.obj

all: $(OUT_NAME)

$(OUT_NAME): $(JUCPP_OBJ_FILES) $(OBJ_FILES)
	mkdir -p $(dir $@)
	ar rvs $@ $^

$(BUILD_DIR)/json_lib.obj: $(JSON_FILES)
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/mongoose_lib.obj: $(MONGOOSE_FILES)
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/tthread_lib.obj: $(TTHREAD_FILES)
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


