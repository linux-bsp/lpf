// SPDX-License-Identifier: MIT
/*
 * PDI hardware debug tool.
 *
 * This is intentionally structured like a small BusyBox-style dispatcher:
 * modules and module actions are registered in static command tables, and each
 * command is implemented by a focused function that only uses the public PDI API.
 */

#include "pdi/pdi.h"

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PDEBUG_ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define PDEBUG_DEVICE_PATH_LEN 64U
#define PDEBUG_MCU_PATH_FORMAT "/dev/pdm/mcu%u"
#define PDEBUG_LED_PATH_FORMAT "/dev/pdm/led%u"

struct pdebug_cmd {
    const char *name;
    const char *summary;
    int (*run)(int argc, char **argv);
};

struct pdebug_selector {
    const char *path;
    const char *name;
    char generated_path[PDEBUG_DEVICE_PATH_LEN];
};

static void pdebug_print_errno(const char *op)
{
    fprintf(stderr, "%s failed: %s (%d)\n", op, strerror(errno), errno);
}

static bool pdebug_has_prefix(const char *str, const char *prefix)
{
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

static int pdebug_parse_u32(const char *arg, uint32_t *value)
{
    char *endptr = NULL;
    unsigned long parsed;

    if (!arg || !value) {
        errno = EINVAL;
        return -1;
    }

    errno = 0;
    parsed = strtoul(arg, &endptr, 0);
    if (errno || endptr == arg || *endptr != '\0' || parsed > UINT32_MAX) {
        errno = EINVAL;
        return -1;
    }

    *value = (uint32_t)parsed;
    return 0;
}

static int pdebug_parse_selector(const char *arg, const char *type,
                                 const char *path_format,
                                 struct pdebug_selector *selector)
{
    uint32_t index;
    int ret;

    memset(selector, 0, sizeof(*selector));
    if (!arg) {
        return 0;
    }

    if (pdebug_has_prefix(arg, "path:")) {
        selector->path = arg + strlen("path:");
        if (selector->path[0] == '\0') {
            errno = EINVAL;
            pdebug_print_errno("parse device path selector");
            return -1;
        }
        return 0;
    }

    if (pdebug_has_prefix(arg, "name:")) {
        selector->name = arg + strlen("name:");
        if (selector->name[0] == '\0') {
            errno = EINVAL;
            pdebug_print_errno("parse device name selector");
            return -1;
        }
        return 0;
    }

    if (arg[0] == '/') {
        selector->path = arg;
        return 0;
    }

    if (pdebug_parse_u32(arg, &index) == 0) {
        ret = snprintf(selector->generated_path,
                       sizeof(selector->generated_path), path_format, index);
        if (ret < 0 || (size_t)ret >= sizeof(selector->generated_path)) {
            errno = ENAMETOOLONG;
            pdebug_print_errno("format device path");
            return -1;
        }
        selector->path = selector->generated_path;
        return 0;
    }

    selector->name = arg;
    (void)type;
    return 0;
}

static const char *pdebug_selector_label(const struct pdebug_selector *selector,
                                         const char *default_label)
{
    if (selector->name) {
        return selector->name;
    }
    if (selector->path) {
        return selector->path;
    }
    return default_label;
}

static int pdebug_mcu_open_selected(pdi_mcu_context_t *ctx,
                                    const struct pdebug_selector *selector)
{
    if (selector->name) {
        return pdi_mcu_open_by_name(ctx, selector->name);
    }
    return pdi_mcu_open(ctx, selector->path);
}

static int pdebug_led_open_selected(pdi_led_context_t *ctx,
                                    const struct pdebug_selector *selector)
{
    if (selector->name) {
        return pdi_led_open_by_name(ctx, selector->name);
    }
    return pdi_led_open(ctx, selector->path);
}

static const char *pdebug_device_type_name(uint32_t type)
{
    switch (type) {
    case PDM_MANAGER_DEVICE_TYPE_MCU:
        return "mcu";
    case PDM_MANAGER_DEVICE_TYPE_LED:
        return "led";
    default:
        return "invalid";
    }
}

static const char *pdebug_device_state_name(uint32_t state)
{
    switch (state) {
    case PDM_MANAGER_DEVICE_STATE_REGISTERED:
        return "registered";
    case PDM_MANAGER_DEVICE_STATE_BOUND:
        return "bound";
    case PDM_MANAGER_DEVICE_STATE_ERROR:
        return "error";
    default:
        return "unknown";
    }
}

static const char *pdebug_owner_name(uint32_t owner)
{
    switch (owner) {
    case PDM_MANAGER_DEVICE_OWNER_KERNEL:
        return "kernel";
    case PDM_MANAGER_DEVICE_OWNER_USER:
        return "user";
    default:
        return "unspecified";
    }
}

static const char *pdebug_transport_name(uint32_t transport)
{
    switch (transport) {
    case PDM_MANAGER_TRANSPORT_CAN:
        return "can";
    case PDM_MANAGER_TRANSPORT_UART:
        return "uart";
    case PDM_MANAGER_TRANSPORT_I2C:
        return "i2c";
    case PDM_MANAGER_TRANSPORT_SPI:
        return "spi";
    default:
        return "none";
    }
}

static const char *pdebug_mcu_state_name(uint32_t state)
{
    switch (state) {
    case PDM_MCU_STATE_UNINITIALIZED:
        return "uninitialized";
    case PDM_MCU_STATE_INIT:
        return "init";
    case PDM_MCU_STATE_READY:
        return "ready";
    case PDM_MCU_STATE_BUSY:
        return "busy";
    case PDM_MCU_STATE_ERROR:
        return "error";
    case PDM_MCU_STATE_OFFLINE:
        return "offline";
    default:
        return "unknown";
    }
}

static void pdebug_print_temperature(int32_t milli_celsius)
{
    int64_t value = milli_celsius;

    if (value < 0) {
        putchar('-');
        value = -value;
    }

    printf("%" PRId64 ".%03" PRId64 "C", value / 1000, value % 1000);
}

static void pdebug_print_device(const struct pdm_manager_device_info *dev)
{
    printf("%-24s type=%-4s index=%u state=%s owner=%s transport=%s caps=0x%016" PRIx64 "\n",
           dev->name,
           pdebug_device_type_name(dev->type),
           dev->index,
           pdebug_device_state_name(dev->state),
           pdebug_owner_name(dev->owner),
           pdebug_transport_name(dev->transport),
           (uint64_t)dev->capabilities);
    if (dev->driver_name[0]) {
        printf("  driver:      %s\n", dev->driver_name);
    }
    if (dev->of_node_path[0]) {
        printf("  of-node:     %s\n", dev->of_node_path);
    }
    if (dev->controller_path[0]) {
        printf("  controller:  %s\n", dev->controller_path);
    }
}

static int pdebug_ctl_open(pdi_ctl_context_t *ctl)
{
    if (pdi_ctl_open(ctl, NULL) < 0) {
        pdebug_print_errno("open /dev/pdm_manager");
        return -1;
    }
    return 0;
}

static int pdebug_discovery_info(int argc, char **argv)
{
    pdi_ctl_context_t ctl = { .fd = -1 };
    struct pdm_manager_info info;
    (void)argc;
    (void)argv;

    if (pdebug_ctl_open(&ctl) < 0) {
        return 1;
    }
    if (pdi_ctl_get_info(&ctl, &info) < 0) {
        pdebug_print_errno("get manager info");
        pdi_ctl_close(&ctl);
        return 1;
    }
    pdi_ctl_close(&ctl);

    printf("manager ABI: 0x%08x\n", info.abi_version);
    printf("module:      %u.%u.%u\n", info.module_version_major,
           info.module_version_minor, info.module_version_patch);
    printf("open-count:  %u\n", info.open_count);
    printf("devices:     %u\n", info.device_count);
    return 0;
}

static int pdebug_discovery_list(int argc, char **argv)
{
    pdi_ctl_context_t ctl = { .fd = -1 };
    struct pdm_manager_info info;
    struct pdm_manager_device_info *devices;
    uint32_t count;
    uint32_t i;
    (void)argc;
    (void)argv;

    if (pdebug_ctl_open(&ctl) < 0) {
        return 1;
    }
    if (pdi_ctl_get_info(&ctl, &info) < 0) {
        pdebug_print_errno("get manager info");
        pdi_ctl_close(&ctl);
        return 1;
    }

    if (info.device_count == 0) {
        printf("no PDM devices registered\n");
        pdi_ctl_close(&ctl);
        return 0;
    }

    devices = calloc(info.device_count, sizeof(*devices));
    if (!devices) {
        pdebug_print_errno("allocate device list");
        pdi_ctl_close(&ctl);
        return 1;
    }

    count = info.device_count;
    if (pdi_list_devices(&ctl, devices, &count) < 0) {
        pdebug_print_errno("list devices");
        free(devices);
        pdi_ctl_close(&ctl);
        return 1;
    }

    for (i = 0; i < count; i++) {
        pdebug_print_device(&devices[i]);
    }

    free(devices);
    pdi_ctl_close(&ctl);
    return 0;
}

static int pdebug_discovery_get(int argc, char **argv)
{
    pdi_ctl_context_t ctl = { .fd = -1 };
    struct pdm_manager_device_info dev;

    if (argc != 1) {
        fprintf(stderr, "usage: pdebug discovery get <name>\n");
        return 1;
    }

    if (pdebug_ctl_open(&ctl) < 0) {
        return 1;
    }
    if (pdi_get_device_by_name(&ctl, argv[0], &dev) < 0) {
        pdebug_print_errno("get device by name");
        pdi_ctl_close(&ctl);
        return 1;
    }
    pdi_ctl_close(&ctl);

    pdebug_print_device(&dev);
    return 0;
}

static const struct pdebug_cmd pdebug_discovery_cmds[] = {
    { "info", "print PDM manager information", pdebug_discovery_info },
    { "list", "list registered PDM devices", pdebug_discovery_list },
    { "get", "print one device by stable name", pdebug_discovery_get },
};

static int pdebug_parse_mcu_selector(int argc, char **argv,
                                     struct pdebug_selector *selector,
                                     const char *usage)
{
    if (argc > 1) {
        fprintf(stderr, "usage: %s\n", usage);
        return -1;
    }

    if (pdebug_parse_selector(argc == 1 ? argv[0] : NULL, "mcu",
                              PDEBUG_MCU_PATH_FORMAT, selector) < 0) {
        fprintf(stderr, "usage: %s\n", usage);
        return -1;
    }
    return 0;
}

static int pdebug_parse_led_selector(int argc, char **argv,
                                     struct pdebug_selector *selector,
                                     const char *usage)
{
    if (argc > 1) {
        fprintf(stderr, "usage: %s\n", usage);
        return -1;
    }

    if (pdebug_parse_selector(argc == 1 ? argv[0] : NULL, "led",
                              PDEBUG_LED_PATH_FORMAT, selector) < 0) {
        fprintf(stderr, "usage: %s\n", usage);
        return -1;
    }
    return 0;
}

static int pdebug_mcu_info(int argc, char **argv)
{
    pdi_mcu_context_t ctx = { .fd = -1 };
    struct pdebug_selector selector;
    struct pdm_mcu_info info;

    if (pdebug_parse_mcu_selector(argc, argv, &selector,
                                  "pdebug mcu info [selector]") < 0) {
        return 1;
    }

    if (pdebug_mcu_open_selected(&ctx, &selector) < 0) {
        pdebug_print_errno("open MCU device");
        return 1;
    }
    if (pdi_mcu_get_info(&ctx, &info) < 0) {
        pdebug_print_errno("get MCU info");
        pdi_mcu_close(&ctx);
        return 1;
    }
    pdi_mcu_close(&ctx);

    printf("device:      %s\n", pdebug_selector_label(&selector, PDI_MCU_DEFAULT_DEVICE));
    printf("mcu ABI:     0x%08x\n", info.abi_version);
    printf("module:      %u.%u.%u\n", info.module_version_major,
           info.module_version_minor, info.module_version_patch);
    printf("open-count:  %u\n", info.open_count);
    printf("max-devices: %u\n", info.max_devices);
    return 0;
}

static int pdebug_mcu_status(int argc, char **argv)
{
    pdi_mcu_context_t ctx = { .fd = -1 };
    struct pdebug_selector selector;
    struct pdm_mcu_status status;

    if (pdebug_parse_mcu_selector(argc, argv, &selector,
                                  "pdebug mcu status [selector]") < 0) {
        return 1;
    }

    memset(&status, 0, sizeof(status));
    if (pdebug_mcu_open_selected(&ctx, &selector) < 0) {
        pdebug_print_errno("open MCU device");
        return 1;
    }
    if (pdi_mcu_get_status(&ctx, &status) < 0) {
        pdebug_print_errno("get MCU status");
        pdi_mcu_close(&ctx);
        return 1;
    }
    pdi_mcu_close(&ctx);

    printf("%s online=%s state=%s uptime=%u error=%u temp=",
           pdebug_selector_label(&selector, PDI_MCU_DEFAULT_DEVICE),
           status.online ? "yes" : "no",
           pdebug_mcu_state_name(status.state),
           status.uptime_sec,
           status.error_code);
    pdebug_print_temperature(status.temperature_milli_celsius);
    printf(" voltage=%umV timestamp=%" PRIu64 "us\n",
           status.voltage_mv, (uint64_t)status.timestamp_us);
    return 0;
}

static int pdebug_mcu_version(int argc, char **argv)
{
    pdi_mcu_context_t ctx = { .fd = -1 };
    struct pdebug_selector selector;
    struct pdm_mcu_version version;

    if (pdebug_parse_mcu_selector(argc, argv, &selector,
                                  "pdebug mcu version [selector]") < 0) {
        return 1;
    }

    memset(&version, 0, sizeof(version));
    if (pdebug_mcu_open_selected(&ctx, &selector) < 0) {
        pdebug_print_errno("open MCU device");
        return 1;
    }
    if (pdi_mcu_get_version(&ctx, &version) < 0) {
        pdebug_print_errno("get MCU version");
        pdi_mcu_close(&ctx);
        return 1;
    }
    pdi_mcu_close(&ctx);

    printf("%s version=%u.%u.%u.%u string=%s\n",
           pdebug_selector_label(&selector, PDI_MCU_DEFAULT_DEVICE),
           version.major, version.minor, version.patch, version.build,
           version.version_string);
    return 0;
}

static int pdebug_mcu_reset(int argc, char **argv)
{
    pdi_mcu_context_t ctx = { .fd = -1 };
    struct pdebug_selector selector;

    if (pdebug_parse_mcu_selector(argc, argv, &selector,
                                  "pdebug mcu reset [selector]") < 0) {
        return 1;
    }

    if (pdebug_mcu_open_selected(&ctx, &selector) < 0) {
        pdebug_print_errno("open MCU device");
        return 1;
    }
    if (pdi_mcu_reset(&ctx) < 0) {
        pdebug_print_errno("reset MCU");
        pdi_mcu_close(&ctx);
        return 1;
    }
    pdi_mcu_close(&ctx);

    printf("%s reset requested\n",
           pdebug_selector_label(&selector, PDI_MCU_DEFAULT_DEVICE));
    return 0;
}

static const struct pdebug_cmd pdebug_mcu_cmds[] = {
    { "info", "read MCU module info", pdebug_mcu_info },
    { "status", "read MCU runtime status", pdebug_mcu_status },
    { "version", "read MCU firmware version", pdebug_mcu_version },
    { "reset", "request MCU reset", pdebug_mcu_reset },
};

static int pdebug_led_info(int argc, char **argv)
{
    pdi_led_context_t ctx = { .fd = -1 };
    struct pdebug_selector selector;
    struct pdm_led_info info;

    if (pdebug_parse_led_selector(argc, argv, &selector,
                                  "pdebug led info [selector]") < 0) {
        return 1;
    }

    if (pdebug_led_open_selected(&ctx, &selector) < 0) {
        pdebug_print_errno("open LED device");
        return 1;
    }
    if (pdi_led_get_info(&ctx, &info) < 0) {
        pdebug_print_errno("get LED info");
        pdi_led_close(&ctx);
        return 1;
    }
    pdi_led_close(&ctx);

    printf("device:      %s\n", pdebug_selector_label(&selector, PDI_LED_DEFAULT_DEVICE));
    printf("led ABI:     0x%08x\n", info.abi_version);
    printf("module:      %u.%u.%u\n", info.module_version_major,
           info.module_version_minor, info.module_version_patch);
    printf("open-count:  %u\n", info.open_count);
    printf("max-devices: %u\n", info.max_devices);
    return 0;
}

static int pdebug_led_state(int argc, char **argv)
{
    pdi_led_context_t ctx = { .fd = -1 };
    struct pdebug_selector selector;
    struct pdm_led_state state;

    if (pdebug_parse_led_selector(argc, argv, &selector,
                                  "pdebug led state [selector]") < 0) {
        return 1;
    }

    memset(&state, 0, sizeof(state));
    if (pdebug_led_open_selected(&ctx, &selector) < 0) {
        pdebug_print_errno("open LED device");
        return 1;
    }
    if (pdi_led_get_state(&ctx, &state) < 0) {
        pdebug_print_errno("get LED state");
        pdi_led_close(&ctx);
        return 1;
    }
    pdi_led_close(&ctx);

    printf("%s brightness=%u/%u enabled=%s\n",
           pdebug_selector_label(&selector, PDI_LED_DEFAULT_DEVICE),
           state.brightness, state.max_brightness,
           state.enabled ? "yes" : "no");
    return 0;
}

static int pdebug_led_brightness(int argc, char **argv)
{
    pdi_led_context_t ctx = { .fd = -1 };
    struct pdebug_selector selector;
    uint32_t brightness;

    if (argc != 2) {
        fprintf(stderr, "usage: pdebug led brightness <selector> <value>\n");
        return 1;
    }
    if (pdebug_parse_selector(argv[0], "led", PDEBUG_LED_PATH_FORMAT,
                              &selector) < 0) {
        fprintf(stderr, "usage: pdebug led brightness <selector> <value>\n");
        return 1;
    }
    if (pdebug_parse_u32(argv[1], &brightness) < 0) {
        pdebug_print_errno("parse LED brightness");
        return 1;
    }

    if (pdebug_led_open_selected(&ctx, &selector) < 0) {
        pdebug_print_errno("open LED device");
        return 1;
    }
    if (pdi_led_set_brightness(&ctx, brightness) < 0) {
        pdebug_print_errno("set LED brightness");
        pdi_led_close(&ctx);
        return 1;
    }
    pdi_led_close(&ctx);

    printf("%s brightness set to %u\n",
           pdebug_selector_label(&selector, PDI_LED_DEFAULT_DEVICE),
           brightness);
    return 0;
}

static int pdebug_led_enable(int argc, char **argv)
{
    pdi_led_context_t ctx = { .fd = -1 };
    struct pdebug_selector selector;

    if (pdebug_parse_led_selector(argc, argv, &selector,
                                  "pdebug led enable [selector]") < 0) {
        return 1;
    }

    if (pdebug_led_open_selected(&ctx, &selector) < 0) {
        pdebug_print_errno("open LED device");
        return 1;
    }
    if (pdi_led_enable(&ctx) < 0) {
        pdebug_print_errno("enable LED");
        pdi_led_close(&ctx);
        return 1;
    }
    pdi_led_close(&ctx);

    printf("%s enabled\n", pdebug_selector_label(&selector, PDI_LED_DEFAULT_DEVICE));
    return 0;
}

static int pdebug_led_disable(int argc, char **argv)
{
    pdi_led_context_t ctx = { .fd = -1 };
    struct pdebug_selector selector;

    if (pdebug_parse_led_selector(argc, argv, &selector,
                                  "pdebug led disable [selector]") < 0) {
        return 1;
    }

    if (pdebug_led_open_selected(&ctx, &selector) < 0) {
        pdebug_print_errno("open LED device");
        return 1;
    }
    if (pdi_led_disable(&ctx) < 0) {
        pdebug_print_errno("disable LED");
        pdi_led_close(&ctx);
        return 1;
    }
    pdi_led_close(&ctx);

    printf("%s disabled\n", pdebug_selector_label(&selector, PDI_LED_DEFAULT_DEVICE));
    return 0;
}

static const struct pdebug_cmd pdebug_led_cmds[] = {
    { "info", "read LED module info", pdebug_led_info },
    { "state", "read LED state", pdebug_led_state },
    { "brightness", "set LED brightness", pdebug_led_brightness },
    { "enable", "enable LED", pdebug_led_enable },
    { "disable", "disable LED", pdebug_led_disable },
};

static void pdebug_print_cmds(const struct pdebug_cmd *cmds, size_t count)
{
    size_t i;

    for (i = 0; i < count; i++) {
        printf("  %-12s %s\n", cmds[i].name, cmds[i].summary);
    }
}

static int pdebug_dispatch(const struct pdebug_cmd *cmds, size_t count,
                           const char *group, int argc, char **argv)
{
    size_t i;

    if (argc < 1) {
        fprintf(stderr, "missing %s command\n", group);
        pdebug_print_cmds(cmds, count);
        return 1;
    }

    for (i = 0; i < count; i++) {
        if (strcmp(argv[0], cmds[i].name) == 0) {
            return cmds[i].run(argc - 1, argv + 1);
        }
    }

    fprintf(stderr, "unknown %s command: %s\n", group, argv[0]);
    pdebug_print_cmds(cmds, count);
    return 1;
}

static int pdebug_module_discovery(int argc, char **argv)
{
    return pdebug_dispatch(pdebug_discovery_cmds,
                           PDEBUG_ARRAY_SIZE(pdebug_discovery_cmds),
                           "discovery", argc, argv);
}

static int pdebug_module_mcu(int argc, char **argv)
{
    return pdebug_dispatch(pdebug_mcu_cmds, PDEBUG_ARRAY_SIZE(pdebug_mcu_cmds),
                           "mcu", argc, argv);
}

static int pdebug_module_led(int argc, char **argv)
{
    return pdebug_dispatch(pdebug_led_cmds, PDEBUG_ARRAY_SIZE(pdebug_led_cmds),
                           "led", argc, argv);
}

static const struct pdebug_cmd pdebug_modules[] = {
    { "discovery", "PDM manager discovery commands", pdebug_module_discovery },
    { "mcu", "MCU PDI commands", pdebug_module_mcu },
    { "led", "LED PDI commands", pdebug_module_led },
};

static void pdebug_usage(const char *program)
{
    printf("usage: %s <module> <command> [args...]\n", program);
    printf("\nmodules:\n");
    pdebug_print_cmds(pdebug_modules, PDEBUG_ARRAY_SIZE(pdebug_modules));
    printf("\nselector forms:\n");
    printf("  omitted             default device, e.g. /dev/pdm/mcu0 or /dev/pdm/led0\n");
    printf("  N                   /dev/pdm/<module>N\n");
    printf("  name:<stable-name>  open by PDM discovery name\n");
    printf("  path:<device-node>  open an explicit device node\n");
    printf("\nexamples:\n");
    printf("  %s discovery info\n", program);
    printf("  %s discovery list\n", program);
    printf("  %s mcu status 0\n", program);
    printf("  %s mcu version name:<stable-name>\n", program);
    printf("  %s led state 0\n", program);
    printf("  %s led brightness 0 128\n", program);
}

int main(int argc, char **argv)
{
    if (argc < 2 || strcmp(argv[1], "help") == 0 ||
        strcmp(argv[1], "--help") == 0) {
        pdebug_usage(argv[0]);
        return argc < 2 ? 1 : 0;
    }

    return pdebug_dispatch(pdebug_modules, PDEBUG_ARRAY_SIZE(pdebug_modules),
                           "module", argc - 1, argv + 1);
}
