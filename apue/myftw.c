/*
 * @Author: your name
 * @Date: 2022-02-20 09:43:18
 * @LastEditTime: 2022-02-20 15:23:41
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /summary/apue/myftw.c
 */

/**************************************************
 * @file         : myftw.c
 * @brief        : 使用 read 和 write 函数复制一个文件
 * @author       : renzheng
 * @since        : 2022-02-20 09:43:18
 * @until        : 2022-02-18 09:21:39
 **************************************************/

#include <apue.h>
#include <dirent.h>
#include <limits.h>

typedef int Myfunc(const char *, const struct stat *, int);

static Myfunc myfunc;
static int myftw(char *, Myfunc *);
static int dopath(Myfunc *);
static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;


int
main(int argc, char *argv[])
{
    int ret;
    if (argc != 2)
        err_quit("usage: myftw <starting-pathname>");
    ret = myftw(argv[1], myfunc);
    ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
    if (ntot == 0)
        ntot = 1;
    printf("reqular files     = %7ld, %5.2f %%\n", nreg, nreg*100.0/ntot);
    printf("directories files = %7ld, %5.2f %%\n", ndir, ndir*100.0/ntot);
    printf("block special     = %7ld, %5.2f %%\n", nblk, nblk*100.0/ntot);
    printf("char special      = %7ld, %5.2f %%\n", nchr, nchr*100.0/ntot);
    printf("FIFOs             = %7ld, %5.2f %%\n", nfifo, nfifo*100.0/ntot);
    printf("symbolic links    = %7ld, %5.2f %%\n", nslink, nslink*100.0/ntot);
    printf("sockets           = %7ld, %5.2f %%\n", nsock, nsock*100.0/ntot);
}


enum FTW_TYPE { FTW_F=1, FTW_D, FTW_DNR, FTW_NS};

static char *fullpath;
static size_t pathlen;

static int
myftw(char *pathname, Myfunc *func)
{
    fullpath = path_alloc(&pathlen);
    if (pathlen <= strlen(pathname)) {
        pathlen = strlen(pathname) * 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL)
            err_sys("realloc failed");
    }
    strcpy(fullpath, pathname);
    return(dopath(func));
}

static int
dopath(Myfunc* func)
{
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int ret, n;

    if (lstat(fullpath, &statbuf) < 0)
        /* stat error */
        return(func(fullpath, &statbuf, FTW_NS));
    if (S_ISDIR(statbuf.st_mode) == 0)
        /* not dir */
        return(func(fullpath, &statbuf, FTW_F));
    
    /* 进入到这里，说明是一个目录文件。调用 func() 来处理该目录 */
    if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
        return(ret);
    n = strlen(fullpath);
    if (n + NAME_MAX + 2 > pathlen) {
        pathlen *= 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL)
            err_sys("realloc failed");
    }
    /* 既然 fullpath 是一个目录，则在其后面添加 / */
    fullpath[n++] = '/';
    fullpath[n] = 0;

    /* 循环处理该目录下的所有文件 */
    if ((dp = opendir(fullpath)) == NULL)
        return(func(fullpath, &statbuf, FTW_DNR));
    while ((dirp = readdir(dp)) != NULL) {
        /* 忽略 . 文件和 .. 文件 */
        if (strcmp(dirp->d_name, ".") == 0 ||
            strcmp(dirp->d_name, "..") == 0)
        {
            continue;
        }
        /*
         * 将目录下的文件名添加到路径后面
         */
        strcpy(&fullpath[n], dirp->d_name);
        if ((ret = dopath(func)) != 0)
            break;
    }
    fullpath[n-1] = 0;
    if (closedir(dp) < 0)
        err_ret("can't close directory %s", fullpath);
    return(ret);
}

static int
myfunc(const char *pathname, const struct stat *statptr, int type)
{
    switch (type) {
    case FTW_F:
        switch (statptr->st_mode & S_IFMT) {
        case S_IFREG:  nreg++;   break;
        case S_IFBLK:  nblk++;   break;
        case S_IFCHR:  nchr++;   break;
        case S_IFIFO:  nfifo++;  break;
        case S_IFLNK:  nslink++; break;
        case S_IFSOCK: nsock++;  break;
        case S_IFDIR:
            err_dump("for S_IFDIR for %s", pathname);
        }
        break;
    case FTW_D:
        ndir++;
        break;
    case FTW_DNR:
        err_ret("can't read directory %s", pathname);
        break;
    case FTW_NS:
        err_ret("stat error for %s", pathname);
        break;
    default:
        err_dump("unknown type %d for pathname %s", type, pathname);
    }
    return (0);
}