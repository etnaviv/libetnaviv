/*
 * Copyright (C) 2014-2015 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#ifndef ETNAVIV_DRMIF_H_
#define ETNAVIV_DRMIF_H_

#include <xf86drm.h>
#include <stdint.h>

#define LIBETNAVIV_BO_EXTENSIONS

struct etna_bo;
struct etna_pipe;
struct etna_gpu;
struct etna_device;
struct etna_cmd_stream;

enum etna_pipe_id {
	ETNA_PIPE_3D = 0,
	ETNA_PIPE_2D = 1,
	ETNA_PIPE_VG = 2,
	ETNA_PIPE_MAX
};

enum etna_param_id {
	ETNA_GPU_MODEL                     = 0x1,
	ETNA_GPU_REVISION                  = 0x2,
	ETNA_GPU_FEATURES_0                = 0x3,
	ETNA_GPU_FEATURES_1                = 0x4,
	ETNA_GPU_FEATURES_2                = 0x5,
	ETNA_GPU_FEATURES_3                = 0x6,
	ETNA_GPU_FEATURES_4                = 0x7,
	ETNA_GPU_FEATURES_5                = 0x8,
	ETNA_GPU_FEATURES_6                = 0x9,

	ETNA_GPU_STREAM_COUNT              = 0x10,
	ETNA_GPU_REGISTER_MAX              = 0x11,
	ETNA_GPU_THREAD_COUNT              = 0x12,
	ETNA_GPU_VERTEX_CACHE_SIZE         = 0x13,
	ETNA_GPU_SHADER_CORE_COUNT         = 0x14,
	ETNA_GPU_PIXEL_PIPES               = 0x15,
	ETNA_GPU_VERTEX_OUTPUT_BUFFER_SIZE = 0x16,
	ETNA_GPU_BUFFER_SIZE               = 0x17,
	ETNA_GPU_INSTRUCTION_COUNT         = 0x18,
	ETNA_GPU_NUM_CONSTANTS             = 0x19,
	ETNA_GPU_NUM_VARYINGS              = 0x1a
};

/* bo create flags */
#define DRM_ETNA_GEM_TYPE_GEN        0x00000000 /* General, undefined */
#define DRM_ETNA_GEM_TYPE_IDX        0x00000001 /* Index buffer */
#define DRM_ETNA_GEM_TYPE_VTX        0x00000002 /* Vertex buffer */
#define DRM_ETNA_GEM_TYPE_TEX        0x00000003 /* Texture */
#define DRM_ETNA_GEM_TYPE_RT         0x00000004 /* Color render target */
#define DRM_ETNA_GEM_TYPE_ZS         0x00000005 /* Depth stencil target */
#define DRM_ETNA_GEM_TYPE_HZ         0x00000006 /* Hierarchical depth render target */
#define DRM_ETNA_GEM_TYPE_BMP        0x00000007 /* Bitmap */
#define DRM_ETNA_GEM_TYPE_TS         0x00000008 /* Tile status cache */
#define DRM_ETNA_GEM_TYPE_TXD        0x00000009 /* Texture descriptor */
#define DRM_ETNA_GEM_TYPE_IC         0x0000000A /* Instruction cache (Shader code) */
#define DRM_ETNA_GEM_TYPE_CMD        0x0000000B /* Command buffer */
#define DRM_ETNA_GEM_TYPE_MASK       0x0000000F

/* bo flags: */
#define DRM_ETNA_GEM_CACHE_CACHED       0x00010000
#define DRM_ETNA_GEM_CACHE_WC           0x00020000
#define DRM_ETNA_GEM_CACHE_UNCACHED     0x00040000
#define DRM_ETNA_GEM_CACHE_MASK         0x000f0000
/* map flags */
#define DRM_ETNA_GEM_FORCE_MMU          0x00100000

/* bo access flags: (keep aligned to ETNA_PREP_x) */
#define DRM_ETNA_PREP_READ              0x01
#define DRM_ETNA_PREP_WRITE             0x02
#define DRM_ETNA_PREP_NOSYNC            0x04

/* device functions:
 */

struct etna_device *etna_device_new(int fd);
struct etna_device *etna_device_new_dup(int fd);
struct etna_device *etna_device_ref(struct etna_device *dev);
void etna_device_del(struct etna_device *dev);
int etna_device_fd(struct etna_device *dev);

/* gpu functions:
 */

struct etna_gpu *etna_gpu_new(struct etna_device *dev, unsigned int core);
void etna_gpu_del(struct etna_gpu *gpu);
int etna_gpu_get_param(struct etna_gpu *gpu, enum etna_param_id param,
		uint64_t *value);


/* pipe functions:
 */

struct etna_pipe *etna_pipe_new(struct etna_gpu *gpu, enum etna_pipe_id id);
void etna_pipe_del(struct etna_pipe *pipe);
int etna_pipe_wait(struct etna_pipe *pipe, uint32_t timestamp, uint32_t ms);
int etna_pipe_wait_ns(struct etna_pipe *pipe, uint32_t timestamp, uint64_t ns);


/* buffer-object functions:
 */

struct etna_bo *etna_bo_new(struct etna_device *dev,
		uint32_t size, uint32_t flags);
struct etna_bo *etna_bo_from_handle(struct etna_device *dev,
		uint32_t handle, uint32_t size);
struct etna_bo *etna_bo_from_name(struct etna_device *dev, uint32_t name);
struct etna_bo *etna_bo_from_dmabuf(struct etna_device *dev, int fd);
struct etna_bo *etna_bo_ref(struct etna_bo *bo);
void etna_bo_del(struct etna_bo *bo);
int etna_bo_get_name(struct etna_bo *bo, uint32_t *name);
uint32_t etna_bo_handle(struct etna_bo *bo);
int etna_bo_dmabuf(struct etna_bo *bo);
uint32_t etna_bo_size(struct etna_bo *bo);
void * etna_bo_map(struct etna_bo *bo);
int etna_bo_cpu_prep(struct etna_bo *bo, uint32_t op);
void etna_bo_cpu_fini(struct etna_bo *bo);

/* Map user memory into GPU memory space */
struct etna_bo *etna_bo_from_usermem(struct etna_device *conn, void *memory, size_t size);

/* Map user memory (which may be write protected) into GPU memory space */
struct etna_bo *etna_bo_from_usermem_prot(struct etna_device *conn, void *memory, size_t size, int prot);

/* Buffer object from framebuffer range */
struct etna_bo *etna_bo_from_fbdev(struct etna_device *conn, int fd, size_t offset, size_t size);

/* cmd stream functions:
 */

/**TODO: move this to internal structure */
/* Number of command buffers, to be used in a circular fashion.
 */
#define NUM_COMMAND_BUFFERS 5

struct etna_cmdbuf {
    /* sync signal for command buffer */
    int sig_id;
    struct etna_bo *bo;
};

struct etna_cmd_stream {
    /* Driver connection */
    struct etna_device *conn;
    /* Keep track of current command buffer and writing location.
     * The offset is kept here instead of in cmdbuf[cur_buf].offset to save an level of indirection
     * when building the buffer. It is only copied to the command buffer before submission to the kernel
     * in etna_flush().
     * Also, this offset is in terms of 32 bit words, instead of in bytes, so it can be directly used to index
     * into buf.
     */
    uint32_t *buf;
    uint32_t offset;
    uint32_t end;
    /* Current buffer id (index into cmdbuf) */
    int cur_buf;
    /* Stored current buffer id when building context */
    int stored_buf;
    /* Synchronization signal for finish() */
    int sig_id;
    /* Structures for kernel */
    struct _gcoCMDBUF *cmdbuf[NUM_COMMAND_BUFFERS];
    /* Extra information per command buffer */
    struct etna_cmdbuf cmdbufi[NUM_COMMAND_BUFFERS];
    /* number of unsignalled flushes (used to work around kernel bug) */
    int flushes;
    /* command queue */
    struct etna_queue *queue;
    /* context */
    uint64_t ctx;
    /* context reset notification */
    void (*reset_notify)(struct etna_cmd_stream *stream, void *priv);
    void *reset_notify_priv;
};

/* internal (non-inline) part of etna_reserve.
   only to be used from etna_reserve. */
void _etna_cmd_stream_reserve_internal(struct etna_cmd_stream *ctx, size_t n);

struct etna_cmd_stream *etna_cmd_stream_new(struct etna_pipe *pipe, uint32_t size,
		void (*reset_notify)(struct etna_cmd_stream *stream, void *priv),
		void *priv);
void etna_cmd_stream_del(struct etna_cmd_stream *stream);
uint32_t etna_cmd_stream_timestamp(struct etna_cmd_stream *stream);
void etna_cmd_stream_flush(struct etna_cmd_stream *stream);
void etna_cmd_stream_flush2(struct etna_cmd_stream *stream, int in_fence_fd,
			    int *out_fence_fd);
void etna_cmd_stream_finish(struct etna_cmd_stream *stream);

static inline void etna_cmd_stream_reserve(struct etna_cmd_stream *ctx, size_t n)
{
        if(ctx->cur_buf >= 0)
        {
#ifdef CMD_DEBUG
                printf("etna_reserve: %i at offset %i\n", (int)n, (int)ctx->offset);
#endif
                if((ctx->offset + n) <= ctx->end) /* enough bytes free in buffer */
                {
                    return;
                }
        }
        _etna_cmd_stream_reserve_internal(ctx, n);
}

static inline uint32_t etna_cmd_stream_avail(struct etna_cmd_stream *stream)
{
	return stream->end - stream->offset;
}

static inline void etna_cmd_stream_emit(struct etna_cmd_stream *stream, uint32_t data)
{
        stream->buf[stream->offset++] = data;
}

static inline uint32_t etna_cmd_stream_get(struct etna_cmd_stream *stream, uint32_t offset)
{
	return stream->buf[offset];
}

static inline void etna_cmd_stream_set(struct etna_cmd_stream *stream, uint32_t offset,
		uint32_t data)
{
	stream->buf[offset] = data;
}

static inline uint32_t etna_cmd_stream_offset(struct etna_cmd_stream *stream)
{
	return stream->offset;
}

struct etna_reloc {
	struct etna_bo *bo;
#define ETNA_RELOC_READ             0x0001
#define ETNA_RELOC_WRITE            0x0002
	uint32_t flags;
	uint32_t offset;
};

void etna_cmd_stream_reloc(struct etna_cmd_stream *stream, const struct etna_reloc *r);

#endif /* ETNAVIV_DRMIF_H_ */