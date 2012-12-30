#include "sigar.h"

static sigar_version_t sigar_version = {
    __DATE__,
    "@SCM_REVISION@",
    "libsigar 1.6.2",
    "x86_64-apple-darwin11.4.0",
    "darwin11.4.0",
    "x86_64",
    "SIGAR-1.6.2, "
    "SCM revision @SCM_REVISION@, "
    "built "__DATE__" as x86_64",
    1,
    6,
    2,
    0
};

SIGAR_DECLARE(sigar_version_t *) sigar_version_get(void)
{
    return &sigar_version;
}
