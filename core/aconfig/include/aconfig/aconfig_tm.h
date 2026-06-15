/**
 * @file aconfig_tm.h
 * @brief ACONFIG 遥测功能定义 - 兼容性包装（已废弃）
 * @deprecated 此文件已废弃，业务功能枚举已移到产品层
 *             请使用 products/ccm/include/ccm/ccm_tm_functions.h
 *
 * @note 此文件保留用于向后兼容，新代码应直接包含产品层头文件
 */

#ifndef ACONFIG_TM_H
#define ACONFIG_TM_H

/* 包含产品层头文件 */
#if defined(CONFIG_CCM) || defined(CONFIG_PRODUCT_CCM)
#include "ccm_tm_functions.h"

/* 类型别名（向后兼容） */
typedef ccm_tm_function_t aconfig_tm_function_t;

#else
#error "No product layer TM functions defined. Please include product-specific header or define CONFIG_PRODUCT_CCM."
#endif

#endif /* ACONFIG_TM_H */
