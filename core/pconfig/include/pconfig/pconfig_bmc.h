/************************************************************************
 * PCONFIG BMC 配置类型定义
 *
 * 功能：
 * - BMC外设配置类型（完整定义）
 *
 * 说明：
 * - 配置类型由 PCONFIG 定义，PDL 使用
 * - 避免循环依赖
 ************************************************************************/

#ifndef PCONFIG_BMC_H
#define PCONFIG_BMC_H

#include <stdint.h>
#include <stdbool.h>
#include "pconfig_common.h"

/*===========================================================================
 * BMC 配置类型定义
 *===========================================================================*/

/**
 * @brief BMC通信通道类型
 */
typedef enum
{
	PCONFIG_BMC_CHANNEL_NETWORK = 0x00,  /* 网络通道（IPMI over LAN） */
	PCONFIG_BMC_CHANNEL_SERIAL  = 0x01   /* 串口通道（IPMI over Serial） */
} pconfig_bmc_channel_t;

/**
 * @brief BMC协议类型
 */
typedef enum
{
	PCONFIG_BMC_PROTOCOL_IPMI = 0x00,    /* IPMI协议 */
	PCONFIG_BMC_PROTOCOL_REDFISH = 0x01  /* Redfish协议 */
} pconfig_bmc_protocol_t;

/**
 * @brief BMC通道配置（union形式）
 */
typedef union
{
	/* 网络通道配置 */
	struct {
		const char *ip_addr;      /* IP地址 */
		uint16_t port;            /* 端口（默认623） */
		const char *username;     /* 用户名 */
		const char *password;     /* 密码 */
		uint32_t timeout_ms;      /* 超时时间 */
	} network;

	/* 串口通道配置 */
	struct {
		const char *device;       /* 串口设备 */
		uint32_t baudrate;        /* 波特率 */
		uint8_t data_bits;        /* 数据位（默认8） */
		uint8_t stop_bits;        /* 停止位（默认1） */
		uint8_t parity;           /* 校验位（默认NONE） */
		uint32_t timeout_ms;      /* 超时时间 */
	} serial;
} pconfig_bmc_channel_config_t;

/**
 * @brief BMC配置（完整定义）
 */
typedef struct
{
	/* 主通道配置 */
	pconfig_bmc_channel_t primary_channel;       /* 主通道类型 */
	pconfig_bmc_channel_config_t primary_config; /* 主通道配置 */

	/* 备用通道配置 */
	pconfig_bmc_channel_t backup_channel;        /* 备用通道类型 */
	pconfig_bmc_channel_config_t backup_config;  /* 备用通道配置 */

	/* 服务配置 */
	bool auto_switch;                            /* 自动切换通道 */
	uint32_t retry_count;                        /* 重试次数 */
	uint32_t health_check_interval;              /* 健康检查间隔(ms) */
} pconfig_bmc_config_t;

/**
 * @brief BMC外设配置条目
 */
typedef struct {
	const char *description;            /* 描述信息 */
	bool enabled;                       /* 是否启用 */
	pconfig_bmc_config_t config;        /* BMC配置 */
} pconfig_bmc_entry_t;

#endif /* PCONFIG_BMC_H */
