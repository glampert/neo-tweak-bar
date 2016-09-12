
BIN_TARGET = ntb_test
CXX = clang++

SRC_FILES = neo_tweak_bar.cpp \
			ntb_geometry_batch.cpp \
			ntb_render_interface.cpp \
			ntb_widgets.cpp \
			test_main.cpp \
			test_glfw.cpp \
			gl3w/src/gl3w.cpp

OBJ_FILES = $(patsubst %.cpp, %.o, $(SRC_FILES))

# "Paranoid" set of warnings for Clang:
CXXFLAGS = \
	-std=c++11 \
	-fno-exceptions \
	-fno-rtti \
	-fstrict-aliasing \
	-pedantic \
	-Wall \
	-Wextra \
	-Weffc++ \
	-Winit-self \
	-Wformat=2 \
	-Wstrict-aliasing \
	-Wuninitialized \
	-Wunused \
	-Wswitch \
	-Wswitch-default \
	-Wpointer-arith \
	-Wwrite-strings \
	-Wmissing-braces \
	-Wparentheses \
	-Wsequence-point \
	-Wreturn-type \
	-Wunknown-pragmas \
	-Wshadow \
	-Wdisabled-optimization \
	-Wgcc-compat \
	-Wheader-guard \
	-Waddress-of-array-temporary \
	-Wglobal-constructors \
	-Wexit-time-destructors \
	-Woverloaded-virtual \
	-Wself-assign \
	-Wweak-vtables \
	-Wweak-template-vtables \
	-Wshorten-64-to-32 \
	-ferror-limit=5 \
	-fno-omit-frame-pointer \
	-g \
	-O0

# Clang static analyzer:
#CXXFLAGS += --analyze -Xanalyzer -analyzer-output=text

INC_DIRS  = -I. -Ivectormath -Igl3w/include
CXXFLAGS += $(INC_DIRS)

OPENGL_LIB = -framework CoreFoundation -framework OpenGL
GLFW_LIB   = -L/usr/local/lib -lglfw3
LIBRARIES  = $(OPENGL_LIB) $(GLFW_LIB)

#############################

all: $(BIN_TARGET)
	strip $(BIN_TARGET)

$(BIN_TARGET): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(OBJ_FILES) -o $(BIN_TARGET) $(LIBRARIES)

$(OBJ_FILES): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -I. -c $< -o $@

clean:
	rm -f $(BIN_TARGET)
	rm -f *.o

run: $(BIN_TARGET)
	strip $(BIN_TARGET)
	./$(BIN_TARGET)

