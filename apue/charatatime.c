/**************************************************
 * @file         : charatatime.c
 * @brief        : 父子进程输出字符串
 * @author       : renzheng
 * @since        : 2022-02-21 08:51:11
 * @until        : 2022-02-21 09:16:59
 **************************************************/

#include <apue.h>

static void charatatime(char *);

int
main(void)
{
    pid_t pid;
    TELL_WAIT();
    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        WAIT_PARENT();
        charatatime("abcdefghigklmnopqrstuvwxyz\n");
    } else {
        charatatime("ABCDEFGHIGKLMNOPQRSTUVWXYZ\n");
        TELL_CHILD(pid);
    }
    exit(0);
}

static void
charatatime(char *str)
{
    char *ptr;
    int c;

    /* 将标准输出设置为不带缓冲 */
    setbuf(stdout, NULL);
    /* 每个字符的输出都调用一次 write */
    for (ptr = str; (c = *ptr++) != 0;)
        putc(c, stdout);
}