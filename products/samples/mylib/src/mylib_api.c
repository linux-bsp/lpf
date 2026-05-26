/**
 * @file mylib_api.c
 * @brief 示例库 API 实现
 */

#include <stdio.h>

int mylib_init(void)
{
	printf("mylib_init() called\n");
	return 0;
}

void mylib_cleanup(void)
{
	printf("mylib_cleanup() called\n");
}
