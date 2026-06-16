/**
 * @file test_compat.h
 * @brief Legacy test framework compatibility layer
 *
 * Provides compatibility macros for old-style test code using
 * TEST_GROUP_START/TEST_RUN/TEST_GROUP_END pattern.
 */

#ifndef TEST_COMPAT_H
#define TEST_COMPAT_H

#include "test_core.h"

/**
 * Legacy compatibility macros
 * These are no-ops in the new framework - tests are registered via
 * constructor functions instead.
 */
#define TEST_GROUP_START(name) \
    do { \
        (void)(name); /* Suppress unused warning */ \
    } while (0)

#define TEST_RUN(func) \
    do { \
        func(); \
    } while (0)

#define TEST_GROUP_END() \
    do { \
    } while (0)

#endif /* TEST_COMPAT_H */
