CC ?= clang

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
CFLAGS := -std=c18 -Weverything -pedantic-errors \
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
CFLAGS += -D_GNU_SOURCE
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
	@echo LINK $@
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(LINKERFLAGS) -o $@ $^

test: $(patsubst $(PATH_BUILD_OBJ)/%.o,$(PATH_BUILD_TESTS)/%,$(OBJS_TESTS))
	@$(foreach t,$^,\
	    echo -e "\nRunning $t"; \
	    ./$t;)

compile_commands.json: clean
	@compiledb --full-path -o $@ make all

$(PATH_BUILD_TESTS)/%_tests: $(PATH_BUILD_OBJ)/%_tests.o $(PATH_BUILD_OBJ)/unity.o \
    $(filter-out $(PATH_BUILD_OBJ)/main.o, $(OBJS_APP)) $(OBJS_DEPS)
	@echo CC $@
	@$(CC) -o $@ $^ $(CFLAGS) $(CPPFLAGS) $(LINKERFLAGS) 

$(PATH_BUILD_OBJ)/%.o:: $(PATH_SRC)/%.c | build_dirs
	@echo CC $@
	@$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS) 

# Unity build rule
$(PATH_BUILD_OBJ)/%.o:: $(PATH_DEPS)/unity/src/%.c | build_dirs
	@echo CC $@
	@$(CC) -c -w $< -o $@ $(CFLAGS) $(CPPFLAGS)

# Mpack build rule
$(PATH_BUILD_OBJ)/%.o:: $(PATH_DEPS)/mpack/%.c | build_dirs
	@echo CC $@
	@$(CC) -c -w $< -o $@ $(CFLAGS) $(CPPFLAGS)

# Dependency file gen rule
$(PATH_BUILD_DEPEND)/%.d: $(PATH_SRC)/%.c | build_dirs
	@echo CC $@
	@$(CC) $< -MM -MG -MF $@

-include $(patsubst $(PATH_BUILD_OBJ)/%.o, $(PATH_BUILD_DEPEND)/%.d, \
    $(OBJS_APP) \
    $(OBJS_DEPS) \
    $(OBJS_TESTS))

