# Buildroot Integration

This directory contains a Buildroot package skeleton for building ES-Middleware as a generic embedded middleware framework.

## Defconfig

Use one of the framework defconfigs, for example:

```text
kernel_x86_modules_defconfig
```

## Install Modes

- Target install: libraries and enabled binaries are installed to the target root filesystem.
- Staging install: development headers and libraries are installed for packages that depend on ES-Middleware.

No product-specific init script is installed by default. Product services should be owned by the product layer that adds them.
