#!/bin/bash
# Script to replace PDM symbols with PDM in source files
# This handles includes, function names, macros, and strings

set -e

echo "=== Step 2: Renaming symbols in source files ==="

# Function to replace in a single file
replace_in_file() {
    local file="$1"

    # Skip if file doesn't exist or is binary
    if [ ! -f "$file" ] || file "$file" | grep -q "binary"; then
        return
    fi

    # Use sed to replace all PDM/lpf occurrences with PDM/pdm
    sed -i \
        -e 's/#include "lpf\//#include "pdm\//g' \
        -e 's/#include <lpf\//#include <pdm\//g' \
        -e 's/\blpf_/pdm_/g' \
        -e 's/\bLPF_/PDM_/g' \
        -e 's/\/lpf\//\/pdm\//g' \
        -e 's/"pdm"/"pdm"/g' \
        -e 's/lpf\.ko/pdm.ko/g' \
        -e 's/pdm_core\.ko/pdm_core.ko/g' \
        -e 's/pdm_configs\.ko/pdm_configs.ko/g' \
        -e 's/pdm-core/pdm-core/g' \
        -e 's/pdm-configs/pdm-configs/g' \
        -e 's/\/dev\/lpf\//\/dev\/pdm\//g' \
        -e 's/\/proc\/lpf\//\/proc\/pdm\//g' \
        -e 's/\/sys\/kernel\/debug\/lpf\//\/sys\/kernel\/debug\/pdm\//g' \
        -e 's/CONFIG_PDM_/CONFIG_PDM_/g' \
        -e 's/\bLPF\b/PDM/g' \
        "$file"
}

# Process all C/H files
echo "Processing kernel source files..."
find kernel/ -type f \( -name "*.c" -o -name "*.h" \) | while read f; do
    echo "  $f"
    replace_in_file "$f"
done

# Process UAPI headers
echo "Processing UAPI headers..."
find uapi/ -type f -name "*.h" | while read f; do
    echo "  $f"
    replace_in_file "$f"
done

# Process userspace files
echo "Processing userspace files..."
find user/ -type f \( -name "*.c" -o -name "*.h" \) 2>/dev/null | while read f; do
    echo "  $f"
    replace_in_file "$f"
done || true

# Process Makefiles and Config files
echo "Processing build files..."
find . -type f \( -name "Makefile" -o -name "*.mk" -o -name "Config.in" -o -name "Kconfig" -o -name "CMakeLists.txt" \) | while read f; do
    echo "  $f"
    replace_in_file "$f"
done

# Process documentation
echo "Processing documentation..."
find docs/ -type f -name "*.md" | while read f; do
    echo "  $f"
    replace_in_file "$f"
done

# Process README
echo "Processing README..."
replace_in_file "README.md"
replace_in_file "CLAUDE.md" 2>/dev/null || true
replace_in_file "RENAME_PLAN.md"

# Process scripts
echo "Processing scripts..."
find scripts/ -type f -name "*.sh" | while read f; do
    echo "  $f"
    replace_in_file "$f"
done

echo ""
echo "=== Symbol replacement complete ==="
echo "Modified files:"
git status --short | wc -l
