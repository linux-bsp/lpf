#!/bin/sh
# SPDX-License-Identifier: MIT

set -eu

module_dir=${LPF_MODULE_DIR:-_build/modules}
required_modules="osal lpf_core lpf_runtime lpf_hw_mock_selftest lpf_dummy_service_selftest"
loaded_modules=

log()
{
	printf '%s\n' "$*"
}

die()
{
	printf 'ERROR: %s\n' "$*" >&2
	exit 1
}

is_loaded()
{
	awk -v name="$1" '$1 == name { found = 1 } END { exit found ? 0 : 1 }' \
		/proc/modules
}

module_path()
{
	printf '%s/%s.ko\n' "$module_dir" "$1"
}

load_module()
{
	module=$1
	path=$(module_path "$module")

	log "  INSMOD  $path"
	insmod "$path"
	loaded_modules="$module $loaded_modules"
}

cleanup()
{
	status=$?

	for module in $loaded_modules; do
		if is_loaded "$module"; then
			log "  RMMOD   $module"
			rmmod "$module" || status=$?
		fi
	done

	exit "$status"
}

trap cleanup EXIT INT TERM

[ -d "$module_dir" ] || die "module directory not found: $module_dir"
module_dir=$(cd "$module_dir" && pwd)

for module in $required_modules; do
	[ -f "$(module_path "$module")" ] || die "missing module: $(module_path "$module")"
done

for module in $required_modules; do
	if is_loaded "$module"; then
		die "module is already loaded, refusing to own unload: $module"
	fi
done

if [ "$(id -u)" != "0" ]; then
	die "module smoke test requires root; run with sudo or as root"
fi

log "LPF mock module smoke test"
log "Module directory: $module_dir"

for module in $required_modules; do
	load_module "$module"
done

log "LPF mock module smoke test passed"
