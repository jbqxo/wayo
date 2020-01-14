#  Copyright (c) 2019 Maxim Lyapin
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.

COMPILE := $(CC) -c
LINK := $(CC)
DEPEND := $(CC) -MM -MG -MF

PATH_DEPS := deps
PATH_SRC := src
PATH_BUILD := build
PATH_BUILD_OBJ := $(PATH_BUILD)/obj
PATH_BUILD_BIN := $(PATH_BUILD)/bin
PATH_BUILD_DEPEND := $(PATH_BUILD)/dep
PATH_BUILD_TESTS := $(PATH_BUILD)/tests
BUILD_PATHS := $(PATH_BUILD) $(PATH_BUILD_BIN) $(PATH_BUILD_OBJ) $(PATH_BUILD_TESTS)

SRC_TESTS := $(shell find $(PATH_SRC) -name '*_tests.c')
SRC_APP := $(filter-out $(SRC_TESTS), $(shell find $(PATH_SRC) -name '*.c'))

CPPFLAGS := -I$(PATH_SRC) -I$(PATH_DEPS)
CFLAGS := -std=c17 -Weverything -pedantic-errors \
    -Wno-c++98-compat

DEBUG_CFLAGS := -g
RELEASE_CFLAGS := -O2 -DNDEBUG
CFLAGS += $(if $(findstring 1,$(NDEBUG)),$(RELEASE_CFLAGS),$(DEBUG_CFLAGS))

DEBUG_CPPFLAGS :=
RELEASE_CPPFLAGS := -DNDEBUG
CPPFLAGS += $(if $(findstring 1,$(NDEBUG)),$(RELEASE_CPPFLAGS),$(DEBUG_CPPFLAGS))

OBJS_APP := $(patsubst $(PATH_SRC)/%.c, $(PATH_BUILD_OBJ)/%.o, $(SRC_APP))
OBJS_TESTS := $(patsubst $(PATH_SRC)/%.c, $(PATH_BUILD_OBJ)/%.o, $(SRC_TESTS))
OBJS_DEPS := \
    $(PATH_BUILD_OBJ)/mpack.o

# Dependencies
## Mpack (msgpack)
CPPFLAGS += -DMPACK_HAS_CONFIG

## libuv
CPPFLAGS += $(shell pkg-config --cflags libuv)
LINKERFLAGS += $(shell pkg-config --libs libuv)

.PHONY: all clean test build_dirs
all: $(PATH_BUILD)/wayo_server

clean:
	@echo Removing $(PATH_BUILD)/ dir
	@rm -rf $(PATH_BUILD)

build_dirs:
	@mkdir -p $(patsubst $(PATH_SRC)/%, $(PATH_BUILD_OBJ)/%, \
	    $(shell find $(PATH_SRC)/ -type d))
	@mkdir -p $(patsubst $(PATH_SRC)/%, $(PATH_BUILD_DEPEND)/%, \
	    $(shell find $(PATH_SRC)/ -type d))
	@mkdir -p $(patsubst $(PATH_SRC)/%, $(PATH_BUILD_TESTS)/%, \
	    $(shell find $(PATH_SRC)/ -type d))

$(PATH_BUILD)/wayo_server: $(OBJS_APP) $(OBJS_DEPS)
	@$(LINK) $(CFLAGS) $(CPPFLAGS) $(LINKERFLAGS) -o $@ $^

test: $(patsubst $(PATH_BUILD_OBJ)/%.o,$(PATH_BUILD_TESTS)/%,$(OBJS_TESTS))
	@$(foreach t,$^,\
	    echo -e "\nRunning $t"; \
	    ./$t;)

compile_commands.json: clean
	@compiledb --full-path -o $@ make all

#TODO(Maxim Lyapin): Fix memcheck
util_memcheck: wayo_tests
	valgrind --leak-check=yes --track-origins=yes ./wayo_tests

$(PATH_BUILD_TESTS)/%_tests: $(PATH_BUILD_OBJ)/%_tests.o $(PATH_BUILD_OBJ)/unity.o \
    $(filter-out $(PATH_BUILD_OBJ)/main.o, $(OBJS_APP)) $(OBJS_DEPS)
	@echo CC $@
	@$(LINK) $(CFLAGS) $(CPPFLAGS) $(LINKERFLAGS) -o $@ $^

$(PATH_BUILD_OBJ)/%.o:: $(PATH_SRC)/%.c | build_dirs
	@echo CC $@
	@$(COMPILE) $(CFLAGS) $(CPPFLAGS) $< -o $@

# Unity build rule
$(PATH_BUILD_OBJ)/%.o:: $(PATH_DEPS)/unity/src/%.c | build_dirs
	@echo CC $@
	@$(COMPILE) $(CFLAGS) $(CPPFLAGS) -w $< -o $@

# Mpack build rule
$(PATH_BUILD_OBJ)/%.o:: $(PATH_DEPS)/mpack/%.c | build_dirs
	@echo CC $@
	@$(COMPILE) $(CFLAGS) $(CPPFLAGS) -w $< -o $@

# Dependency file gen rule
$(PATH_BUILD_DEPEND)/%.d: $(PATH_SRC)/%.c | build_dirs
	@echo CC $@
	@$(DEPEND) $@ $<

-include $(patsubst $(PATH_BUILD_OBJ)/%.o, $(PATH_BUILD_DEPEND)/%.d, \
    $(OBJS_APP) \
    $(OBJS_DEPS) \
    $(OBJS_TESTS))

