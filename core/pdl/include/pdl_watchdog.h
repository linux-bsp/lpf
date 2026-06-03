/************************************************************************
 * Watchdog外设驱动接口（兼容性头文件）
 *
 * 注意：此头文件已废弃，请使用 pdl_watchdog_api.h
 * 此文件仅用于向后兼容，新代码应直接包含 api/ 目录下的头文件
 ************************************************************************/

#ifndef PDL_WATCHDOG_H
#define PDL_WATCHDOG_H

#include "pdl_watchdog_api.h"

/* 兼容性类型别名 */
typedef pdl_watchdog_handle_t watchdog_handle_t;
typedef pdl_watchdog_status_t watchdog_status_t;

#endif /* PDL_WATCHDOG_H */
