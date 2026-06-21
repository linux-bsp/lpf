# LPF Core Config Note

Configuration engine implementation no longer lives under `lpf-core/`.

Use [`kernel/lpf-configs/README.md`](/home/wanguo/imx6ull/LPF/kernel/lpf-configs/README.md)
for the current `lpf_configs.ko` layout and responsibilities.

`lpf_core.ko` consumes the public `lpf_config_*` API but does not own config
backend, parsing, validation, or static configuration data implementation.
