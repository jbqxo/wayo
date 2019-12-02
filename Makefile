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
#

DEBUG_CFLAGS := -g
RELEASE_CFLAGS := -O2 -DNDEBUG
CFLAGS := $(if $(findstring 1,$(NDEBUG)),$(RELEASE_CFLAGS),$(DEBUG_CFLAGS))
CPPFLAGS := -Isrc
LINKER :=

# Dependencies
## Mpack (msgpack)
CPPFLAGS := $(CPPFLAGS) -Ideps -DMPACK_HAS_CONFIG

## libuv
CFLAGS := $(CFLAGS) $(shell pkg-config --cflags libuv)
LINKER := $(LINKER) $(shell pkg-config --libs libuv)

DEPS_OBJS := \
  deps/mpack/mpack.o

OBJS := \
  src/main.o \
  src/utils/memory_pool.o \
  src/nvim/api.o \
  src/nvim/funcs.o \
  src/nvim/storage.o \
  $(DEPS_OBJS)

TESTS_OBJS := \
  src/test.o \
  deps/unity/src/unity.c \
  $(filter-out src/main.o, $(OBJS))


.PHONY: all clean test
all: neobolt_server neobolt_tests

test: neobolt_tests
	@./neobolt_tests

clean:
	@echo Removing all translation units
	@rm -f $(shell find . -name '*.o')
	@echo Removing all depfiles
	@rm -f $(shell find . -name '*.d')
	@echo Removing linked binaries
	@rm -f neobolt_server neobolt_tests

compile_commands.json: clean
	@compiledb --full-path -o $@ make all

neobolt_server: $(OBJS)
	@echo CC    $@
	@$(CC) -o $@ $(LINKER) $(CFLAGS) $^

util_memcheck: neobolt_tests
	valgrind --leak-check=yes --track-origins=yes ./neobolt_tests

neobolt_tests: $(TESTS_OBJS)
	@echo CC    $@
	@echo 
	@$(CC) -o $@ $(LINKER) $(CFLAGS) $^

%.o: %.c
	@echo CC    $@
	@$(CC) -std=gnu18 -MD -c -o $@ $(CFLAGS) $(CPPFLAGS) $<

-include $(OBJS:.o=.d)
