#include "xf86drm.h"
#include "etna_util.h"
#include "etna_bo.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

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
    switch(request) {
    case DRM_IOCTL_MODE_CREATE_DUMB: {
            struct drm_mode_create_dumb *create_dumb = (struct drm_mode_create_dumb*)arg;
            struct fb_fix_screeninfo fb_fix;
            printf("DRM_IOCTL_MODE_CREATE_DUMB %dx%dx%d\n", create_dumb->width, create_dumb->height, create_dumb->bpp);
            int fd = open("/dev/fb0", O_RDWR | O_CLOEXEC);
            if (ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix)) {
                    printf("Error: failed to run FBIOGET_FSCREENINFO ioctl\n");
                close(fd);
                return -1;
            }
            create_dumb->pitch = fb_fix.line_length;
            /* Fake "dmabuf" spanning the console */
            create_dumb->handle = fd;
            return 0;
        } break;
    case DRM_IOCTL_MODE_DESTROY_DUMB: {
            struct drm_mode_destroy_dumb *destroy_dumb = (struct drm_mode_destroy_dumb*)arg;
            printf("DRM_IOCTL_MODE_DESTROY_DUMB %d\n", destroy_dumb->handle);
            close(destroy_dumb->handle);
            return 0;
        } break;
    default:
        printf("unhandled drmIoctl %d %08x %p\n", fd, (unsigned)request, arg);
    }
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
    fake_dmabuf_mode = true;
    *prime_fd = handle;
    return 0;
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
