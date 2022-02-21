/**************************************************
 * @file         : iocopy.c
 * @brief        : 探索各种 I/O 复制一个文件的效率
 * @author       : renzheng
 * @since        : 2022-02-18 09:14:15
 * @until        : 2022-02-21 02:26:07
 **************************************************/
#include <apue.h>

#define BUFFSIZE 4096

void unbufferedio(char *);
void bufferedio();
void linebufferedio(char *);

int main(int argc, char *argv[])
{
    int n;
    char buf[BUFFSIZE];

    if (argc != 2)
        err_quit("usage: %s [0|1|2]\n"
                 "\t0 - unbuffered I/O\n"
                 "\t1 - buffered I/O\n"
                 "\t2 - linebuffered I/O\n",
                 argv[0]);
    n = atoi(argv[1]);
    if (n == 0)
        unbufferedio(buf);
    else if (n == 1)
        bufferedio();
    else if (n == 2)
        linebufferedio(buf);
    else
        err_quit("usage: %s [0|1|2]\n"
                 "\t0 - unbuffered I/O\n"
                 "\t1 - buffered I/O\n"
                 "\t2 - linebuffered I/O\n",
                 argv[0]);

    exit(0);
}

/**
 * @brief       : 文件 I/O 为 unbuffered I/O、低级磁盘 I/O，
 *                每个 read、write 都调用内核中的一个系统调用
 * @param        *buf [char]
 * @return       [*]
 */
void unbufferedio(char *buf)
{
    int n;
    while ((n = read(STDIN_FILENO, buf, BUFFSIZE)) > 0) {
        if (write(STDOUT_FILENO, buf, n) != n) {
            err_sys("write error");
        }
    }

    if (n < 0) {
        err_sys("read error");
    }
}

/**
 * @brief       : 标准 I/O 为高级磁盘 I/O（缓冲区分配、以优化的块长度执行）
 *                对流（stream）进行操作
 * @return       [*]
 */
void bufferedio()
{
    int n;
    while ((n = getc(stdin)) != EOF)
        if (putc(n, stdout) == EOF)
            err_sys("output error");

    if (ferror(stdin))
        err_sys("input error");
}

/**
 * @brief       : 行缓冲
 * @return       [*]
 */
void linebufferedio(char *buf)
{
    int n;
    while (fgets(buf, n, stdin) != NULL)
        if (fputs(buf, stdout) == EOF)
            err_sys("output error");

    if (ferror(stdin))
        err_sys("input error");
}