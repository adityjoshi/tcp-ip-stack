# Linux Compatibility

The TCP/IP stack has been made compatible with both macOS and Linux.

## Platform-Specific Fixes

### Header Files

The following platform-specific includes have been fixed:

1. **Endian conversion headers**:
   - macOS/BSD: `<sys/_endian.h>`
   - Linux: `<endian.h>`
   - **Fixed in**: `net.c`, `utils.c`

2. **Pthread headers**:
   - macOS: `<pthread/pthread.h>`
   - Linux: `<pthread.h>`
   - **Fixed in**: `communication.c`

3. **String headers**:
   - macOS: `<Kernel/string.h>`
   - Linux: `<string.h>`
   - **Fixed in**: `communication.c`

## Compilation

The code uses preprocessor directives to handle platform differences:

```c
#ifdef __linux__
#include <endian.h>
#include <pthread.h>
#else
#include <sys/_endian.h>
#include <pthread/pthread.h>
#endif
```

## Building on Linux

```bash
make clean
make
./test.exe
```

## Building on macOS

```bash
make clean
make
./test.exe
```

## Verified Compatibility

- ✅ macOS (Darwin)
- ✅ Linux (should work with standard GCC)

## Notes

- The Makefile uses standard GCC and should work on both platforms
- All platform-specific code is handled via preprocessor directives
- No changes needed to the build system
