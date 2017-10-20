#include "xf86drm.h"
#include "etna_util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Just enough to make Mesa work */

drmVersionPtr drmGetVersion(int fd)
{
    drmVersionPtr version = ETNA_CALLOC_STRUCT(_drmVersion);
    version->name = "etnaviv";
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
    printf("drmIoctl %d %08x %p\n", fd, (unsigned)request, arg);
    return -1;
}

int drmGetDevice2(int fd, uint32_t flags, drmDevicePtr *device)
{
    drmDevicePtr ddev = ETNA_CALLOC_STRUCT(_drmDevice);
    printf("drmGetDevice2 %d %08x\n", fd, flags);

    ddev->bustype = DRM_BUS_PLATFORM;
    ddev->businfo.platform = ETNA_CALLOC_STRUCT(_drmPlatformBusInfo);
    strcpy(ddev->businfo.platform->fullname, "etnaviv");
    ddev->deviceinfo.platform = ETNA_CALLOC_STRUCT(_drmPlatformDeviceInfo);
    ddev->deviceinfo.platform->compatible = malloc(sizeof(char*)*2);
    ddev->deviceinfo.platform->compatible[0] = "etnaviv";
    ddev->deviceinfo.platform->compatible[1] = NULL;
    *device = ddev;
    return 0;
}

void drmFreeDevice(drmDevicePtr *device)
{
    free((*device)->deviceinfo.platform->compatible);
    ETNA_FREE(*device);
}

int drmGetDevices2(uint32_t flags, drmDevicePtr devices[], int max_devices)
{
    printf("drmGetDevices2 %08x\n", flags);
    return -1;
}

void drmFreeDevices(drmDevicePtr devices[], int count)
{
}

int drmAuthMagic(int fd, drm_magic_t magic)
{
    printf("drmAuthMagic\n");
    return 0;
}

int drmPrimeHandleToFD(int fd, uint32_t handle, uint32_t flags, int *prime_fd)
{
    printf("drmPrimeHandleToFD %d %d %08x\n", fd, handle, flags);
    return -1;
}

int drmPrimeFDToHandle(int fd, int prime_fd, uint32_t *handle)
{
    printf("drmPrimeFDToHandle %d %d\n", fd, prime_fd);
    return -1;
}

int drmGetCap(int fd, uint64_t capability, uint64_t *value)
{
    return -1;
}
