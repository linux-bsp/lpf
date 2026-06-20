/************************************************************************
 * PDI syscall boundary
 ************************************************************************/

#include "pdi_syscall.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int pdi_syscall_default_open(const char *path, int flags)
{
	return open(path, flags);
}

static int pdi_syscall_default_close(int fd)
{
	return close(fd);
}

static int pdi_syscall_default_ioctl(int fd, unsigned long request, void *arg)
{
	return ioctl(fd, request, arg);
}

static const pdi_syscall_ops_t g_pdi_syscall_default_ops = {
	.open = pdi_syscall_default_open,
	.close = pdi_syscall_default_close,
	.ioctl = pdi_syscall_default_ioctl,
};

static const pdi_syscall_ops_t *g_pdi_syscall_ops =
	&g_pdi_syscall_default_ops;

void pdi_syscall_set_ops(const pdi_syscall_ops_t *ops)
{
	g_pdi_syscall_ops = ops ? ops : &g_pdi_syscall_default_ops;
}

void pdi_syscall_reset_ops(void)
{
	g_pdi_syscall_ops = &g_pdi_syscall_default_ops;
}

int pdi_syscall_open(const char *path, int flags)
{
	return g_pdi_syscall_ops->open(path, flags);
}

int pdi_syscall_close(int fd)
{
	return g_pdi_syscall_ops->close(fd);
}

int pdi_syscall_ioctl(int fd, unsigned long request, void *arg)
{
	return g_pdi_syscall_ops->ioctl(fd, request, arg);
}
