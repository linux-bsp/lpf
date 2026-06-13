# Configuration Directory Structure

ES-Middleware uses a product-based subdirectory structure for organizing configuration files.

## Directory Layout

```
configs/
├── tests/          # Test configurations
│   ├── tests_x86_full_defconfig
│   ├── tests_x86_minimal_defconfig
│   ├── tests_arm64_full_defconfig
│   └── ...
└── ccm/            # CCM product configurations
    ├── ccm_h200_100p_am625_debug_defconfig
    └── ccm_h200_100p_am625_release_defconfig
```

## Naming Convention

Configuration files follow this naming pattern:

```
<product>_<platform>_<variant>_defconfig
```

Where:
- **product**: Determines the subdirectory (tests, ccm, etc.)
- **platform**: Target platform (x86, arm64, am625, etc.)
- **variant**: Configuration variant (full, minimal, debug, release, etc.)

## How It Works

The build system automatically extracts the product name from the configuration filename and looks for the file in the corresponding subdirectory.

### Example

```bash
make tests_x86_full_defconfig
```

The build system:
1. Extracts `tests` from the configuration name (prefix before first `_`)
2. Looks for `configs/tests/tests_x86_full_defconfig`
3. Loads the configuration

## Adding New Configurations

### 1. Create Configuration File

Place the new configuration in the appropriate subdirectory:

```bash
# For test configurations
configs/tests/tests_<platform>_<variant>_defconfig

# For product configurations
configs/<product>/<product>_<platform>_<variant>_defconfig
```

### 2. Create New Product Category

To add a new product category:

```bash
# Create directory
mkdir configs/myproduct

# Add configuration
cp .config configs/myproduct/myproduct_x86_defconfig
```

The configuration will automatically be available:

```bash
make myproduct_x86_defconfig
```

## Listing Configurations

View all available configurations grouped by category:

```bash
make list
```

Output example:
```
Available defconfigs:

ccm configurations:
  ccm_h200_100p_am625_debug_defconfig
  ccm_h200_100p_am625_release_defconfig

tests configurations:
  tests_x86_full_defconfig
  tests_x86_minimal_defconfig
  tests_arm64_full_defconfig
  ...
```

## Benefits

1. **Organization**: Configurations are grouped by product/category
2. **Scalability**: Easy to add new product lines without cluttering
3. **Clarity**: Configuration purpose is clear from directory structure
4. **Automation**: Build system automatically finds configs based on naming

## Migration from Flat Structure

Old structure (deprecated):
```
configs/
├── tests_x86_full_defconfig
├── ccm_h200_100p_am625_debug_defconfig
└── ...
```

New structure:
```
configs/
├── tests/
│   └── tests_x86_full_defconfig
└── ccm/
    └── ccm_h200_100p_am625_debug_defconfig
```

Configuration loading remains the same:
```bash
make tests_x86_full_defconfig  # Works in both structures
```
