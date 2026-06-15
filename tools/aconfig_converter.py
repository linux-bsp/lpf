#!/usr/bin/env python3
"""
AConfig 配置转换工具

功能：将旧格式的 AConfig 配置转换为新格式

用法：
    python3 aconfig_converter.py <input_file> <output_file>

示例：
    python3 aconfig_converter.py ccm_aconfig_config.c h200_aconfig_optimized.c
"""

import re
import sys
from typing import List, Dict, Tuple

class AConfigConverter:
    """AConfig 配置格式转换器"""

    def __init__(self):
        self.tc_configs = []
        self.tm_configs = []
        self.invalidation_maps = []
        self.device_name_to_ref = {}

    def parse_legacy_tc_config(self, content: str) -> List[Dict]:
        """解析旧格式的遥控配置"""
        tc_entries = []

        # 匹配 TC 配置条目
        pattern = r'\[(\w+)\]\s*=\s*\{([^}]+)\}'
        matches = re.finditer(pattern, content, re.MULTILINE | re.DOTALL)

        for match in matches:
            func_id = match.group(1)
            config_body = match.group(2)

            # 解析配置字段
            entry = {
                'function_id': func_id,
                'device_type': self._extract_field(config_body, 'device_type'),
                'device_name': self._extract_field(config_body, 'device_name'),
                'enabled': self._extract_field(config_body, 'enabled', default='true')
            }

            tc_entries.append(entry)

        return tc_entries

    def parse_legacy_tm_config(self, content: str) -> List[Dict]:
        """解析旧格式的遥测配置"""
        tm_entries = []

        # 匹配 TM 配置条目
        pattern = r'\[(\w+)\]\s*=\s*\{([^}]+)\}'
        matches = re.finditer(pattern, content, re.MULTILINE | re.DOTALL)

        for match in matches:
            func_id = match.group(1)
            config_body = match.group(2)

            # 解析配置字段
            entry = {
                'function_id': func_id,
                'device_type': self._extract_field(config_body, 'device_type'),
                'device_name': self._extract_field(config_body, 'device_name'),
                'poll_period': self._extract_field(config_body, 'background_update_period_ms', default='1000'),
                'validity_period': self._extract_field(config_body, 'data_validity_ms', default='2000'),
                'enabled': self._extract_field(config_body, 'enabled', default='true')
            }

            tm_entries.append(entry)

        return tm_entries

    def parse_invalidation_map(self, content: str) -> List[Dict]:
        """解析失效映射"""
        inv_maps = []

        # 匹配失效映射条目
        pattern = r'\{\s*\.source_tm_id\s*=\s*(\w+),\s*\.affected_tm_ids\s*=\s*(\w+),\s*\.affected_count\s*=\s*(\d+)\s*\}'
        matches = re.finditer(pattern, content, re.MULTILINE)

        for match in matches:
            source_id = match.group(1)
            affected_array = match.group(2)

            inv_maps.append({
                'source_id': source_id,
                'affected_array': affected_array
            })

        return inv_maps

    def _extract_field(self, config_body: str, field_name: str, default: str = None) -> str:
        """从配置体中提取字段值"""
        pattern = rf'\.{field_name}\s*=\s*([^,\n]+)'
        match = re.search(pattern, config_body)
        if match:
            return match.group(1).strip()
        return default

    def map_device_name_to_ref(self, device_name: str, device_type: str) -> Tuple[str, int]:
        """将设备名称映射到设备引用"""
        # 简化映射：根据设备类型分配索引
        device_name = device_name.strip('"')

        if device_name not in self.device_name_to_ref:
            # 统计同类型设备数量作为索引
            type_count = sum(1 for ref in self.device_name_to_ref.values() if ref[0] == device_type)
            self.device_name_to_ref[device_name] = (device_type, type_count)

        return self.device_name_to_ref[device_name]

    def generate_new_format(self, tc_entries: List[Dict], tm_entries: List[Dict],
                           inv_maps: List[Dict]) -> str:
        """生成新格式的配置文件"""

        output = []

        # 文件头
        output.append('''/**
 * @file aconfig_optimized.c
 * @brief AConfig 优化格式配置（自动转换生成）
 * @note 本文件由 aconfig_converter.py 自动生成
 */

#include "osal.h"
#include "aconfig.h"
#include "ccm_tc_functions.h"
#include "ccm_tm_functions.h"

/*===========================================================================
 * 失效映射定义（内嵌方式）
 *===========================================================================*/
''')

        # 生成失效映射数组
        for inv_map in inv_maps:
            source_id = inv_map['source_id'].replace('ACONFIG_', 'CCM_')
            affected_array = inv_map['affected_array']

            output.append(f'''static const uint32_t {affected_array}[] = {{
    /* 需要手动填充受影响的 TM ID */
}};
''')

        output.append('''
/*===========================================================================
 * 遥控配置（稀疏数组）
 *===========================================================================*/

static const aconfig_tc_entry_t g_tc_entries[] = {
''')

        # 生成 TC 配置
        for entry in tc_entries:
            func_id = entry['function_id'].replace('ACONFIG_', 'CCM_')
            device_type = entry['device_type']
            device_name = entry['device_name']
            enabled = entry['enabled']

            # 映射设备引用
            dev_type, dev_index = self.map_device_name_to_ref(device_name, device_type)

            output.append(f'''    {{
        .function_id = {func_id},
        .config = {{
            .function_id = {func_id},
            .device = {{.type = {dev_type}, .index = {dev_index}}},
            .invalidated_tm_ids = NULL,  /* 需要手动关联失效映射 */
            .invalidated_tm_count = 0,
            .enabled = {enabled},
            .user_context = NULL
        }}
    }},
''')

        output.append('''};

/*===========================================================================
 * 遥测配置（稀疏数组）
 *===========================================================================*/

static const aconfig_tm_entry_t g_tm_entries[] = {
''')

        # 生成 TM 配置
        for entry in tm_entries:
            func_id = entry['function_id'].replace('ACONFIG_', 'CCM_')
            device_type = entry['device_type']
            device_name = entry['device_name']
            poll_period = entry['poll_period']
            validity_period = entry['validity_period']
            enabled = entry['enabled']

            # 映射设备引用
            dev_type, dev_index = self.map_device_name_to_ref(device_name, device_type)

            output.append(f'''    {{
        .function_id = {func_id},
        .config = {{
            .function_id = {func_id},
            .device = {{.type = {dev_type}, .index = {dev_index}}},
            .poll_period_ms = {poll_period},
            .validity_period_ms = {validity_period},
            .enabled = {enabled},
            .user_context = NULL
        }}
    }},
''')

        output.append('''};

/*===========================================================================
 * 配置表（优化版）
 *===========================================================================*/

const aconfig_config_table_t g_aconfig_optimized = {
    .name = "Optimized_Config",
    .hwid_count = 0,
    .hwid_list = NULL,
    .tc_entries = g_tc_entries,
    .tc_count = sizeof(g_tc_entries) / sizeof(aconfig_tc_entry_t),
    .tm_entries = g_tm_entries,
    .tm_count = sizeof(g_tm_entries) / sizeof(aconfig_tm_entry_t)
};

/**
 * @brief 初始化 AConfig（优化版）
 * @return OSAL_SUCCESS 成功，其他值失败
 */
int32_t AConfig_Init_Optimized(void)
{
    int32_t ret;

    ret = ACONFIG_Init();
    if (OSAL_SUCCESS != ret) {
        return ret;
    }

    ret = ACONFIG_RegisterTable(&g_aconfig_optimized);
    if (OSAL_SUCCESS != ret) {
        return ret;
    }

    LOG_INFO("ACONFIG", "Initialized (optimized format)");
    return OSAL_SUCCESS;
}
''')

        return ''.join(output)

    def convert(self, input_file: str, output_file: str):
        """执行转换"""
        print(f"正在转换：{input_file} -> {output_file}")

        # 读取输入文件
        with open(input_file, 'r', encoding='utf-8') as f:
            content = f.read()

        # 解析配置
        print("解析 TC 配置...")
        tc_entries = self.parse_legacy_tc_config(content)
        print(f"  找到 {len(tc_entries)} 个 TC 配置")

        print("解析 TM 配置...")
        tm_entries = self.parse_legacy_tm_config(content)
        print(f"  找到 {len(tm_entries)} 个 TM 配置")

        print("解析失效映射...")
        inv_maps = self.parse_invalidation_map(content)
        print(f"  找到 {len(inv_maps)} 个失效映射")

        # 生成新格式
        print("生成新格式配置...")
        new_content = self.generate_new_format(tc_entries, tm_entries, inv_maps)

        # 写入输出文件
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(new_content)

        print(f"✅ 转换完成！")
        print(f"\n⚠️  注意事项：")
        print(f"  1. 请检查设备索引映射是否正确")
        print(f"  2. 请手动关联失效映射到 TC 配置")
        print(f"  3. 请验证生成的配置并测试")
        print(f"\n设备映射表：")
        for name, (dev_type, index) in self.device_name_to_ref.items():
            print(f"  {name:20} -> ({dev_type}, {index})")


def main():
    """主函数"""
    if len(sys.argv) != 3:
        print("用法: python3 aconfig_converter.py <input_file> <output_file>")
        print("\n示例:")
        print("  python3 aconfig_converter.py ccm_aconfig_config.c h200_aconfig_new.c")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    converter = AConfigConverter()
    try:
        converter.convert(input_file, output_file)
    except Exception as e:
        print(f"❌ 转换失败: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
