// SPDX-License-Identifier: GPL-2.0

#include "osal/osal.h"

#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/string.h>

void *osal_memset(void *s, int32_t c, osal_size_t n)
{
	return memset(s, c, n);
}
EXPORT_SYMBOL_GPL(osal_memset);

void *osal_memcpy(void *dest, const void *src, osal_size_t n)
{
	return memcpy(dest, src, n);
}
EXPORT_SYMBOL_GPL(osal_memcpy);

void *osal_memmove(void *dest, const void *src, osal_size_t n)
{
	return memmove(dest, src, n);
}
EXPORT_SYMBOL_GPL(osal_memmove);

int32_t osal_memcmp(const void *s1, const void *s2, osal_size_t n)
{
	return memcmp(s1, s2, n);
}
EXPORT_SYMBOL_GPL(osal_memcmp);

osal_size_t osal_strlen(const char *s)
{
	return strlen(s);
}
EXPORT_SYMBOL_GPL(osal_strlen);

int32_t osal_strcmp(const char *s1, const char *s2)
{
	return strcmp(s1, s2);
}
EXPORT_SYMBOL_GPL(osal_strcmp);

int32_t osal_strncmp(const char *s1, const char *s2, osal_size_t n)
{
	return strncmp(s1, s2, n);
}
EXPORT_SYMBOL_GPL(osal_strncmp);

int32_t osal_strcasecmp(const char *s1, const char *s2)
{
	return strcasecmp(s1, s2);
}
EXPORT_SYMBOL_GPL(osal_strcasecmp);

char *osal_strcpy(char *dest, const char *src)
{
	strcpy(dest, src);
	return dest;
}
EXPORT_SYMBOL_GPL(osal_strcpy);

char *osal_strncpy(char *dest, const char *src, osal_size_t n)
{
	strscpy(dest, src, n);
	return dest;
}
EXPORT_SYMBOL_GPL(osal_strncpy);

char *osal_strcat(char *dest, const char *src)
{
	strcat(dest, src);
	return dest;
}
EXPORT_SYMBOL_GPL(osal_strcat);

char *osal_strncat(char *dest, const char *src, osal_size_t n)
{
	strncat(dest, src, n);
	return dest;
}
EXPORT_SYMBOL_GPL(osal_strncat);

char *osal_strstr(const char *haystack, const char *needle)
{
	return strstr(haystack, needle);
}
EXPORT_SYMBOL_GPL(osal_strstr);

osal_size_t osal_strcspn(const char *s, const char *reject)
{
	return strcspn(s, reject);
}
EXPORT_SYMBOL_GPL(osal_strcspn);

int32_t osal_sprintf(char *str, const char *format, ...)
{
	va_list args;
	int32_t ret;

	va_start(args, format);
	ret = vsprintf(str, format, args);
	va_end(args);

	return ret;
}
EXPORT_SYMBOL_GPL(osal_sprintf);

int32_t osal_snprintf(char *str, osal_size_t size, const char *format, ...)
{
	va_list args;
	int32_t ret;

	va_start(args, format);
	ret = vsnprintf(str, size, format, args);
	va_end(args);

	return ret;
}
EXPORT_SYMBOL_GPL(osal_snprintf);

int32_t osal_vsnprintf(char *str, osal_size_t size, const char *format,
		       va_list args)
{
	return vsnprintf(str, size, format, args);
}
EXPORT_SYMBOL_GPL(osal_vsnprintf);

int32_t osal_sscanf(const char *str, const char *format, ...)
{
	va_list args;
	int32_t ret;

	va_start(args, format);
	ret = vsscanf(str, format, args);
	va_end(args);

	return ret;
}
EXPORT_SYMBOL_GPL(osal_sscanf);

int32_t osal_atoi(const char *nptr)
{
	long value = 0;

	if (kstrtol(nptr, 0, &value))
		return 0;

	return (int32_t)value;
}
EXPORT_SYMBOL_GPL(osal_atoi);

long osal_atol(const char *nptr)
{
	long value = 0;

	if (kstrtol(nptr, 0, &value))
		return 0;

	return value;
}
EXPORT_SYMBOL_GPL(osal_atol);

long osal_strtol(const char *nptr, char **endptr, int32_t base)
{
	long value = 0;

	if (endptr)
		*endptr = (char *)nptr;

	if (kstrtol(nptr, base, &value))
		return 0;

	if (endptr)
		*endptr = (char *)nptr + strlen(nptr);

	return value;
}
EXPORT_SYMBOL_GPL(osal_strtol);
