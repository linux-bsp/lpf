# SPDX-License-Identifier: GPL-2.0

obj-$(CONFIG_PDM_CONFIGS) += pdm_configs.o

pdm_configs-y += pdm-configs/core/pdm_config_api.o
pdm_configs-y += pdm-configs/core/pdm_config_backend.o
pdm_configs-y += pdm-configs/core/pdm_config_normalizer.o
pdm_configs-y += pdm-configs/core/pdm_config_validator.o
pdm_configs-y += pdm-configs/parser/static/pdm_config_static_backend.o
pdm_configs-y += pdm-configs/parser/static/pdm_config_static_start.o
pdm_configs-y += pdm-configs/parser/static/pdm_config_configs.o
pdm_configs-y += pdm-configs/parser/static/pdm_config_static_end.o
pdm_configs-$(CONFIG_OF) += pdm-configs/parser/dt/pdm_config_dt_backend.o
pdm_configs-$(CONFIG_OF) += pdm-configs/parser/dt/pdm_config_dt_parser.o
pdm_configs-$(CONFIG_PDM_CONFIG_OF_SELFTEST) += pdm-configs/selftest/pdm_config_of_selftest.o
pdm_configs-$(CONFIG_PDM_CONFIG_IMX6ULL_MODULES) += pdm-configs/products/imx6ull/modules/pdm_config_imx6ull_modules_v1.o
pdm_configs-$(CONFIG_PDM_CONFIG_UBUNTU_X86_MODULES) += pdm-configs/products/ubuntu/x86_modules/pdm_config_ubuntu_x86_modules_v1.o
pdm_configs-$(CONFIG_PDM_CONFIG_UBUNTU_X86_MOCK_MODULES) += pdm-configs/products/ubuntu/x86_mock_modules/pdm_config_ubuntu_x86_mock_modules_v1.o
