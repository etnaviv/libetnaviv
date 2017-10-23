#include "etna_internal.h"
#include "etna_util.h"

struct etna_pipe *etna_pipe_new(struct etna_gpu *gpu, enum etna_pipe_id id)
{
    if (id != ETNA_PIPE_3D) {
        /* Only handle 3D pipe for now, as viv hardcodes VIV_HW_3D */
        return NULL;
    }
    struct etna_pipe *pipe = ETNA_CALLOC_STRUCT(etna_pipe);
    pipe->gpu = gpu;
    pipe->id = id;
    return pipe;
}

void etna_pipe_del(struct etna_pipe *pipe)
{
    ETNA_FREE(pipe);
}

int etna_pipe_wait(struct etna_pipe *pipe, uint32_t timestamp, uint32_t ms)
{
    return viv_fence_finish(pipe->gpu->dev, timestamp, ms);
}

#define PIPE_TIMEOUT_INFINITE 0xffffffffffffffffull

int etna_pipe_wait_ns(struct etna_pipe *pipe, uint32_t timestamp, uint64_t ns)
{
    uint32_t ms = (ns + 999) / 1000;
    if (ns == PIPE_TIMEOUT_INFINITE)
        ms = 0xffffffff;
    return viv_fence_finish(pipe->gpu->dev, timestamp, ms);
}
