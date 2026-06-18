/**
 * @file prl.h
 * @brief PRL Protocol Layer - Unified API Entry Point
 */

#ifndef PRL_H
#define PRL_H

/* Core protocol definitions - always required */
#include "prl_common.h"
#include "prl_device.h"

/* Device-specific protocol - current MCU path */
#ifdef CONFIG_PRL_MCU
#include "prl_mcu.h"
#endif /* CONFIG_PRL_MCU */

int prl_init(void);
int prl_deinit(void);

#endif /* PRL_H */
