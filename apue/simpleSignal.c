/**************************************************
 * @file         : simpleSignal.c
 * @brief        : 一个简单的信号处理函数
 * @author       : renzheng
 * @since        : 2022-02-21 09:27:27
 * @until        : 2022-02-21 09:31:11
 **************************************************/

#include <apue.h>

static void sig_usr(int);

int
main(void)
{
    if (signal(SIGUSR1, sig_usr) == SIG_ERR)
        err_sys("can't catch SIGUSR1");
    if (signal(SIGUSR2, sig_usr) == SIG_ERR)
        err_sys("can't catch SIGUSR2");
    for (;;)
        pause();
}