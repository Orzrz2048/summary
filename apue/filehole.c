/**************************************************
 * @file         : filehole.c
 * @brief        : 创建一个具有空洞的文件
 * @author       : renzheng
 * @since        : 2022-02-18 08:49:23
 * @until        : 2022-02-18 09:46:31
 **************************************************/

#include <apue.h>
#include <fcntl.h>

char buf1[] = "abcdefghij";
char buf2[] = "ABCDEFGHIJ";

int main(void)
{
    int fd;

    /*
     * FILE_MODE 为创建一个新文件默认的权限：
     * - S_IRUSR|S_IWUSR: 文件所属用户可读写
     * - S_IRGRP: 文件所属组可读
     * - S_IROTH: 其他用户可读
     */
    fd = creat("file.hole", FILE_MODE);
    if (fd < 0) {
        err_sys("creat error");
    }
    if (write(fd, buf1, 10) != 10) {
        err_sys("write error");
    }
    /* offset now = 10 */
    if (lseek(fd, 16384, SEEK_SET) == -1) {
        err_sys("lseek error");
    }
    /* offset now = 16384 */
    if (write(fd, buf2, 10) != 10) {
        err_sys("write error");
    }
    /* offset now = 16394 */
    return 0;
}