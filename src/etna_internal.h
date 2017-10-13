#ifndef H_ETNA_INTERNAL
#define H_ETNA_INTERNAL

#include "etnaviv_drmif.h"

struct etna_gpu {
    struct etna_device *dev;
    unsigned int core;
};

struct etna_pipe {
    struct etna_gpu *gpu;
    enum etna_pipe_id id;
};

#endif
