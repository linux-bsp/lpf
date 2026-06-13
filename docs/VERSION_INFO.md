# Version Information System

ES-Middleware includes a comprehensive version information system similar to Linux kernel's version tracking. This provides complete traceability of the build environment and configuration.

## Overview

The version information system automatically generates `include/version.h` during configuration, containing:
- Software version and git commit
- Builder information (username and hostname)
- Compiler version and build timestamp
- Platform details (architecture and kernel version)

## Generated Macros

### Version Information
```c
ES_MIDDLEWARE_VERSION           // "1.0.0"
ES_MIDDLEWARE_VERSION_CODE      // Numeric timestamp
ES_MIDDLEWARE_VERSION_STRING    // "1.0.0-e9bc9d3"
ES_MIDDLEWARE_GIT_COMMIT        // "e9bc9d3" or "e9bc9d3-dirty"
```

### Build Information
```c
ES_MIDDLEWARE_COMPILE_BY        // Username who built it
ES_MIDDLEWARE_COMPILE_HOST      // Hostname of build machine
ES_MIDDLEWARE_COMPILER          // Full GCC version string
ES_MIDDLEWARE_COMPILE_TIME      // "2024-06-13 10:49:00 UTC"
ES_MIDDLEWARE_COMPILE_TIMESTAMP // Unix timestamp (numeric)
```

### Platform Information
```c
ES_MIDDLEWARE_BUILD_ARCH        // "x86_64", "arm", etc.
ES_MIDDLEWARE_BUILD_KERNEL      // "6.17.0-35-generic"
```

### Banner (Complete Version String)
```c
ES_MIDDLEWARE_BANNER
// "ES-Middleware version 1.0.0 (wanguo@cspd-Server) (gcc 13.3.0) 2024-06-13 10:49:00 UTC"
```

## Usage Examples

### Display Version Banner
```c
#include "version.h"
#include <stdio.h>

int main(void)
{
    printf("%s\n", ES_MIDDLEWARE_BANNER);
    return 0;
}
```

### Check Version at Runtime
```c
#include "version.h"
#include <string.h>

void check_version(void)
{
    if (strstr(ES_MIDDLEWARE_GIT_COMMIT, "dirty")) {
        printf("Warning: Built from modified source tree\n");
    }
    
    printf("Running version: %s\n", ES_MIDDLEWARE_VERSION_STRING);
}
```

### Add Version to Log Files
```c
#include "version.h"

void init_logging(void)
{
    log_info("Starting %s", ES_MIDDLEWARE_BANNER);
    log_info("Built by: %s@%s", 
             ES_MIDDLEWARE_COMPILE_BY, 
             ES_MIDDLEWARE_COMPILE_HOST);
    log_info("Compiler: %s", ES_MIDDLEWARE_COMPILER);
}
```

### Runtime Version Check
```c
#include "version.h"

#define REQUIRED_VERSION "1.0.0"

int check_version_compatibility(void)
{
    if (strcmp(ES_MIDDLEWARE_VERSION, REQUIRED_VERSION) != 0) {
        fprintf(stderr, "Version mismatch: expected %s, got %s\n",
                REQUIRED_VERSION, ES_MIDDLEWARE_VERSION);
        return -1;
    }
    return 0;
}
```

## Command-Line Tools

### Display Version Information
Create a `--version` flag in your application:

```c
#include "version.h"
#include <stdio.h>

void print_version(void)
{
    printf("%s\n\n", ES_MIDDLEWARE_BANNER);
    printf("Version:      %s\n", ES_MIDDLEWARE_VERSION);
    printf("Git commit:   %s\n", ES_MIDDLEWARE_GIT_COMMIT);
    printf("Built by:     %s@%s\n", 
           ES_MIDDLEWARE_COMPILE_BY, 
           ES_MIDDLEWARE_COMPILE_HOST);
    printf("Build time:   %s\n", ES_MIDDLEWARE_COMPILE_TIME);
    printf("Compiler:     %s\n", ES_MIDDLEWARE_COMPILER);
    printf("Architecture: %s\n", ES_MIDDLEWARE_BUILD_ARCH);
    printf("Kernel:       %s\n", ES_MIDDLEWARE_BUILD_KERNEL);
}
```

## Build System Integration

### When is version.h Generated?

`version.h` is automatically regenerated:
1. When running any `*config` target (e.g., `make menuconfig`, `make defconfig`)
2. When running `make all` (if `.config` exists)
3. Manually by running: `./scripts/gen_version.sh`

### Force Regeneration

```bash
# Regenerate version.h with current timestamp
./scripts/gen_version.sh

# Or clean and reconfigure
make distclean
make defconfig
```

### Version.h is Not Committed to Git

The `version.h` file is automatically generated and should never be committed to version control. It's already listed in `.gitignore`.

## Comparison with Linux Kernel

Similar to Linux kernel's version information:

**Linux Kernel:**
```
Linux version 5.15.0-86-generic (buildd@lcy02-amd64-079) 
(gcc version 11.3.0 (Ubuntu 11.3.0-1ubuntu1~22.04)) 
Wed Sep 27 14:23:11 UTC 2023
```

**ES-Middleware:**
```c
ES-Middleware version 1.0.0 (wanguo@cspd-Server) 
(gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0) 
2024-06-13 10:49:00 UTC
```

## Files

- `include/version.h` - Generated header file (auto-generated, not in git)
- `scripts/gen_version.sh` - Generator script
- `examples/version_info.c` - Complete usage example

## See Also

- `examples/version_info.c` - Full working example
- `include/autoconf.h` - Configuration macros from Kconfig
- Linux kernel documentation: `Documentation/kbuild/`
