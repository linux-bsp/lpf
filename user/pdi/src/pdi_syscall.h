/************************************************************************
 * PDI syscall boundary
 ************************************************************************/

#ifndef PDI_SYSCALL_H
#define PDI_SYSCALL_H

typedef struct {
	int (*open)(const char *path, int flags);
	int (*close)(int fd);
	int (*ioctl)(int fd, unsigned long request, void *arg);
} pdi_syscall_ops_t;

void pdi_syscall_set_ops(const pdi_syscall_ops_t *ops);
void pdi_syscall_reset_ops(void);
int pdi_syscall_open(const char *path, int flags);
int pdi_syscall_close(int fd);
int pdi_syscall_ioctl(int fd, unsigned long request, void *arg);

#endif /* PDI_SYSCALL_H */
