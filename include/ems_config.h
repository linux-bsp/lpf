/**
 * @file ems_config.h
 * @brief EMS Master Configuration Header
 *
 * This file includes the auto-generated Kconfig header and provides
 * backward compatibility macros for existing code.
 */

#ifndef EMS_CONFIG_H
#define EMS_CONFIG_H

/* Include auto-generated Kconfig configuration */
#if __has_include("generated/autoconf.h")
#include "generated/autoconf.h"
#else
#warning "No Kconfig configuration found, using defaults"
/* Provide default values if Kconfig not configured */
#define CONFIG_HAL_UART 1
#define CONFIG_HAL_UART_BAUDRATE 115200
#define CONFIG_HAL_CAN 1
#define CONFIG_HAL_GPIO 1
#define CONFIG_HAL_WATCHDOG 1
#endif

/* Backward compatibility macros (for existing code) */
#ifdef CONFIG_HAL_UART_BAUDRATE
#define UART_DEFAULT_BAUDRATE CONFIG_HAL_UART_BAUDRATE
#endif

#ifdef CONFIG_HAL_UART_DEVICE
#define UART_DEFAULT_DEVICE CONFIG_HAL_UART_DEVICE
#endif

#ifdef CONFIG_HAL_CAN_DEVICE
#define CAN_DEFAULT_DEVICE CONFIG_HAL_CAN_DEVICE
#endif

#ifdef CONFIG_HAL_WATCHDOG_TIMEOUT
#define WATCHDOG_DEFAULT_TIMEOUT CONFIG_HAL_WATCHDOG_TIMEOUT
#endif

#ifdef CONFIG_HAL_WATCHDOG_DEVICE
#define WATCHDOG_DEFAULT_DEVICE CONFIG_HAL_WATCHDOG_DEVICE
#endif

/* Feature test macros */
#ifdef CONFIG_BUILD_TYPE_DEBUG
#define EMS_DEBUG_BUILD 1
#else
#define EMS_DEBUG_BUILD 0
#endif

#ifdef CONFIG_BUILD_TESTING
#define EMS_TESTING_ENABLED 1
#else
#define EMS_TESTING_ENABLED 0
#endif

/* OSAL configuration macros */
#ifdef CONFIG_OSAL_MAX_TASKS
#define OSAL_MAX_TASKS CONFIG_OSAL_MAX_TASKS
#endif

#ifdef CONFIG_OSAL_MAX_QUEUES
#define OSAL_MAX_QUEUES CONFIG_OSAL_MAX_QUEUES
#endif

#ifdef CONFIG_OSAL_MAX_MUTEXES
#define OSAL_MAX_MUTEXES CONFIG_OSAL_MAX_MUTEXES
#endif

/* ACL configuration macros */
#ifdef CONFIG_ACL_MAX_DEVICES
#define ACL_MAX_DEVICES CONFIG_ACL_MAX_DEVICES
#endif

#endif /* EMS_CONFIG_H */
