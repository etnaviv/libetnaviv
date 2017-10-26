#include "xf86drm.h"
#include "xf86drmMode.h"
#include "etna_util.h"
#include "etna_bo.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#define WIDTH 1920
#define HEIGHT 1080
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

int drmOpen(const char *name, const char *busid)
{
    printf("drmOpen %s\n", name);
    assert(0);
    return -1;
}

/* drmMode emulation */
int drmModeConnectorSetProperty(int fd, uint32_t connector_id, uint32_t property_id,
				    uint64_t value)
{
    assert(0);
    return -1;
}

static void fill_mode(drmModeModeInfoPtr mode)
{
    mode->hdisplay = WIDTH;
    mode->vdisplay = HEIGHT;
    mode->vrefresh = 60;
    mode->type = DRM_MODE_TYPE_BUILTIN | DRM_MODE_TYPE_DEFAULT;
}

drmModeConnectorPtr drmModeGetConnector(int fd, uint32_t connectorId)
{
    static uint32_t id = 0;
    drmModeConnectorPtr res = ETNA_CALLOC_STRUCT(_drmModeConnector);
    res->connector_type = DRM_MODE_CONNECTOR_HDMIA;
    res->subpixel = DRM_MODE_SUBPIXEL_NONE;

    res->count_encoders = 1;
    res->encoders = &id;

    res->count_modes = 1;
    res->modes = ETNA_CALLOC_STRUCT(_drmModeModeInfo);
    fill_mode(&res->modes[0]);
    return res;
}

void drmModeFreeConnector( drmModeConnectorPtr ptr )
{
    ETNA_FREE(ptr->modes);
    ETNA_FREE(ptr);
}

drmModeCrtcPtr drmModeGetCrtc(int fd, uint32_t crtcId)
{
    drmModeCrtcPtr res = ETNA_CALLOC_STRUCT(_drmModeCrtc);
    res->crtc_id = 0;
    res->buffer_id = 0;
    res->width = WIDTH;
    res->height = HEIGHT;
    res->mode_valid = 1;
    fill_mode(&res->mode);
    return res;
}

void drmModeFreeCrtc( drmModeCrtcPtr ptr )
{
    ETNA_FREE(ptr);
}

drmModeEncoderPtr drmModeGetEncoder(int fd, uint32_t encoder_id)
{
    drmModeEncoderPtr res = ETNA_CALLOC_STRUCT(_drmModeEncoder);
    res->encoder_id = 0;
    res->crtc_id = 0;
    res->possible_crtcs = 1<<0;
    return res;
}

void drmModeFreeEncoder( drmModeEncoderPtr ptr )
{
    ETNA_FREE(ptr);
}

drmModePropertyPtr drmModeGetProperty(int fd, uint32_t propertyId)
{
    assert(0);
    return NULL;
}

void drmModeFreeProperty(drmModePropertyPtr ptr)
{
}

drmModeResPtr drmModeGetResources(int fd)
{
    static uint32_t id = 0;
    drmModeResPtr res = ETNA_CALLOC_STRUCT(_drmModeRes);
    res->count_fbs = 1;
    res->fbs = &id;
    res->count_crtcs = 1;
    res->crtcs = &id;
    res->count_connectors = 1;
    res->connectors = &id;
    res->count_encoders = 1;
    res->encoders = &id;
    res->min_width = res->max_width = WIDTH;
    res->min_height = res->max_height = HEIGHT;
    return res;
}

void drmModeFreeResources( drmModeResPtr ptr )
{
    ETNA_FREE(ptr);
}

int drmModeSetCrtc(int fd, uint32_t crtcId, uint32_t bufferId,
                   uint32_t x, uint32_t y, uint32_t *connectors, int count,
		   drmModeModeInfoPtr mode)
{
    printf("drmModeSetCrtc\n");
    return 0;
}
