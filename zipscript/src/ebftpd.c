
#include <stdint.h>
#include <stdio.h>

#if defined(__FreeBSD__)
# include <sys/extattr.h>
#else
# include <sys/xattr.h>
#endif

#ifndef ENOATTR
# define ENOATTR ENODATA
#endif

#ifndef ENODATA
# define ENODATA ENOATTR
#endif

#define EBFTPD_UID_ATTR_NAME "user.ebftpd.uid"
#define EBFTPD_GID_ATTR_NAME "user.ebftpd.gid"

#if defined(__FreeBSD__)

static int setxattr(const char *path, const char *name, const void *value, size_t size, int /* flags */)
{
        int ret = extattr_set_file(path, EXTATTR_NAMESPACE_USER, name, value, size);  
        return ret >= 0 ? 0 : ret;
}

#endif

static int set_attribute(const char* path, const char* attribute, int32_t id)
{
        char value[12];
        int len = snprintf(value, sizeof(value), "%i", id);
        return setxattr(path, attribute, value, len, 0);
}

int ebftpd_chown(const char* path, int32_t uid, int32_t gid)
{
       if (uid != -1) {
                int ret = set_attribute(path, EBFTPD_UID_ATTR_NAME, uid);
                if (ret < 0) return ret;
       }
       
       if (gid != -1) {
                int ret = set_attribute(path, EBFTPD_GID_ATTR_NAME, gid);
                if (ret < 0) return ret;
       }
       
       return 0;
}
