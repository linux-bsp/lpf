/************************************************************************
 * Kernel OSAL log API
 ************************************************************************/

#ifndef OSAL_LOG_H
#define OSAL_LOG_H

#include "osal_types.h"

#define OS_LOG_LEVEL_DEBUG 0x0
#define OS_LOG_LEVEL_INFO 0x1
#define OS_LOG_LEVEL_WARN 0x2
#define OS_LOG_LEVEL_ERROR 0x3
#define OS_LOG_LEVEL_FATAL 0x4

#define OSAL_LOG_DEFAULT_LEVEL OS_LOG_LEVEL_INFO
#define OSAL_LOG_MESSAGE_SIZE 0x200U
#define OSAL_LOG_MAX_KV_PAIRS 0x8U

#ifndef OSAL_LOG_COMPILE_LEVEL
#if !defined(CONFIG_LOGGING_ENABLE)
#define OSAL_LOG_COMPILE_LEVEL (OS_LOG_LEVEL_FATAL + 1)
#elif defined(CONFIG_LOG_LEVEL_FATAL)
#define OSAL_LOG_COMPILE_LEVEL OS_LOG_LEVEL_FATAL
#elif defined(CONFIG_LOG_LEVEL_ERROR)
#define OSAL_LOG_COMPILE_LEVEL OS_LOG_LEVEL_ERROR
#elif defined(CONFIG_LOG_LEVEL_WARN)
#define OSAL_LOG_COMPILE_LEVEL OS_LOG_LEVEL_WARN
#elif defined(CONFIG_LOG_LEVEL_INFO)
#define OSAL_LOG_COMPILE_LEVEL OS_LOG_LEVEL_INFO
#else
#define OSAL_LOG_COMPILE_LEVEL OS_LOG_LEVEL_DEBUG
#endif
#endif

typedef struct {
	const char *key;
	const char *value;
} log_kv_pair_t;

#define LOG_DEBUG(...)                                               \
	do {                                                              \
		if (OS_LOG_LEVEL_DEBUG >= OSAL_LOG_COMPILE_LEVEL)             \
			osal_log_emit(OS_LOG_LEVEL_DEBUG, __FILE__, __LINE__,       \
				      __VA_ARGS__);                                      \
	} while (0)

#define LOG_INFO(...)                                                \
	do {                                                              \
		if (OS_LOG_LEVEL_INFO >= OSAL_LOG_COMPILE_LEVEL)              \
			osal_log_emit(OS_LOG_LEVEL_INFO, __FILE__, __LINE__,        \
				      __VA_ARGS__);                                      \
	} while (0)

#define LOG_WARN(...)                                                \
	do {                                                              \
		if (OS_LOG_LEVEL_WARN >= OSAL_LOG_COMPILE_LEVEL)              \
			osal_log_emit(OS_LOG_LEVEL_WARN, __FILE__, __LINE__,        \
				      __VA_ARGS__);                                      \
	} while (0)

#define LOG_ERROR(...)                                               \
	do {                                                              \
		if (OS_LOG_LEVEL_ERROR >= OSAL_LOG_COMPILE_LEVEL)             \
			osal_log_emit(OS_LOG_LEVEL_ERROR, __FILE__, __LINE__,       \
				      __VA_ARGS__);                                      \
	} while (0)

#define LOG_FATAL(...)                                               \
	do {                                                              \
		if (OS_LOG_LEVEL_FATAL >= OSAL_LOG_COMPILE_LEVEL)             \
			osal_log_emit(OS_LOG_LEVEL_FATAL, __FILE__, __LINE__,       \
				      __VA_ARGS__);                                      \
	} while (0)

#define LOG_DEBUG_ONCE(...)                                          \
	do {                                                              \
		static bool __logged;                                         \
		if (!__logged && OS_LOG_LEVEL_DEBUG >= OSAL_LOG_COMPILE_LEVEL) { \
			__logged = true;                                      \
			osal_log_emit(OS_LOG_LEVEL_DEBUG, __FILE__, __LINE__,  \
				      __VA_ARGS__);                                \
		}                                                           \
	} while (0)

#define LOG_WARN_ONCE(...)                                           \
	do {                                                              \
		static bool __logged;                                         \
		if (!__logged && OS_LOG_LEVEL_WARN >= OSAL_LOG_COMPILE_LEVEL) { \
			__logged = true;                                      \
			osal_log_emit(OS_LOG_LEVEL_WARN, __FILE__, __LINE__,   \
				      __VA_ARGS__);                                \
		}                                                           \
	} while (0)

#define LOG_ERROR_ONCE(...)                                          \
	do {                                                              \
		static bool __logged;                                         \
		if (!__logged && OS_LOG_LEVEL_ERROR >= OSAL_LOG_COMPILE_LEVEL) { \
			__logged = true;                                      \
			osal_log_emit(OS_LOG_LEVEL_ERROR, __FILE__, __LINE__,  \
				      __VA_ARGS__);                                \
		}                                                           \
	} while (0)

int32_t osal_log_init(const char *log_file_path, int32_t level);
void osal_log_shutdown(void);
void osal_log_set_level(int32_t level);
void osal_log_set_max_file_size(uint32_t size_bytes);
void osal_log_set_max_files(uint32_t max_files);
int32_t osal_log_set_filter(const char *pattern);
void osal_log_set_sampling(uint32_t rate);
int32_t osal_log_set_remote(const char *host, uint16_t port);
void osal_log_disable_remote(void);
void osal_log(int32_t level, const char *format, ...);
void osal_log_structured(int32_t level, const char *message,
			 const log_kv_pair_t *kv_pairs, uint32_t kv_count);
void osal_printf(const char *format, ...);
void osal_log_get_stats(uint64_t *total_count, uint64_t *dropped_count);
void osal_log_emit(int32_t level, const char *file, int32_t line,
		   const char *format, ...)
	__printf(4, 5);

#endif /* OSAL_LOG_H */
