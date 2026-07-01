/************************************************************************
 * PDI lightweight logging API
 ************************************************************************/

#ifndef PDI_LOG_H
#define PDI_LOG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PDI_LOG_LEVEL_DEBUG 0
#define PDI_LOG_LEVEL_INFO 1
#define PDI_LOG_LEVEL_WARN 2
#define PDI_LOG_LEVEL_ERROR 3
#define PDI_LOG_LEVEL_FATAL 4

#define PDI_LOG_DEFAULT_LEVEL PDI_LOG_LEVEL_INFO

#ifdef CONFIG_LOG_BUFFER_SIZE
#define PDI_LOG_MESSAGE_SIZE CONFIG_LOG_BUFFER_SIZE
#else
#define PDI_LOG_MESSAGE_SIZE 512U
#endif

#ifndef PDI_LOG_COMPILE_LEVEL
#if !defined(CONFIG_PDI_LOGGING_ENABLE)
#define PDI_LOG_COMPILE_LEVEL (PDI_LOG_LEVEL_FATAL + 1)
#elif defined(CONFIG_PDI_LOG_LEVEL_FATAL)
#define PDI_LOG_COMPILE_LEVEL PDI_LOG_LEVEL_FATAL
#elif defined(CONFIG_PDI_LOG_LEVEL_ERROR)
#define PDI_LOG_COMPILE_LEVEL PDI_LOG_LEVEL_ERROR
#elif defined(CONFIG_PDI_LOG_LEVEL_WARN)
#define PDI_LOG_COMPILE_LEVEL PDI_LOG_LEVEL_WARN
#elif defined(CONFIG_PDI_LOG_LEVEL_INFO)
#define PDI_LOG_COMPILE_LEVEL PDI_LOG_LEVEL_INFO
#else
#define PDI_LOG_COMPILE_LEVEL PDI_LOG_LEVEL_DEBUG
#endif
#endif

#define LOG_DEBUG(...)                                                \
	do {                                                           \
		if (PDI_LOG_LEVEL_DEBUG >= PDI_LOG_COMPILE_LEVEL)       \
			pdi_log_emit(PDI_LOG_LEVEL_DEBUG, __FILE__,       \
				     __LINE__, __VA_ARGS__);           \
	} while (0)

#define LOG_INFO(...)                                                 \
	do {                                                           \
		if (PDI_LOG_LEVEL_INFO >= PDI_LOG_COMPILE_LEVEL)        \
			pdi_log_emit(PDI_LOG_LEVEL_INFO, __FILE__,        \
				     __LINE__, __VA_ARGS__);           \
	} while (0)

#define LOG_WARN(...)                                                 \
	do {                                                           \
		if (PDI_LOG_LEVEL_WARN >= PDI_LOG_COMPILE_LEVEL)        \
			pdi_log_emit(PDI_LOG_LEVEL_WARN, __FILE__,        \
				     __LINE__, __VA_ARGS__);           \
	} while (0)

#define LOG_ERROR(...)                                                \
	do {                                                           \
		if (PDI_LOG_LEVEL_ERROR >= PDI_LOG_COMPILE_LEVEL)       \
			pdi_log_emit(PDI_LOG_LEVEL_ERROR, __FILE__,       \
				     __LINE__, __VA_ARGS__);           \
	} while (0)

#define LOG_FATAL(...)                                                \
	do {                                                           \
		if (PDI_LOG_LEVEL_FATAL >= PDI_LOG_COMPILE_LEVEL)       \
			pdi_log_emit(PDI_LOG_LEVEL_FATAL, __FILE__,       \
				     __LINE__, __VA_ARGS__);           \
	} while (0)

#define LOG_DEBUG_ONCE(...)                                           \
	do {                                                           \
		static uint8_t __logged;                                \
		if (!__logged &&                                      \
		    PDI_LOG_LEVEL_DEBUG >= PDI_LOG_COMPILE_LEVEL) {   \
			__logged = 1;                                  \
			pdi_log_emit(PDI_LOG_LEVEL_DEBUG, __FILE__,    \
				     __LINE__, __VA_ARGS__);           \
		}                                                      \
	} while (0)

#define LOG_WARN_ONCE(...)                                            \
	do {                                                           \
		static uint8_t __logged;                                \
		if (!__logged &&                                      \
		    PDI_LOG_LEVEL_WARN >= PDI_LOG_COMPILE_LEVEL) {    \
			__logged = 1;                                  \
			pdi_log_emit(PDI_LOG_LEVEL_WARN, __FILE__,     \
				     __LINE__, __VA_ARGS__);           \
		}                                                      \
	} while (0)

#define LOG_ERROR_ONCE(...)                                           \
	do {                                                           \
		static uint8_t __logged;                                \
		if (!__logged &&                                      \
		    PDI_LOG_LEVEL_ERROR >= PDI_LOG_COMPILE_LEVEL) {   \
			__logged = 1;                                  \
			pdi_log_emit(PDI_LOG_LEVEL_ERROR, __FILE__,    \
				     __LINE__, __VA_ARGS__);           \
		}                                                      \
	} while (0)

void pdi_log_set_level(int level);
void pdi_log_get_stats(uint64_t *total_count, uint64_t *dropped_count);
void pdi_log_emit(int level, const char *file, int line, const char *format, ...)
	__attribute__((format(printf, 4, 5)));

#ifdef __cplusplus
}
#endif

#endif /* PDI_LOG_H */
