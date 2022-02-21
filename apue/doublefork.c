/**************************************************
 * @file         : doublefork.c
 * @brief        : fork 两次，可以创造出父进程终止而子进程未僵死状态
 * @author       : renzheng
 * @since        : 2022-02-21 08:13:25
 * @until        : 2022-02-21 08:48:17
 **************************************************/

#include <apue.h>
#include <sys/wait.h>

int
main(void)
{
    pid_t pid;
    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        if ((pid = fork()) < 0) {
            err_sys("fork error");
        } else if (pid > 0 ) {
            /* 第二次 fork 的父进程执行退出 */
            exit(0);
        }

        /*
         * 第二次 fork 的子进程等待两秒以便第二次 fork 的父进程退出
         * 此时该进程应交由 init 进程管理，即 ppid = 1
         */
        sleep(2);
        printf("second child, parent pid = %ld\n", (long)getppid());
        exit(0);
    }

    if (waitpid(pid, NULL, 0) != pid)
        err_sys("waitpid error");
    exit(0);
}
