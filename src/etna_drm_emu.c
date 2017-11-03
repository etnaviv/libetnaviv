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

struct etna_drm_emu {
    bool initialized;
    unsigned width;
    unsigned height;
    unsigned stride;
};

static struct etna_drm_emu state;

static void drm_emu_initialize()
{
    if (state.initialized)
        return;
    /* Query video mode once, at startup */
    struct fb_fix_screeninfo fb_fix;
    struct fb_var_screeninfo fb_var;
    int fd = open("/dev/fb0", O_RDWR | O_CLOEXEC);
    if (ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix)) {
        printf("%s: failed to run FBIOGET_FSCREENINFO ioctl\n", __func__);
        abort();
    }
    if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_var)) {
        printf("%s: failed to run FBIOGET_VSCREENINFO ioctl\n", __func__);
        abort();
    }
    state.initialized = true;
    state.width = fb_var.xres;
    state.height = fb_var.yres;
    state.stride = fb_fix.line_length;
    printf("%s: %dx%d stride %d\n", __func__, state.width, state.height, state.stride);
    assert(!(state.stride & 0x40)); /* PE framebuffer alignment */
    close(fd);
}

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

char *drmGetDeviceNameFromFd2(int fd)
{
    return "etnaviv";
}

int drmIoctl(int fd, unsigned long request, void *arg)
{
    drm_emu_initialize();
    switch(request) {
    case DRM_IOCTL_MODE_CREATE_DUMB: {
            struct drm_mode_create_dumb *create_dumb = (struct drm_mode_create_dumb*)arg;
            printf("DRM_IOCTL_MODE_CREATE_DUMB %dx%dx%d\n", create_dumb->width, create_dumb->height, create_dumb->bpp);
            if (create_dumb->width != state.width) {
                printf("%s: Allocation of non-framebuffer-width surface not supported\n", __func__);
                return -1;
            }
            int fd = open("/dev/fb0", O_RDWR | O_CLOEXEC);
            create_dumb->pitch = state.stride;
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
    drm_emu_initialize();
    mode->hdisplay = state.width;
    mode->vdisplay = state.height;
    mode->vrefresh = 60;
    mode->type = DRM_MODE_TYPE_BUILTIN | DRM_MODE_TYPE_DEFAULT;
}

drmModeConnectorPtr drmModeGetConnector(int fd, uint32_t connectorId)
{
    static uint32_t id = 0;
    drmModeConnectorPtr res = ETNA_CALLOC_STRUCT(_drmModeConnector);
    res->connection = DRM_MODE_CONNECTED;
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
    drm_emu_initialize();
    res->crtc_id = 0;
    res->buffer_id = 0;
    res->width = state.width;
    res->height = state.height;
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
    drm_emu_initialize();
    res->count_fbs = 1;
    res->fbs = &id;
    res->count_crtcs = 1;
    res->crtcs = &id;
    res->count_connectors = 1;
    res->connectors = &id;
    res->count_encoders = 1;
    res->encoders = &id;
    res->min_width = res->max_width = state.width;
    res->min_height = res->max_height = state.height;
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

drmModeFBPtr drmModeGetFB(int fd, uint32_t bufferId)
{
    drmModeFBPtr res = ETNA_CALLOC_STRUCT(_drmModeFB);
    drm_emu_initialize();
    res->width = state.width;
    res->height = state.height;
    /* ? */
    return res;
}

void drmModeFreeFB( drmModeFBPtr ptr )
{
    ETNA_FREE(ptr);
}

int drmModeAddFB(int fd, uint32_t width, uint32_t height, uint8_t depth,
			uint8_t bpp, uint32_t pitch, uint32_t bo_handle,
			uint32_t *buf_id)
{
    *buf_id = 0;
    return 0;
}

int drmModeAddFB2(int fd, uint32_t width, uint32_t height,
			 uint32_t pixel_format, uint32_t bo_handles[4],
			 uint32_t pitches[4], uint32_t offsets[4],
			 uint32_t *buf_id, uint32_t flags)
{
    *buf_id = 0;
    return 0;
}

int drmModeAddFB2WithModifiers(int fd, uint32_t width, uint32_t height,
			       uint32_t pixel_format, uint32_t bo_handles[4],
			       uint32_t pitches[4], uint32_t offsets[4],
			       uint64_t modifier[4], uint32_t *buf_id, uint32_t flags)
{
    *buf_id = 0;
    return 0;
}

int drmModeRmFB(int fd, uint32_t bufferId)
{
    return 0;
}

int drmModeSetCursor(int fd, uint32_t crtcId, uint32_t bo_handle, uint32_t width, uint32_t height)
{
    return 0;
}

int drmModeSetCursor2(int fd, uint32_t crtcId, uint32_t bo_handle, uint32_t width, uint32_t height, int32_t hot_x, int32_t hot_y)
{
    return 0;
}

int drmModeMoveCursor(int fd, uint32_t crtcId, int x, int y)
{
    return 0;
}
