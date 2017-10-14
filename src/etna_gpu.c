#include "etna_internal.h"
#include "etna_util.h"
#include "viv.h"

struct etna_gpu *etna_gpu_new(struct etna_device *dev, unsigned int core)
{
    if (core == 0) {
        struct etna_gpu *gpu = ETNA_CALLOC_STRUCT(etna_gpu);
        gpu->dev = dev;
        gpu->core = core;
        return gpu;
    } else {
        return NULL;
    }
}

void etna_gpu_del(struct etna_gpu *gpu)
{
    ETNA_FREE(gpu);
}

int etna_gpu_get_param(struct etna_gpu *gpu, enum etna_param_id param,
		uint64_t *value)
{
    switch(param) {
        case ETNA_GPU_MODEL:
            *value = gpu->dev->chip.chip_model;
            return 0;
        case ETNA_GPU_REVISION:
            *value = gpu->dev->chip.chip_revision;
            return 0;
        case ETNA_GPU_FEATURES_0:
            *value = gpu->dev->chip.chip_features[0];
            return 0;
        case ETNA_GPU_FEATURES_1:
            *value = gpu->dev->chip.chip_features[1];
            return 0;
        case ETNA_GPU_FEATURES_2:
            *value = gpu->dev->chip.chip_features[2];
            return 0;
        case ETNA_GPU_FEATURES_3:
            *value = gpu->dev->chip.chip_features[3];
            return 0;
        case ETNA_GPU_FEATURES_4:
            *value = gpu->dev->chip.chip_features[4];
            return 0;
        case ETNA_GPU_FEATURES_5:
            *value = gpu->dev->chip.chip_features[5];
            return 0;
        case ETNA_GPU_FEATURES_6:
            *value = gpu->dev->chip.chip_features[6];
            return 0;
        case ETNA_GPU_STREAM_COUNT:
            *value = gpu->dev->chip.stream_count;
            return 0;
        case ETNA_GPU_REGISTER_MAX:
            *value = gpu->dev->chip.register_max;
            return 0;
        case ETNA_GPU_THREAD_COUNT:
            *value = gpu->dev->chip.thread_count;
            return 0;
        case ETNA_GPU_VERTEX_CACHE_SIZE:
            *value = gpu->dev->chip.vertex_cache_size;
            return 0;
        case ETNA_GPU_SHADER_CORE_COUNT:
            *value = gpu->dev->chip.shader_core_count;
            return 0;
        case ETNA_GPU_PIXEL_PIPES:
            *value = gpu->dev->chip.pixel_pipes;
            return 0;
        case ETNA_GPU_VERTEX_OUTPUT_BUFFER_SIZE:
            *value = gpu->dev->chip.vertex_output_buffer_size;
            return 0;
        case ETNA_GPU_BUFFER_SIZE:
            *value = gpu->dev->chip.buffer_size;
            return 0;
        case ETNA_GPU_INSTRUCTION_COUNT:
            *value = gpu->dev->chip.instruction_count;
            return 0;
        case ETNA_GPU_NUM_CONSTANTS:
            *value = gpu->dev->chip.num_constants;
            return 0;
        case ETNA_GPU_NUM_VARYINGS:
            *value = gpu->dev->chip.varyings_count;
            return 0;
    }
    return -1;
}
