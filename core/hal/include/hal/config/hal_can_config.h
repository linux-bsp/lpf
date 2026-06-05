/************************************************************************
 * CAN硬件配置
 ************************************************************************/

#ifndef HAL_CAN_CONFIG_H
#define HAL_CAN_CONFIG_H

/* CAN接口 */
#define HAL_CAN_INTERFACE           "can0"

/* CAN波特率 */
#define HAL_CAN_BAUDRATE            0x7A120  /* 500Kbps */

/* CAN接收队列深度 */
#define HAL_CAN_RX_QUEUE_DEPTH      0x64

/* CAN发送队列深度 */
#define HAL_CAN_TX_QUEUE_DEPTH      0x64

/* CAN消息最大长度 */
#define HAL_CAN_MAX_DATA_LEN        0x08

/* CAN超时时间(ms) */
#define HAL_CAN_TIMEOUT_MS          0x3E8

#endif /* HAL_CAN_CONFIG_H */
