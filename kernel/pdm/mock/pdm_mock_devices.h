// SPDX-License-Identifier: GPL-2.0

#ifndef PDM_MOCK_DEVICES_H
#define PDM_MOCK_DEVICES_H

#ifdef CONFIG_PDM_MOCK_DEVICES
int pdm_mock_devices_init(void);
void pdm_mock_devices_exit(void);
#else
static inline int pdm_mock_devices_init(void)
{
	return 0;
}

static inline void pdm_mock_devices_exit(void)
{
}
#endif

#endif /* PDM_MOCK_DEVICES_H */
