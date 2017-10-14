#include "xf86drm.h"
#include "etna_util.h"

#include <stdlib.h>
#include <string.h>

/* Just enough to make Mesa work */

drmVersionPtr drmGetVersion(int fd)
{
    drmVersionPtr version = ETNA_CALLOC_STRUCT(_drmVersion);
    version->name = "libetnaviv galcore emulated DRM driver";
    version->name_len = strlen(version->name);
    version->date = "2017-10-13";
    version->date_len = strlen(version->date);
    version->desc = "Vantenstein's Monster";
    version->desc_len = strlen(version->desc);
    return version;
}

void drmFreeVersion(drmVersionPtr version)
{
    ETNA_FREE(version);
}

int drmIoctl(int fd, unsigned long request, void *arg)
{
    return -1;
}

int drmGetDevice2(int fd, uint32_t flags, drmDevicePtr *device)
{
    return -1;
}

int drmGetDevices2(uint32_t flags, drmDevicePtr devices[], int max_devices)
{
    return -1;
}

void drmFreeDevice(drmDevicePtr *device)
{
}

void drmFreeDevices(drmDevicePtr devices[], int count)
{
}

int drmAuthMagic(int fd, drm_magic_t magic)
{
    return 0;
}
