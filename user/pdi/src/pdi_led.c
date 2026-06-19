/************************************************************************
 * PDI LED user API implementation
 ************************************************************************/

#include "pdi/pdi_led.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int32_t pdi_led_ioctl_checked(pdi_led_context_t *ctx,
				     unsigned long request, void *arg)
{
	if (ctx == NULL || ctx->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	return ioctl(ctx->fd, request, arg);
}

int32_t pdi_led_open(pdi_led_context_t *ctx, const char *device_path)
{
	const char *path;

	if (ctx == NULL) {
		errno = EINVAL;
		return -1;
	}

	path = (device_path != NULL) ? device_path : PDI_LED_DEFAULT_DEVICE;
	ctx->fd = open(path, O_RDWR | O_CLOEXEC);
	return (ctx->fd < 0) ? -1 : 0;
}

int32_t pdi_led_close(pdi_led_context_t *ctx)
{
	int ret;

	if (ctx == NULL || ctx->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	ret = close(ctx->fd);
	ctx->fd = -1;
	return ret;
}

int32_t pdi_led_get_info(pdi_led_context_t *ctx, struct pdi_led_info *info)
{
	if (info == NULL) {
		errno = EINVAL;
		return -1;
	}

	return pdi_led_ioctl_checked(ctx, PDI_LED_IOC_GET_INFO, info);
}

int32_t pdi_led_get_state(pdi_led_context_t *ctx,
			  struct pdi_led_state *state)
{
	if (state == NULL) {
		errno = EINVAL;
		return -1;
	}

	return pdi_led_ioctl_checked(ctx, PDI_LED_IOC_GET_STATE, state);
}

int32_t pdi_led_set_brightness(pdi_led_context_t *ctx, uint32_t index,
			       uint32_t brightness)
{
	struct pdi_led_brightness request = {
		.index = index,
		.brightness = brightness,
	};

	return pdi_led_ioctl_checked(ctx, PDI_LED_IOC_SET_BRIGHTNESS,
				     &request);
}

int32_t pdi_led_enable(pdi_led_context_t *ctx, uint32_t index)
{
	return pdi_led_ioctl_checked(ctx, PDI_LED_IOC_ENABLE, &index);
}

int32_t pdi_led_disable(pdi_led_context_t *ctx, uint32_t index)
{
	return pdi_led_ioctl_checked(ctx, PDI_LED_IOC_DISABLE, &index);
}
