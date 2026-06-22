// SPDX-License-Identifier: GPL-2.0
/**
 * @file pdm_mcu_driver.c
 * @brief PDM MCU bus driver with generic, UART and CAN transports
 */

#include <linux/atomic.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#include "../pdm_peripheral.h"
#include "pdm_mcu_internal.h"

#include "pdm/core/pdm_bus.h"
#include "pdm/pdm_ctl.h"
#include "osal.h"

static atomic_t pdm_mcu_device_count = ATOMIC_INIT(0);

static int pdm_mcu_memory_setup(struct pdm_mcu_instance *inst)
{
	(void)inst;
	return 0;
}

static void pdm_mcu_memory_cleanup(struct pdm_mcu_instance *inst)
{
	(void)inst;
}

static int pdm_mcu_memory_reset(struct pdm_mcu_instance *inst)
{
	(void)inst;
	return 0;
}

static int pdm_mcu_memory_command(struct pdm_mcu_instance *inst,
				  struct pdm_mcu_command *command)
{
	(void)inst;
	(void)command;
	return -EOPNOTSUPP;
}

static int pdm_mcu_memory_read_data(struct pdm_mcu_instance *inst,
				    struct pdm_mcu_data *data)
{
	(void)inst;
	(void)data;
	return -EOPNOTSUPP;
}

static int pdm_mcu_memory_write_data(struct pdm_mcu_instance *inst,
				     const struct pdm_mcu_data *data)
{
	(void)inst;
	(void)data;
	return -EOPNOTSUPP;
}

static const struct pdm_mcu_transport_ops pdm_mcu_memory_ops = {
	.type = PDM_MCU_BACKEND_MEMORY,
	.name = "memory",
	.capability = PDM_CTL_DEVICE_CAP_NONE,
	.setup = pdm_mcu_memory_setup,
	.cleanup = pdm_mcu_memory_cleanup,
	.reset = pdm_mcu_memory_reset,
	.command = pdm_mcu_memory_command,
	.read_data = pdm_mcu_memory_read_data,
	.write_data = pdm_mcu_memory_write_data,
};

const struct pdm_mcu_transport_ops *pdm_mcu_transport_select(const char *compatible)
{
	if (!compatible)
		return &pdm_mcu_memory_ops;

	if (!strcmp(compatible, "pdm,mcu-uart") ||
	    !strcmp(compatible, "vendor,pdm-mcu-uart"))
		return &pdm_mcu_uart_ops;

	if (!strcmp(compatible, "pdm,mcu-can") ||
	    !strcmp(compatible, "vendor,pdm-mcu-can"))
		return &pdm_mcu_can_ops;

	return &pdm_mcu_memory_ops;
}

static int pdm_mcu_record_result(struct pdm_mcu_instance *inst, int ret)
{
	if (ret < 0) {
		inst->last_error = ret;
		inst->state = inst->online ? PDM_MCU_STATE_ERROR : PDM_MCU_STATE_OFFLINE;
		inst->pdm_dev->last_error = ret;
		inst->pdm_dev->error_count++;
		if (inst->online)
			inst->pdm_dev->state = PDM_CTL_DEVICE_STATE_ERROR;
		return ret;
	}

	inst->last_error = 0;
	inst->state = inst->online ? PDM_MCU_STATE_READY : PDM_MCU_STATE_OFFLINE;
	inst->pdm_dev->last_error = 0;
	if (inst->online)
		inst->pdm_dev->state = PDM_CTL_DEVICE_STATE_BOUND;
	return ret;
}

static long pdm_mcu_get_info(struct pdm_client *client, unsigned long arg)
{
	struct pdm_mcu_info info = {
		.abi_version = PDM_MCU_ABI_VERSION,
		.module_version_major = 1,
		.module_version_minor = 0,
		.module_version_patch = 0,
		.open_count = pdm_client_open_count(client),
		.max_devices = (u32)atomic_read(&pdm_mcu_device_count),
	};

	if (copy_to_user((void __user *)arg, &info, sizeof(info)))
		return -EFAULT;
	return 0;
}

static long pdm_mcu_get_version(struct pdm_mcu_instance *inst, unsigned long arg)
{
	struct pdm_mcu_version version;

	if (copy_from_user(&version, (void __user *)arg, sizeof(version)))
		return -EFAULT;

	version.major = 1;
	version.minor = 0;
	version.patch = 0;
	version.build = 0;
	snprintf(version.version_string, sizeof(version.version_string),
		 "pdm-mcu-%s", inst->ops ? inst->ops->name : "unknown");

	if (copy_to_user((void __user *)arg, &version, sizeof(version)))
		return -EFAULT;
	return 0;
}

static long pdm_mcu_get_status(struct pdm_mcu_instance *inst, unsigned long arg)
{
	struct pdm_mcu_status status;
	u64 elapsed_ns;
	int ret = 0;

	if (copy_from_user(&status, (void __user *)arg, sizeof(status)))
		return -EFAULT;

	mutex_lock(&inst->lock);
	elapsed_ns = ktime_to_ns(ktime_sub(ktime_get(), inst->start_time));
	status.online = inst->online ? 1U : 0U;
	status.state = inst->state;
	status.uptime_sec = (u32)(elapsed_ns / NSEC_PER_SEC);
	status.error_code = inst->last_error < 0 ? (u32)(-inst->last_error) : 0U;
	status.temperature_milli_celsius = 0;
	status.voltage_mv = 0;
	status.timestamp_us = ktime_to_us(ktime_get());
	if (!inst->online)
		ret = -ENODEV;
	mutex_unlock(&inst->lock);

	if (copy_to_user((void __user *)arg, &status, sizeof(status)))
		return -EFAULT;
	return ret;
}

static int pdm_mcu_require_online_locked(struct pdm_mcu_instance *inst)
{
	if (!inst->online)
		return -ENODEV;
	if (!inst->ops)
		return -EOPNOTSUPP;

	return 0;
}

static long pdm_mcu_reset(struct pdm_mcu_instance *inst, unsigned long arg)
{
	u32 index;
	int ret;

	if (copy_from_user(&index, (void __user *)arg, sizeof(index)))
		return -EFAULT;

	mutex_lock(&inst->lock);
	ret = pdm_mcu_require_online_locked(inst);
	if (!ret)
		ret = inst->ops->reset(inst);
	ret = pdm_mcu_record_result(inst, ret);
	mutex_unlock(&inst->lock);
	return ret;
}

static long pdm_mcu_command_ioctl(struct pdm_mcu_instance *inst, unsigned long arg)
{
	struct pdm_mcu_command command;
	int ret;

	if (copy_from_user(&command, (void __user *)arg, sizeof(command)))
		return -EFAULT;

	mutex_lock(&inst->lock);
	ret = pdm_mcu_require_online_locked(inst);
	if (!ret)
		ret = inst->ops->command(inst, &command);
	ret = pdm_mcu_record_result(inst, ret);
	mutex_unlock(&inst->lock);
	if (ret)
		return ret;

	if (copy_to_user((void __user *)arg, &command, sizeof(command)))
		return -EFAULT;
	return 0;
}

static long pdm_mcu_read_data_ioctl(struct pdm_mcu_instance *inst, unsigned long arg)
{
	struct pdm_mcu_data data;
	int ret;

	if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
		return -EFAULT;

	mutex_lock(&inst->lock);
	ret = pdm_mcu_require_online_locked(inst);
	if (!ret)
		ret = inst->ops->read_data(inst, &data);
	ret = pdm_mcu_record_result(inst, ret);
	mutex_unlock(&inst->lock);
	if (ret)
		return ret;

	if (copy_to_user((void __user *)arg, &data, sizeof(data)))
		return -EFAULT;
	return 0;
}

static long pdm_mcu_write_data_ioctl(struct pdm_mcu_instance *inst, unsigned long arg)
{
	struct pdm_mcu_data data;
	int ret;

	if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
		return -EFAULT;

	mutex_lock(&inst->lock);
	ret = pdm_mcu_require_online_locked(inst);
	if (!ret)
		ret = inst->ops->write_data(inst, &data);
	ret = pdm_mcu_record_result(inst, ret);
	mutex_unlock(&inst->lock);
	return ret;
}

static long pdm_mcu_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct pdm_client *client = pdm_client_from_file(filp);
	struct pdm_mcu_instance *inst;

	if (!client)
		return -ENODEV;
	inst = container_of(client, struct pdm_mcu_instance, client);

	switch (cmd) {
	case PDM_MCU_IOC_GET_INFO:
		return pdm_mcu_get_info(client, arg);
	case PDM_MCU_IOC_GET_VERSION:
		return pdm_mcu_get_version(inst, arg);
	case PDM_MCU_IOC_GET_STATUS:
		return pdm_mcu_get_status(inst, arg);
	case PDM_MCU_IOC_RESET:
		return pdm_mcu_reset(inst, arg);
	case PDM_MCU_IOC_COMMAND:
		return pdm_mcu_command_ioctl(inst, arg);
	case PDM_MCU_IOC_READ_DATA:
		return pdm_mcu_read_data_ioctl(inst, arg);
	case PDM_MCU_IOC_WRITE_DATA:
		return pdm_mcu_write_data_ioctl(inst, arg);
	default:
		return -ENOTTY;
	}
}

static const struct file_operations pdm_mcu_fops = {
	.owner = THIS_MODULE,
	.open = pdm_client_default_open,
	.release = pdm_client_default_release,
	.unlocked_ioctl = pdm_mcu_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = pdm_mcu_ioctl,
#endif
};

static void pdm_mcu_client_release(struct pdm_client *client)
{
	struct pdm_mcu_instance *inst;

	if (!client)
		return;

	inst = container_of(client, struct pdm_mcu_instance, client);
	kfree(inst);
}

static int pdm_mcu_probe(struct pdm_device *pdm_dev)
{
	struct pdm_mcu_instance *inst;
	char nodename[32];
	int ret;

	inst = kzalloc(sizeof(*inst), GFP_KERNEL);
	if (!inst)
		return -ENOMEM;

	inst->pdm_dev = pdm_dev;
	inst->ops = pdm_mcu_transport_select(pdm_dev->compatible);
	inst->start_time = ktime_get();
	inst->online = true;
	inst->state = PDM_MCU_STATE_READY;
	mutex_init(&inst->lock);

	ret = inst->ops->setup(inst);
	if (ret)
		goto err_free;

	snprintf(nodename, sizeof(nodename), "mcu%d", pdm_dev->id);
	ret = pdm_client_register(&inst->client, pdm_dev, "pdm-mcu",
				  nodename, &pdm_mcu_fops, pdm_mcu_client_release);
	if (ret)
		goto err_cleanup_transport;

	pdm_dev->capabilities |= inst->ops->capability;
	pdm_device_set_drvdata(pdm_dev, inst);
	atomic_inc(&pdm_mcu_device_count);
	LOG_INFO("PDM-MCU", "Registered MCU %s transport for %s",
		 inst->ops->name, dev_name(&pdm_dev->dev));
	return 0;

err_cleanup_transport:
	inst->ops->cleanup(inst);
err_free:
	kfree(inst);
	return ret;
}

static void pdm_mcu_remove(struct pdm_device *pdm_dev)
{
	struct pdm_mcu_instance *inst = pdm_device_get_drvdata(pdm_dev);

	if (!inst)
		return;

	atomic_dec_if_positive(&pdm_mcu_device_count);
	pdm_device_set_drvdata(pdm_dev, NULL);

	mutex_lock(&inst->lock);
	inst->online = false;
	inst->state = PDM_MCU_STATE_OFFLINE;
	if (inst->ops && inst->ops->cleanup)
		inst->ops->cleanup(inst);
	mutex_unlock(&inst->lock);

	pdm_client_unregister(&inst->client);
}

static const struct of_device_id pdm_mcu_of_match[] = {
	{ .compatible = "pdm,mcu" },
	{ .compatible = "pdm,mcu-uart" },
	{ .compatible = "pdm,mcu-can" },
	{ .compatible = "vendor,pdm-mcu" },
	{ .compatible = "vendor,pdm-mcu-uart" },
	{ .compatible = "vendor,pdm-mcu-can" },
	{ }
};
MODULE_DEVICE_TABLE(of, pdm_mcu_of_match);

static struct pdm_driver pdm_mcu_driver = {
	.driver = {
		.name = "pdm-mcu",
		.of_match_table = pdm_mcu_of_match,
	},
	.device_type = PDM_CTL_DEVICE_TYPE_MCU,
	.capabilities = PDM_CTL_DEVICE_CAP_USER_IOCTL,
	.probe = pdm_mcu_probe,
	.remove = pdm_mcu_remove,
};

int pdm_mcu_driver_init(void)
{
	return pdm_bus_register_driver(THIS_MODULE, &pdm_mcu_driver);
}

void pdm_mcu_driver_exit(void)
{
	pdm_bus_unregister_driver(&pdm_mcu_driver);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("PDM MCU bus driver");
