# A simple Makefile for compiling small SDL projects

# set the compiler flags
PACKAGES := sdl2 SDL2_image SDL2_mixer SDL2_ttf
CXXFLAGS := -ggdb -gdwarf -O0 -g -Wall -Wextra -std=gnu++20 -Wno-unused-parameter
CXXFLAGS += `pkg-config --cflags --static $(PACKAGES)` -DCLIENT
LDFLAGS := -ggdb -gdwarf -mconsole -static
LDFLAGS += `pkg-config --cflags --static --libs $(PACKAGES)` -lws2_32
# add header files here
HDRS :=
BUILDIR := build

# add source files here
SRCS := $(wildcard source/*.cpp) $(wildcard ../shared/*.cpp)

# generate names of object files
OBJS := $(addprefix $(BUILDIR)/, $(SRCS:.cpp=.o))
DEPS := $(OBJS:.o=.d)

# default recipe
all: a

# recipe for building the final executable
a: $(OBJS) $(HDRS) Makefile
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(BUILDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -c -o $@ $<

# recipe for building object files
#$(OBJS): $(@:.o=.c) $(HDRS) Makefile
#    $(CC) -o $@ $(@:.o=.c) -c $(CFLAGS)

# recipe to clean the workspace
clean:
	rm -f $(EXEC) $(DEPS) $(OBJS)

.PHONY: all clean

-include $(DEPS)