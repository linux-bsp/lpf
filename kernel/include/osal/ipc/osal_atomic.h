/************************************************************************
 * Kernel OSAL atomic API
 ************************************************************************/

#ifndef OSAL_ATOMIC_H
#define OSAL_ATOMIC_H

#include <linux/atomic.h>

#include "osal_types.h"

typedef struct {
	atomic_t value;
} osal_atomic_uint32_t;

typedef struct {
	atomic64_t value;
} osal_atomic_uint64_t;

typedef struct {
	atomic_t value;
} osal_atomic_bool_t;

static inline void osal_atomic_init(osal_atomic_uint32_t *atomic,
				    uint32_t value)
{
	atomic_set(&atomic->value, (int)value);
}

static inline uint32_t osal_atomic_load(const osal_atomic_uint32_t *atomic)
{
	return (uint32_t)atomic_read(&atomic->value);
}

static inline void osal_atomic_store(osal_atomic_uint32_t *atomic,
				     uint32_t value)
{
	atomic_set(&atomic->value, (int)value);
}

static inline uint32_t osal_atomic_fetch_add(osal_atomic_uint32_t *atomic,
					     uint32_t value)
{
	return (uint32_t)atomic_fetch_add((int)value, &atomic->value);
}

static inline uint32_t osal_atomic_fetch_sub(osal_atomic_uint32_t *atomic,
					     uint32_t value)
{
	return (uint32_t)atomic_fetch_sub((int)value, &atomic->value);
}

static inline uint32_t osal_atomic_inc(osal_atomic_uint32_t *atomic)
{
	return (uint32_t)atomic_inc_return(&atomic->value);
}

static inline uint32_t osal_atomic_dec(osal_atomic_uint32_t *atomic)
{
	return (uint32_t)atomic_dec_return(&atomic->value);
}

static inline bool
osal_atomic_compare_exchange_strong(osal_atomic_uint32_t *atomic,
				    uint32_t *expected, uint32_t desired)
{
	int exp;

	if (!expected) {
		return false;
	}

	exp = (int)*expected;
	if (atomic_cmpxchg(&atomic->value, exp, (int)desired) == exp) {
		return true;
	}

	*expected = (uint32_t)atomic_read(&atomic->value);
	return false;
}

static inline void osal_atomic_init_u64(osal_atomic_uint64_t *atomic,
					uint64_t value)
{
	atomic64_set(&atomic->value, (s64)value);
}

static inline uint64_t osal_atomic_load_u64(const osal_atomic_uint64_t *atomic)
{
	return (uint64_t)atomic64_read(&atomic->value);
}

static inline void osal_atomic_store_u64(osal_atomic_uint64_t *atomic,
					 uint64_t value)
{
	atomic64_set(&atomic->value, (s64)value);
}

static inline uint64_t osal_atomic_fetch_add_u64(osal_atomic_uint64_t *atomic,
						 uint64_t value)
{
	return (uint64_t)atomic64_fetch_add((s64)value, &atomic->value);
}

static inline uint64_t osal_atomic_fetch_sub_u64(osal_atomic_uint64_t *atomic,
						 uint64_t value)
{
	return (uint64_t)atomic64_fetch_sub((s64)value, &atomic->value);
}

static inline uint64_t osal_atomic_inc_u64(osal_atomic_uint64_t *atomic)
{
	return (uint64_t)atomic64_inc_return(&atomic->value);
}

static inline uint64_t osal_atomic_dec_u64(osal_atomic_uint64_t *atomic)
{
	return (uint64_t)atomic64_dec_return(&atomic->value);
}

static inline bool
osal_atomic_compare_exchange_strong_u64(osal_atomic_uint64_t *atomic,
					uint64_t *expected, uint64_t desired)
{
	s64 exp;

	if (!expected) {
		return false;
	}

	exp = (s64)*expected;
	if (atomic64_cmpxchg(&atomic->value, exp, (s64)desired) == exp) {
		return true;
	}

	*expected = (uint64_t)atomic64_read(&atomic->value);
	return false;
}

static inline void osal_atomic_init_bool(osal_atomic_bool_t *atomic, bool value)
{
	atomic_set(&atomic->value, value ? 1 : 0);
}

static inline bool osal_atomic_load_bool(const osal_atomic_bool_t *atomic)
{
	return atomic_read(&atomic->value) != 0;
}

static inline void osal_atomic_store_bool(osal_atomic_bool_t *atomic,
					  bool value)
{
	atomic_set(&atomic->value, value ? 1 : 0);
}

static inline bool
osal_atomic_compare_exchange_strong_bool(osal_atomic_bool_t *atomic,
					 bool *expected, bool desired)
{
	uint32_t exp;
	bool ret;

	if (!expected) {
		return false;
	}

	exp = *expected ? 1U : 0U;
	ret = osal_atomic_compare_exchange_strong(
		(osal_atomic_uint32_t *)atomic, &exp, desired ? 1U : 0U);
	*expected = (exp != 0);
	return ret;
}

#endif /* OSAL_ATOMIC_H */
