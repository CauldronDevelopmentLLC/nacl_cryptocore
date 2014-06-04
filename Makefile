ifeq ($(NACL_SDK_ROOT),)
$(error NACL_SDK_ROOT not set.)
endif

NAME = cryptocore
ARCH = pnacl
BUILD = build
TARGETS = $(NAME).pexe

SRC = $(wildcard src/*.cpp) $(wildcard src/*.c)
OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(SRC))
OBJ := $(patsubst %.c,$(BUILD)/%.o,$(OBJ))

TC_PATH := $(NACL_SDK_ROOT)/toolchain/linux_pnacl
TC_PREFIX := $(ARCH)-
CC := $(TC_PATH)/bin/$(TC_PREFIX)clang
CXX := $(TC_PATH)/bin/$(TC_PREFIX)clang++
FINALIZE := $(TC_PATH)/bin/$(TC_PREFIX)finalize
LIBDIR := $(ARCH)

STRIP := $(TC_PATH)/bin/$(TC_PREFIX)strip
AR := $(TC_PATH)/bin/$(TC_PREFIX)ar
RANLIB := $(TC_PATH)/bin/$(TC_PREFIX)ranlib

CCACHE_EXISTS := $(shell ccache -V)
ifdef CCACHE_EXISTS
	CC := ccache $(CC)
	CXX := ccache $(CXX)
endif

CCFLAGS += -MMD -MP -MF $(BUILD)/dep/$(@F).d
CCFLAGS += -I$(NACL_SDK_ROOT)/include -I.
CCFLAGS += -Wall -Werror -O3
LIBS += ppapi ppapi_cpp ssl crypto # glibc-compat
LDFLAGS := -L$(NACL_SDK_ROOT)/lib/$(LIBDIR)/Release $(foreach i,$(LIBS),-l$i)

all: $(TARGETS)


$(NAME).bc: $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)

$(NAME).pexe: $(NAME).bc
	$(FINALIZE) $< -o $@

$(BUILD)/%.o: %.cpp
	mkdir -p $(shell dirname $@)
	$(CXX) -c -o $@ $< $(CCFLAGS)

$(BUILD)/%.o: %.c
	mkdir -p $(shell dirname $@)
	$(CC) -c -o $@ $< $(CCFLAGS)

tidy:
	rm -f $(shell find . -name \*~) core*

clean: tidy
	rm -rf build $(NAME).bc $(NAME).pexe

.PHONY: all clean tidy

# Include the dependency files
-include $(shell mkdir -p $(BUILD)/dep 2>/dev/null) $(wildcard $(BUILD)/dep/*)
