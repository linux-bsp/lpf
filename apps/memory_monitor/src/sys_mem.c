/*******************************************************************************
 * AM62x Memory Statistics Tool - System Memory Collection Implementation
 * 系统内存采集模块实现
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdint.h>
#include <inttypes.h>
#include "sys_mem.h"

#define PROC_MEMINFO "/proc/meminfo"
#define PROC_IOMEM   "/proc/iomem"
#define PROC_ZONEINFO "/proc/zoneinfo"

/*******************************************************************************
 * 采集系统内存信息
 ******************************************************************************/
int collect_system_memory(system_mem_info_t *info) {
    FILE *fp;
    char line[256];
    char key[64];
    mem_size_t value;

    if (!info) return -1;
    memset(info, 0, sizeof(system_mem_info_t));

    fp = fopen(PROC_MEMINFO, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open %s: %s\n", PROC_MEMINFO, strerror(errno));
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%63s %" SCNu64 " kB", key, &value) == 2) {
            if (strcmp(key, "MemTotal:") == 0) {
                info->total = value;
            } else if (strcmp(key, "MemFree:") == 0) {
                info->free = value;
            } else if (strcmp(key, "MemAvailable:") == 0) {
                info->available = value;
            } else if (strcmp(key, "Buffers:") == 0) {
                info->buffers = value;
            } else if (strcmp(key, "Cached:") == 0) {
                info->cached = value;
            } else if (strcmp(key, "Slab:") == 0) {
                info->slab = value;
            } else if (strcmp(key, "KernelStack:") == 0) {
                info->kernel_stack = value;
            } else if (strcmp(key, "PageTables:") == 0) {
                info->page_tables = value;
            } else if (strcmp(key, "VmallocUsed:") == 0) {
                info->vmalloc_used = value;
            }
        }
    }

    fclose(fp);

    /* 计算已使用内存 */
    info->used = info->total - info->free;

    return 0;
}

/*******************************************************************************
 * 采集内核内存信息
 ******************************************************************************/
int collect_kernel_memory(kernel_mem_info_t *kernel, const system_mem_info_t *sys) {
    if (!kernel || !sys) return -1;

    kernel->slab = sys->slab;
    kernel->kernel_stack = sys->kernel_stack;
    kernel->page_tables = sys->page_tables;
    kernel->vmalloc_used = sys->vmalloc_used;
    kernel->total = kernel->slab + kernel->kernel_stack +
                    kernel->page_tables + kernel->vmalloc_used;

    return 0;
}

/*******************************************************************************
 * 采集保留内存信息 - 从设备树读取
 ******************************************************************************/
int collect_reserved_memory(reserved_mem_info_t *reserved) {
    DIR *dir;
    struct dirent *entry;
    const char *dt_path = "/proc/device-tree/reserved-memory";

    if (!reserved) return -1;
    memset(reserved, 0, sizeof(reserved_mem_info_t));

    /* 先尝试从设备树读取 */
    dir = opendir(dt_path);
    if (dir) {
        while ((entry = readdir(dir)) != NULL && reserved->count < MAX_RESERVED_REGIONS) {
            /* 跳过特殊目录和属性文件 */
            if (entry->d_name[0] == '.' || entry->d_name[0] == '#' ||
                strcmp(entry->d_name, "name") == 0 ||
                strcmp(entry->d_name, "ranges") == 0 ||
                strcmp(entry->d_name, "phandle") == 0) {
                continue;
            }

            char reg_path[512];
            snprintf(reg_path, sizeof(reg_path), "%s/%s/reg", dt_path, entry->d_name);

            FILE *fp = fopen(reg_path, "rb");
            if (fp) {
                unsigned char buf[16];
                memset(buf, 0, sizeof(buf));
                size_t n = fread(buf, 1, 16, fp);
                fclose(fp);

                if (n >= 8) {  /* 至少需要8字节（地址部分） */
                    /* 解析设备树的reg属性 (big-endian) */
                    uint64_t addr, size;

                    if (n == 16) {
                        /* 64位地址 + 64位大小 */
                        addr = ((uint64_t)buf[0] << 56) | ((uint64_t)buf[1] << 48) |
                               ((uint64_t)buf[2] << 40) | ((uint64_t)buf[3] << 32) |
                               ((uint64_t)buf[4] << 24) | ((uint64_t)buf[5] << 16) |
                               ((uint64_t)buf[6] << 8)  | ((uint64_t)buf[7]);

                        size = ((uint64_t)buf[8] << 56) | ((uint64_t)buf[9] << 48) |
                               ((uint64_t)buf[10] << 40) | ((uint64_t)buf[11] << 32) |
                               ((uint64_t)buf[12] << 24) | ((uint64_t)buf[13] << 16) |
                               ((uint64_t)buf[14] << 8)  | ((uint64_t)buf[15]);

                        /* 调试输出 */
                        #ifdef DEBUG_REG_PARSE
                        fprintf(stderr, "DEBUG: %s: n=%zu, addr=0x%" PRIx64
                                ", size=0x%" PRIx64 " (%" PRIu64 " bytes)\n",
                                entry->d_name, n, addr, size, size);
                        #endif
                    } else if (n == 8) {
                        /* 32位地址 + 32位大小 */
                        addr = ((uint64_t)buf[0] << 24) | ((uint64_t)buf[1] << 16) |
                               ((uint64_t)buf[2] << 8)  | ((uint64_t)buf[3]);

                        size = ((uint64_t)buf[4] << 24) | ((uint64_t)buf[5] << 16) |
                               ((uint64_t)buf[6] << 8)  | ((uint64_t)buf[7]);
                    } else {
                        /* 其他长度，跳过 */
                        continue;
                    }

                    /* 统计所有区域，包括size为0的（但只记录有大小的） */
                    if (size > 0) {
                        reserved_mem_region_t *region = &reserved->regions[reserved->count];
                        region->start_addr = addr;
                        region->end_addr = addr + size - 1;
                        /* 保存原始字节数，不要立即转换为KB，避免小于1024字节的被截断为0 */
                        region->size = size; /* 保持字节单位 */

                        /* 复制名称 (去掉@后的地址部分) */
                        snprintf(region->name, sizeof(region->name), "%s", entry->d_name);

                        /* 去掉@后的地址 */
                        char *at = strchr(region->name, '@');
                        if (at) *at = '\0';

                        reserved->count++;
                    }
                }
            }
        }
        closedir(dir);
    }

    /* 如果设备树读取失败，回退到/proc/iomem */
    if (reserved->count == 0) {
        FILE *fp = fopen(PROC_IOMEM, "r");
        if (!fp) {
            fprintf(stderr, "Failed to open %s: %s\n", PROC_IOMEM, strerror(errno));
            return -1;
        }

        char line[512];
        while (fgets(line, sizeof(line), fp) && reserved->count < MAX_RESERVED_REGIONS) {
            char *colon = strchr(line, ':');
            if (!colon) continue;

            /* 检查是否是Reserved */
            if (strstr(line, "Reserved")) {
                reserved_mem_region_t *region = &reserved->regions[reserved->count];

                unsigned long long start, end;
                if (sscanf(line, "%llx-%llx", &start, &end) == 2) {
                    region->start_addr = start;
                    region->end_addr = end;
                    region->size = (end - start + 1) / 1024;

                    char *name_start = colon + 1;
                    while (*name_start == ' ') name_start++;
                    char *name_end = strchr(name_start, '\n');
                    if (name_end) *name_end = '\0';

                    snprintf(region->name, sizeof(region->name), "%s", name_start);

                    reserved->count++;
                }
            }
        }
        fclose(fp);
    }

    return 0;
}

/*******************************************************************************
 * 采集内存Zone信息
 ******************************************************************************/
int collect_zones_info(zones_info_t *zones) {
    FILE *fp;
    char line[256];

    if (!zones) return -1;
    memset(zones, 0, sizeof(zones_info_t));

    fp = fopen(PROC_ZONEINFO, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open %s: %s\n", PROC_ZONEINFO, strerror(errno));
        return -1;
    }

    zone_info_t *current_zone = NULL;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "Node") && strstr(line, "zone")) {
            /* 新的Zone */
            if (zones->count < MAX_ZONES) {
                current_zone = &zones->zones[zones->count];
                sscanf(line, "Node %*d, zone %63s", current_zone->name);
                zones->count++;
            }
        } else if (current_zone && strstr(line, "pages free")) {
            unsigned long pages;
            sscanf(line, " pages free %lu", &pages);
            current_zone->free_pages = pages * 4; /* 页面大小4KB */
        } else if (current_zone && strstr(line, "min")) {
            unsigned long min;
            sscanf(line, " min %lu", &min);
            current_zone->min = min * 4;
        } else if (current_zone && strstr(line, "low")) {
            unsigned long low;
            sscanf(line, " low %lu", &low);
            current_zone->low = low * 4;
        } else if (current_zone && strstr(line, "high")) {
            unsigned long high;
            sscanf(line, " high %lu", &high);
            current_zone->high = high * 4;
        } else if (current_zone && strstr(line, "present")) {
            unsigned long present;
            sscanf(line, " present %lu", &present);
            current_zone->present = present * 4;
        }
    }

    fclose(fp);
    return 0;
}

/*******************************************************************************
 * 计算内存健康状态
 ******************************************************************************/
void calculate_memory_health(memory_health_t *health, const system_mem_info_t *sys) {
    if (!health || !sys || sys->total == 0) return;

    health->usage_percent = (int)((sys->used * 100) / sys->total);
    health->available_percent = (int)((sys->available * 100) / sys->total);

    /* 判断健康状态 */
    if (health->usage_percent < 70) {
        health->status = MEM_HEALTH_GOOD;
    } else if (health->usage_percent < 85) {
        health->status = MEM_HEALTH_WARNING;
    } else {
        health->status = MEM_HEALTH_CRITICAL;
    }
}

/*******************************************************************************
 * 计算内存矩阵
 ******************************************************************************/
void calculate_memory_matrix(memory_matrix_t *matrix, const memory_statistics_t *stats) {
    if (!matrix || !stats) return;

    matrix->total = stats->system.total;

    /* 1. OS内核 */
    matrix->layers[0].layer_name = "OS内核 (Kernel)";
    matrix->layers[0].size = stats->kernel.total;
    matrix->layers[0].percentage = (int)((stats->kernel.total * 100) / matrix->total);
    matrix->layers[0].description = "内核核心";

    /* 2. 缓冲区 */
    matrix->layers[1].layer_name = "缓冲区 (Buffers)";
    matrix->layers[1].size = stats->system.buffers;
    matrix->layers[1].percentage = (int)((stats->system.buffers * 100) / matrix->total);
    matrix->layers[1].description = "IO缓冲";

    /* 3. 页缓存 */
    matrix->layers[2].layer_name = "页缓存 (Cached)";
    matrix->layers[2].size = stats->system.cached;
    matrix->layers[2].percentage = (int)((stats->system.cached * 100) / matrix->total);
    matrix->layers[2].description = "文件缓存";

    /* 4. 用户态 */
    matrix->layers[3].layer_name = "用户态 (User Space)";
    matrix->layers[3].size = stats->processes.total_rss;
    matrix->layers[3].percentage = (int)((stats->processes.total_rss * 100) / matrix->total);
    matrix->layers[3].description = "应用程序";

    /* 5. 其他 */
    mem_size_t accounted = stats->kernel.total + stats->system.buffers +
                           stats->system.cached + stats->processes.total_rss +
                           stats->system.free;
    matrix->layers[4].layer_name = "其他 (Other)";
    matrix->layers[4].size = (matrix->total > accounted) ? (matrix->total - accounted) : 0;
    matrix->layers[4].percentage = (int)((matrix->layers[4].size * 100) / matrix->total);
    matrix->layers[4].description = "保留/硬件";

    /* 6. 空闲 */
    matrix->layers[5].layer_name = "空闲 (Free)";
    matrix->layers[5].size = stats->system.free;
    matrix->layers[5].percentage = (int)((stats->system.free * 100) / matrix->total);
    matrix->layers[5].description = "未使用";
}
