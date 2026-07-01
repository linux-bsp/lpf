/************************************************************************
 * PDI lightweight logging implementation
 ************************************************************************/

#include "pdi/pdi_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static int g_log_level = PDI_LOG_DEFAULT_LEVEL;
static uint64_t g_log_total;
static uint64_t g_log_dropped;

static const char *pdi_log_level_name(int level)
{
	switch (level) {
	case PDI_LOG_LEVEL_DEBUG:
		return "DEBUG";
	case PDI_LOG_LEVEL_INFO:
		return "INFO";
	case PDI_LOG_LEVEL_WARN:
		return "WARN";
	case PDI_LOG_LEVEL_ERROR:
		return "ERROR";
	case PDI_LOG_LEVEL_FATAL:
		return "FATAL";
	default:
		return "UNKNOWN";
	}
}

static const char *pdi_log_basename(const char *path)
{
	const char *name;

	if (!path) {
		return "";
	}

	name = strrchr(path, '/');
	return name ? name + 1 : path;
}

void pdi_log_set_level(int level)
{
	g_log_level = level;
}

void pdi_log_get_stats(uint64_t *total_count, uint64_t *dropped_count)
{
	if (total_count) {
		*total_count = g_log_total;
	}
	if (dropped_count) {
		*dropped_count = g_log_dropped;
	}
}

void pdi_log_emit(int level, const char *file, int line, const char *format, ...)
{
	va_list args;
	const char *filename = pdi_log_basename(file);

	if (level < g_log_level) {
		g_log_dropped++;
		return;
	}

	fprintf(stderr, "[PDI:%s] %s:%d - ", pdi_log_level_name(level),
		filename, line);
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputc('\n', stderr);
	g_log_total++;
}
