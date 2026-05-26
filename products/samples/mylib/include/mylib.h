/**
 * @file mylib.h
 * @brief 示例库公共头文件
 */

#ifndef MYLIB_H
#define MYLIB_H

/**
 * @brief 初始化库
 * @return 成功返回 0，失败返回负数
 */
int mylib_init(void);

/**
 * @brief 清理库资源
 */
void mylib_cleanup(void);

#endif /* MYLIB_H */
