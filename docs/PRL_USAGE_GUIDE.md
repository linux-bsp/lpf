# PRL 使用指南

本文档提供 PRL（Protocol Layer）协议层的完整使用示例。

## 1. 基本使用

### 1.1 包含头文件

```c
#include "prl_api.h"  /* PRL 对外 API */
```

### 1.2 初始化（可选）

```c
int main(void)
{
    /* 初始化 PRL（可选，主要用于初始化全局状态） */
    PRL_Init();
    
    /* 应用程序逻辑 */
    
    /* 清理 */
    PRL_Deinit();
    
    return 0;
}
```

## 2. 编码示例

### 2.1 编码简单消息

```c
void example_encode_simple(void)
{
    /* 准备负载数据 */
    prl_mcu_version_t version = {
        .major = 1,
        .minor = 2,
        .patch = 3,
        .reserved = 0,
        .build_time = 1234567890,
        .git_hash = "abcd1234",
    };
    
    /* 准备缓冲区 */
    uint8_t buffer[PRL_MAX_PACKET_SIZE];
    
    /* 编码 */
    int len = PRL_Encode(
        PRL_DEV_TYPE_MCU,           /* 设备类型：MCU */
        PRL_MCU_MSG_GET_VERSION,    /* 消息类型：获取版本 */
        &version,                   /* 负载数据 */
        sizeof(version),            /* 负载长度 */
        buffer,                     /* 输出缓冲区 */
        sizeof(buffer),             /* 缓冲区大小 */
        0                           /* 标志位：无 */
    );
    
    if (len > 0) {
        /* 编码成功，发送数据 */
        printf("Encoded %d bytes\n", len);
        /* 通过 HAL 层发送：HAL_CAN_Send(handle, buffer, len); */
    } else {
        /* 编码失败 */
        printf("Encode failed: %s\n", PRL_GetErrorString(len));
    }
}
```

### 2.2 编码需要应答的消息

```c
void example_encode_with_ack(void)
{
    uint8_t buffer[PRL_MAX_PACKET_SIZE];
    
    /* 编码时设置 PRL_FLAG_ACK_REQUIRED 标志 */
    int len = PRL_Encode(
        PRL_DEV_TYPE_MCU,
        PRL_MCU_MSG_RESET,
        NULL,                       /* 无负载 */
        0,
        buffer,
        sizeof(buffer),
        PRL_FLAG_ACK_REQUIRED       /* 需要应答 */
    );
    
    if (len > 0) {
        /* 发送并等待应答 */
        send_and_wait_ack(buffer, len);
    }
}
```

### 2.3 编码空负载消息

```c
void example_encode_empty_payload(void)
{
    uint8_t buffer[PRL_MAX_PACKET_SIZE];
    
    /* 心跳消息通常没有负载 */
    int len = PRL_Encode(
        PRL_DEV_TYPE_MCU,
        PRL_MCU_MSG_HEARTBEAT,
        NULL,                       /* 无负载 */
        0,                          /* 长度为 0 */
        buffer,
        sizeof(buffer),
        0
    );
    
    if (len > 0) {
        /* 发送心跳 */
        send_heartbeat(buffer, len);
    }
}
```

## 3. 解码示例

### 3.1 解码基本流程

```c
void example_decode_basic(const uint8_t *packet, size_t packet_len)
{
    uint8_t dev_type, msg_type;
    const uint8_t *payload;
    uint16_t payload_len;
    
    /* 解码 */
    int ret = PRL_Decode(
        packet,         /* 报文数据 */
        packet_len,     /* 报文长度 */
        &dev_type,      /* 输出：设备类型 */
        &msg_type,      /* 输出：消息类型 */
        &payload,       /* 输出：负载指针（零拷贝） */
        &payload_len    /* 输出：负载长度 */
    );
    
    if (ret == PRL_OK) {
        /* 解码成功，根据设备类型和消息类型处理 */
        printf("Device: %s, Message: 0x%02X, Payload: %u bytes\n",
               PRL_GetDeviceTypeName(dev_type), msg_type, payload_len);
        
        /* 处理消息 */
        handle_message(dev_type, msg_type, payload, payload_len);
    } else {
        /* 解码失败 */
        printf("Decode failed: %s\n", PRL_GetErrorString(ret));
    }
}
```

### 3.2 按设备类型分发

```c
void example_decode_dispatch(const uint8_t *packet, size_t packet_len)
{
    uint8_t dev_type, msg_type;
    const uint8_t *payload;
    uint16_t payload_len;
    
    int ret = PRL_Decode(packet, packet_len,
                         &dev_type, &msg_type,
                         &payload, &payload_len);
    
    if (ret != PRL_OK) {
        return;
    }
    
    /* 根据设备类型分发 */
    switch (dev_type) {
        case PRL_DEV_TYPE_MCU:
            handle_mcu_message(msg_type, payload, payload_len);
            break;
            
        case PRL_DEV_TYPE_CCM:
            handle_ccm_message(msg_type, payload, payload_len);
            break;
            
        case PRL_DEV_TYPE_PMC:
            handle_pmc_message(msg_type, payload, payload_len);
            break;
            
        case PRL_DEV_TYPE_POWER:
            handle_power_message(msg_type, payload, payload_len);
            break;
            
        default:
            printf("Unknown device type: 0x%02X\n", dev_type);
            break;
    }
}
```

### 3.3 处理 MCU 消息

```c
void handle_mcu_message(uint8_t msg_type, const uint8_t *payload, uint16_t payload_len)
{
    switch (msg_type) {
        case PRL_MCU_MSG_GET_VERSION: {
            if (payload_len == sizeof(prl_mcu_version_t)) {
                const prl_mcu_version_t *version = (const prl_mcu_version_t *)payload;
                printf("MCU Version: %u.%u.%u\n",
                       version->major, version->minor, version->patch);
            }
            break;
        }
        
        case PRL_MCU_MSG_GET_STATUS: {
            if (payload_len == sizeof(prl_mcu_status_t)) {
                const prl_mcu_status_t *status = (const prl_mcu_status_t *)payload;
                printf("MCU Status: state=%u, error=%u, uptime=%u\n",
                       status->state, status->error_code, status->uptime);
            }
            break;
        }
        
        case PRL_MCU_MSG_HEARTBEAT:
            printf("MCU Heartbeat received\n");
            break;
            
        default:
            printf("Unknown MCU message: 0x%02X\n", msg_type);
            break;
    }
}
```

## 4. 应答处理

### 4.1 构建应答报文

```c
void example_build_response(const uint8_t *request, size_t request_len)
{
    /* 准备应答负载 */
    prl_mcu_status_t status = {
        .state = 1,
        .error_code = 0,
        .uptime = 3600,
        .cpu_usage = 500,   /* 50.0% */
        .mem_usage = 300,   /* 30.0% */
    };
    
    /* 构建应答 */
    uint8_t response[PRL_MAX_PACKET_SIZE];
    int len = PRL_BuildResponse(
        request,            /* 请求报文 */
        request_len,        /* 请求长度 */
        &status,            /* 应答负载 */
        sizeof(status),     /* 应答长度 */
        response,           /* 输出缓冲区 */
        sizeof(response)    /* 缓冲区大小 */
    );
    
    if (len > 0) {
        /* 发送应答 */
        send_response(response, len);
    }
}
```

### 4.2 检查是否需要应答

```c
void example_check_ack_required(const uint8_t *packet, size_t packet_len)
{
    if (packet_len < PRL_HEADER_SIZE) {
        return;
    }
    
    const prl_header_t *hdr = (const prl_header_t *)packet;
    
    /* 检查是否需要应答 */
    if (hdr->flags & PRL_FLAG_ACK_REQUIRED) {
        printf("ACK required, seq=%u\n", hdr->seq);
        
        /* 构建并发送应答 */
        build_and_send_ack(packet, packet_len);
    }
    
    /* 检查是否是应答报文 */
    if (hdr->flags & PRL_FLAG_IS_ACK) {
        printf("This is an ACK packet, seq=%u\n", hdr->seq);
    }
}
```

## 5. 在 PDL 中使用

### 5.1 PDL_MCU 实现示例

```c
/* PDL_MCU 内部实现 */
int32_t PDL_MCU_GetVersion(pdl_mcu_handle_t handle, pdl_mcu_version_t *version)
{
    uint8_t tx_buf[PRL_MAX_PACKET_SIZE];
    uint8_t rx_buf[PRL_MAX_PACKET_SIZE];
    size_t rx_len;
    int ret;
    
    /* 1. 使用 PRL 编码请求 */
    int len = PRL_Encode(
        PRL_DEV_TYPE_MCU,
        PRL_MCU_MSG_GET_VERSION,
        NULL, 0,
        tx_buf, sizeof(tx_buf),
        PRL_FLAG_ACK_REQUIRED
    );
    
    if (len < 0) {
        LOG_ERROR("PDL_MCU", "PRL_Encode failed: %s", PRL_GetErrorString(len));
        return OSAL_ERR_GENERIC;
    }
    
    /* 2. 通过 HAL 发送 */
    ret = HAL_CAN_Send(handle->can_handle, tx_buf, len);
    if (ret != OSAL_SUCCESS) {
        LOG_ERROR("PDL_MCU", "HAL_CAN_Send failed: %d", ret);
        return ret;
    }
    
    /* 3. 通过 HAL 接收（带超时） */
    ret = HAL_CAN_Recv(handle->can_handle, rx_buf, &rx_len, 1000);
    if (ret != OSAL_SUCCESS) {
        LOG_ERROR("PDL_MCU", "HAL_CAN_Recv failed: %d", ret);
        return ret;
    }
    
    /* 4. 使用 PRL 解码响应 */
    uint8_t dev_type, msg_type;
    const uint8_t *payload;
    uint16_t payload_len;
    
    ret = PRL_Decode(rx_buf, rx_len,
                     &dev_type, &msg_type,
                     &payload, &payload_len);
    
    if (ret != PRL_OK) {
        LOG_ERROR("PDL_MCU", "PRL_Decode failed: %s", PRL_GetErrorString(ret));
        return OSAL_ERR_GENERIC;
    }
    
    /* 5. 验证响应 */
    if (dev_type != PRL_DEV_TYPE_MCU) {
        LOG_ERROR("PDL_MCU", "Invalid device type: %u", dev_type);
        return OSAL_ERR_GENERIC;
    }
    
    if (msg_type != PRL_MCU_MSG_GET_VERSION) {
        LOG_ERROR("PDL_MCU", "Invalid message type: %u", msg_type);
        return OSAL_ERR_GENERIC;
    }
    
    if (payload_len != sizeof(prl_mcu_version_t)) {
        LOG_ERROR("PDL_MCU", "Invalid payload length: %u", payload_len);
        return OSAL_ERR_GENERIC;
    }
    
    /* 6. 复制数据到输出参数 */
    const prl_mcu_version_t *prl_version = (const prl_mcu_version_t *)payload;
    
    version->major = prl_version->major;
    version->minor = prl_version->minor;
    version->patch = prl_version->patch;
    version->build = prl_version->reserved;
    snprintf(version->version_string, sizeof(version->version_string),
             "%u.%u.%u", prl_version->major, prl_version->minor, prl_version->patch);
    
    return OSAL_SUCCESS;
}
```

### 5.2 PDL_MCU 发送命令示例

```c
int32_t PDL_MCU_Reset(pdl_mcu_handle_t handle)
{
    uint8_t tx_buf[PRL_MAX_PACKET_SIZE];
    int ret;
    
    /* 编码复位命令（无负载） */
    int len = PRL_Encode(
        PRL_DEV_TYPE_MCU,
        PRL_MCU_MSG_RESET,
        NULL, 0,
        tx_buf, sizeof(tx_buf),
        0  /* 不需要应答 */
    );
    
    if (len < 0) {
        return OSAL_ERR_GENERIC;
    }
    
    /* 发送命令 */
    ret = HAL_CAN_Send(handle->can_handle, tx_buf, len);
    if (ret != OSAL_SUCCESS) {
        return ret;
    }
    
    LOG_INFO("PDL_MCU", "Reset command sent");
    return OSAL_SUCCESS;
}
```

## 6. 工具函数使用

### 6.1 快速验证报文

```c
void example_validate_packet(const uint8_t *packet, size_t packet_len)
{
    /* 快速验证报文（不解析负载） */
    int ret = PRL_ValidatePacket(packet, packet_len);
    
    if (ret == PRL_OK) {
        printf("Packet is valid\n");
    } else {
        printf("Packet validation failed: %s\n", PRL_GetErrorString(ret));
    }
}
```

### 6.2 快速获取设备类型

```c
void example_get_device_type(const uint8_t *packet, size_t packet_len)
{
    uint8_t dev_type;
    
    /* 快速获取设备类型（不验证 CRC） */
    int ret = PRL_GetDeviceType(packet, packet_len, &dev_type);
    
    if (ret == PRL_OK) {
        printf("Device type: %s\n", PRL_GetDeviceTypeName(dev_type));
        
        /* 根据设备类型路由到不同的处理线程 */
        route_to_handler(dev_type, packet, packet_len);
    }
}
```

### 6.3 序列号去重

```c
/* 序列号缓存（用于去重） */
#define SEQ_CACHE_SIZE 256
static uint32_t g_seq_cache[SEQ_CACHE_SIZE];
static size_t g_seq_cache_index = 0;

bool is_duplicate_packet(const uint8_t *packet, size_t packet_len)
{
    uint32_t seq;
    
    /* 获取序列号 */
    int ret = PRL_GetSequence(packet, packet_len, &seq);
    if (ret != PRL_OK) {
        return false;
    }
    
    /* 检查是否重复 */
    for (size_t i = 0; i < SEQ_CACHE_SIZE; i++) {
        if (g_seq_cache[i] == seq) {
            return true;  /* 重复报文 */
        }
    }
    
    /* 添加到缓存 */
    g_seq_cache[g_seq_cache_index] = seq;
    g_seq_cache_index = (g_seq_cache_index + 1) % SEQ_CACHE_SIZE;
    
    return false;
}
```

## 7. 错误处理

### 7.1 完整的错误处理示例

```c
void example_error_handling(void)
{
    uint8_t buffer[PRL_MAX_PACKET_SIZE];
    prl_mcu_version_t version = {1, 2, 3};
    
    int len = PRL_Encode(
        PRL_DEV_TYPE_MCU,
        PRL_MCU_MSG_GET_VERSION,
        &version, sizeof(version),
        buffer, sizeof(buffer),
        0
    );
    
    if (len < 0) {
        /* 编码失败，根据错误码处理 */
        switch (len) {
            case PRL_ERR_INVALID_PARAM:
                LOG_ERROR("PRL", "Invalid parameter");
                break;
                
            case PRL_ERR_INVALID_DEV_TYPE:
                LOG_ERROR("PRL", "Invalid device type");
                break;
                
            case PRL_ERR_BUFFER_TOO_SMALL:
                LOG_ERROR("PRL", "Buffer too small");
                break;
                
            default:
                LOG_ERROR("PRL", "Encode failed: %s", PRL_GetErrorString(len));
                break;
        }
        return;
    }
    
    /* 编码成功，继续处理 */
    printf("Encoded %d bytes\n", len);
}
```

## 8. 性能优化

### 8.1 零拷贝解码

```c
void example_zero_copy(const uint8_t *packet, size_t packet_len)
{
    uint8_t dev_type, msg_type;
    const uint8_t *payload;  /* 指向 packet 内部，零拷贝 */
    uint16_t payload_len;
    
    int ret = PRL_Decode(packet, packet_len,
                         &dev_type, &msg_type,
                         &payload, &payload_len);
    
    if (ret == PRL_OK) {
        /* 直接使用 payload，无需额外分配内存 */
        const prl_mcu_status_t *status = (const prl_mcu_status_t *)payload;
        
        /* 注意：payload 的生命周期与 packet 相同 */
        process_status(status);
    }
}
```

### 8.2 批量编码

```c
void example_batch_encode(void)
{
    uint8_t buffer[PRL_MAX_PACKET_SIZE * 10];
    size_t offset = 0;
    
    /* 批量编码多个消息 */
    for (int i = 0; i < 10; i++) {
        prl_mcu_status_t status = {i, 0, i * 100, 0, 0};
        
        int len = PRL_Encode(
            PRL_DEV_TYPE_MCU,
            PRL_MCU_MSG_GET_STATUS,
            &status, sizeof(status),
            buffer + offset,
            sizeof(buffer) - offset,
            0
        );
        
        if (len > 0) {
            offset += len;
        }
    }
    
    /* 批量发送 */
    send_batch(buffer, offset);
}
```

## 9. 调试技巧

### 9.1 打印报文内容

```c
void print_packet(const uint8_t *packet, size_t packet_len)
{
    if (packet_len < PRL_HEADER_SIZE) {
        printf("Invalid packet (too short)\n");
        return;
    }
    
    const prl_header_t *hdr = (const prl_header_t *)packet;
    
    printf("=== PRL Packet ===\n");
    printf("Magic:     0x%04X\n", hdr->magic);
    printf("Version:   %u.%u\n", hdr->version >> 4, hdr->version & 0x0F);
    printf("Device:    %s (0x%02X)\n", PRL_GetDeviceTypeName(hdr->dev_type), hdr->dev_type);
    printf("Message:   0x%02X\n", hdr->msg_type);
    printf("Flags:     0x%02X", hdr->flags);
    if (hdr->flags & PRL_FLAG_ACK_REQUIRED) printf(" [ACK_REQ]");
    if (hdr->flags & PRL_FLAG_IS_ACK) printf(" [IS_ACK]");
    printf("\n");
    printf("Length:    %u bytes\n", hdr->length);
    printf("Sequence:  %u\n", hdr->seq);
    printf("Timestamp: %u\n", hdr->timestamp);
    printf("CRC16:     0x%04X\n", hdr->crc16);
    printf("==================\n");
}
```

### 9.2 性能测试

```c
void benchmark_encode_decode(void)
{
    uint8_t buffer[PRL_MAX_PACKET_SIZE];
    prl_mcu_version_t version = {1, 2, 3};
    
    uint64_t start = get_time_us();
    
    /* 编码 10000 次 */
    for (int i = 0; i < 10000; i++) {
        PRL_Encode(PRL_DEV_TYPE_MCU, PRL_MCU_MSG_GET_VERSION,
                   &version, sizeof(version),
                   buffer, sizeof(buffer), 0);
    }
    
    uint64_t encode_time = get_time_us() - start;
    
    start = get_time_us();
    
    /* 解码 10000 次 */
    for (int i = 0; i < 10000; i++) {
        uint8_t dev_type, msg_type;
        const uint8_t *payload;
        uint16_t payload_len;
        
        PRL_Decode(buffer, PRL_HEADER_SIZE + sizeof(version),
                   &dev_type, &msg_type, &payload, &payload_len);
    }
    
    uint64_t decode_time = get_time_us() - start;
    
    printf("Encode: %.2f us/packet\n", encode_time / 10000.0);
    printf("Decode: %.2f us/packet\n", decode_time / 10000.0);
}
```

---

**文档版本**：v1.0  
**最后更新**：2026-06-01  
**维护者**：wanguo
