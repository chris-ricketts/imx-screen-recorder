CXX = g++
CPPFLAGS := -std=c++11
LDLIBS := -limxvpuapi -lipu 

OBJ_DIR=obj/
SRC_DIR=src/

SRC_FILES := $(wildcard $(SRC_DIR)*.cc)
INC_FILES := $(wildcard $(SRC_DIR)*.h)
OBJ_FILES := $(addprefix $(OBJ_DIR), $(notdir $(SRC_FILES:.cc=.o)))

all: dirs imx_screen_recorder

dirs:
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

.PHONY: dirs all clean

imx_screen_recorder: $(OBJ_FILES) $(DECODER)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	
obj/%.o: src/%.cc
	$(CXX) $(CPPFLAGS) -c -o $@ $^
