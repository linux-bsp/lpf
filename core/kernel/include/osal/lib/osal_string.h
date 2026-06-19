/************************************************************************
 * Kernel OSAL string API
 ************************************************************************/

#ifndef OSAL_STRING_H
#define OSAL_STRING_H

#include <linux/stdarg.h>

#include "osal_types.h"

void *osal_memset(void *s, int32_t c, osal_size_t n);
void *osal_memcpy(void *dest, const void *src, osal_size_t n);
void *osal_memmove(void *dest, const void *src, osal_size_t n);
int32_t osal_memcmp(const void *s1, const void *s2, osal_size_t n);
osal_size_t osal_strlen(const char *s);
int32_t osal_strcmp(const char *s1, const char *s2);
int32_t osal_strncmp(const char *s1, const char *s2, osal_size_t n);
int32_t osal_strcasecmp(const char *s1, const char *s2);
char *osal_strcpy(char *dest, const char *src);
char *osal_strncpy(char *dest, const char *src, osal_size_t n);
char *osal_strcat(char *dest, const char *src);
char *osal_strncat(char *dest, const char *src, osal_size_t n);
char *osal_strstr(const char *haystack, const char *needle);
osal_size_t osal_strcspn(const char *s, const char *reject);
int32_t osal_sprintf(char *str, const char *format, ...);
int32_t osal_snprintf(char *str, osal_size_t size, const char *format, ...);
int32_t osal_vsnprintf(char *str, osal_size_t size, const char *format,
		       va_list ap);
int32_t osal_sscanf(const char *str, const char *format, ...);
int32_t osal_atoi(const char *nptr);
long osal_atol(const char *nptr);
long osal_strtol(const char *nptr, char **endptr, int32_t base);

#endif /* OSAL_STRING_H */
