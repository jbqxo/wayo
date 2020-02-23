#pragma once

#define MPACK_EXTENSIONS 1

// Disable the usage of general purpose memory allocators for mpack
#undef MPACK_MALLOC
#undef MPACK_FREE
#undef MPACK_REALLOC

// MPACK_STDIO requires general purpose memory allocator
#undef MPACK_STDIO
