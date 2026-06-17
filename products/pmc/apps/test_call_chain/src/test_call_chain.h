/**
 * @file test_call_chain.h
 * @brief Complete PMC call-chain test declarations.
 */

#ifndef TEST_CALL_CHAIN_H
#define TEST_CALL_CHAIN_H

#include <stdint.h>

/**
 * @brief Test a TC mapping that resolves to the CAN MCU path.
 * @return OSAL_SUCCESS on success, OSAL_ERR_* on failure.
 */
int32_t TestCallChain_MCU_CAN(void);

/**
 * @brief Test a TC mapping that resolves to the serial MCU path.
 * @return OSAL_SUCCESS on success, OSAL_ERR_* on failure.
 */
int32_t TestCallChain_MCU_Serial(void);

/**
 * @brief Test a power-control TC mapping through the current MCU-backed path.
 * @return OSAL_SUCCESS on success, OSAL_ERR_* on failure.
 */
int32_t TestCallChain_PowerControl(void);

/**
 * @brief Run all call-chain tests.
 * @return OSAL_SUCCESS when all tests pass, OSAL_ERR_GENERIC otherwise.
 */
int32_t TestCallChain_RunAll(void);

#endif /* TEST_CALL_CHAIN_H */
