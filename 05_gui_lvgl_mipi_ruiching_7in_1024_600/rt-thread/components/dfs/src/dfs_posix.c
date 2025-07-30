#include <dfs_file.h>
#include <dfs_private.h>
#include <sys/errno.h>
#include <newlib_compiler.h>
#include <dirent.h>

DIR *opendir(const char *name)
{
    return rt_opendir(name);
}

int closedir(DIR *d)
{
    return rt_closedir(d);
}

struct dirent *readdir(DIR *d)
{
    return rt_readdir(d);
}

int mkdir(const char *path, mode_t mode)
{
    return rt_mkdir(path, mode);
}