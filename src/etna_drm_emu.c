#include "xf86drm.h"
#include "etna_util.h"

#include <stdlib.h>
#include <string.h>

drmVersionPtr drmGetVersion(int fd)
{
    drmVersionPtr version = ETNA_CALLOC_STRUCT(_drmVersion);
    version->name = "libetnaviv galcore emulated DRM driver";
    version->name_len = strlen(version->name);
    version->date = "2017-10-13";
    version->date_len = strlen(version->date);
    version->desc = "2017-10-13";
    version->desc_len = strlen(version->desc);
    return version;
}

void drmFreeVersion(drmVersionPtr version)
{
    ETNA_FREE(version);
}
