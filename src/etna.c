/*
 * Copyright (c) 2012-2013 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "etna.h"
#include "etna_bo.h"
#include "viv.h"
#include "etna_queue.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "gc_abi.h"

#include "viv_internal.h"
#include "etna_internal.h"

static pthread_mutex_t idx_lock = PTHREAD_MUTEX_INITIALIZER;

//#define DEBUG
//#define DEBUG_CMDBUF

static int gpu_context_initialize(struct etna_cmd_stream_priv *ctx)
{
    /* attach to GPU */
    int err;
    gcsHAL_INTERFACE id = {};
    id.command = gcvHAL_ATTACH;
    if((err=viv_invoke(ctx->base.conn, &id)) != gcvSTATUS_OK)
    {
#ifdef DEBUG
        fprintf(stderr, "Error attaching to GPU\n");
#endif
        return ETNA_INTERNAL_ERROR;
    }

#ifdef DEBUG
    fprintf(stderr, "Context 0x%08x\n", (int)id.u.Attach.context);
#endif

    ctx->ctx = VIV_TO_HANDLE(id.u.Attach.context);
    return ETNA_OK;
}

static int gpu_context_free(struct etna_cmd_stream_priv *ctx)
{
    /* attach to GPU */
    int err;
    gcsHAL_INTERFACE id = {};
    id.command = gcvHAL_DETACH;
    id.u.Detach.context = HANDLE_TO_VIV(ctx->ctx);

    if((err=viv_invoke(ctx->base.conn, &id)) != gcvSTATUS_OK)
    {
#ifdef DEBUG
        fprintf(stderr, "Error detaching from the GPU\n");
#endif
        return ETNA_INTERNAL_ERROR;
    }

    return ETNA_OK;
}

/* Clear a command buffer */
static void clear_buffer(gcoCMDBUF cmdbuf)
{
    /* Prepare command buffer for use */
    cmdbuf->startOffset = 0x0;
    cmdbuf->offset = BEGIN_COMMIT_CLEARANCE;
}

/* Switch to next buffer, optionally wait for it to be available */
static int switch_next_buffer(struct etna_cmd_stream_priv *ctx)
{
    int next_buf_id = (ctx->base.cur_buf + 1) % NUM_COMMAND_BUFFERS;
#if 0
    fprintf(stderr, "Switching to new buffer %i\n", next_buf_id);
#endif
    if(viv_user_signal_wait(ctx->base.conn, ctx->cmdbufi[next_buf_id].sig_id, VIV_WAIT_INDEFINITE) != 0)
    {
#ifdef DEBUG
        fprintf(stderr, "Error waiting for command buffer sync signal\n");
#endif
        return ETNA_INTERNAL_ERROR;
    }
    clear_buffer(ctx->cmdbuf[next_buf_id]);
    ctx->base.cur_buf = next_buf_id;
    ctx->base.buf = VIV_TO_PTR(ctx->cmdbuf[next_buf_id]->logical);
    ctx->base.offset = ctx->cmdbuf[next_buf_id]->offset / 4;
#ifdef DEBUG
    fprintf(stderr, "Switched to command buffer %i\n", ctx->cur_buf);
#endif
    return ETNA_OK;
}

struct etna_cmd_stream *etna_cmd_stream_new(struct etna_pipe *pipe, uint32_t size,
		void (*reset_notify)(struct etna_cmd_stream *stream, void *priv),
		void *priv)
{
    int rv;
    struct etna_device *conn = pipe->gpu->dev;
    struct etna_cmd_stream_priv *ctx = ETNA_CALLOC_STRUCT(etna_cmd_stream_priv);
    ctx->base.conn = conn;
    ctx->reset_notify = reset_notify;
    ctx->reset_notify_priv = priv;

    if(gpu_context_initialize(ctx) != ETNA_OK)
    {
        ETNA_FREE(ctx);
        return NULL;
    }

    /* Create synchronization signal */
    if(viv_user_signal_create(conn, 0, &ctx->sig_id) != 0) /* automatic resetting signal */
    {
#ifdef DEBUG
        fprintf(stderr, "Cannot create user signal\n");
#endif
        return NULL;
    }
#ifdef DEBUG
    fprintf(stderr, "Created user signal %i\n", ctx->sig_id);
#endif

    /* Allocate command buffers, and create a synchronization signal for each.
     * Also signal the synchronization signal for the buffers to tell that the buffers are ready for use.
     */
    for(int x=0; x<NUM_COMMAND_BUFFERS; ++x)
    {
        ctx->cmdbuf[x] = ETNA_CALLOC_STRUCT(_gcoCMDBUF);
        if((ctx->cmdbufi[x].bo = etna_bo_new(conn, COMMAND_BUFFER_SIZE, DRM_ETNA_GEM_TYPE_CMD))==NULL)
        {
#ifdef DEBUG
            fprintf(stderr, "Error allocating host memory for command buffer\n");
#endif
            return NULL;
        }
        ctx->cmdbuf[x]->object.type = gcvOBJ_COMMANDBUFFER;
#ifdef GCABI_CMDBUF_HAS_PHYSICAL
        ctx->cmdbuf[x]->physical = etna_bo_gpu_address(ctx->cmdbufi[x].bo);
        ctx->cmdbuf[x]->bytes = etna_bo_size(ctx->cmdbufi[x].bo);
#endif
        ctx->cmdbuf[x]->logical = PTR_TO_VIV((void*)etna_bo_map(ctx->cmdbufi[x].bo));
#ifdef GCABI_CMDBUF_HAS_RESERVED_TAIL
        ctx->cmdbuf[x]->reservedTail = END_COMMIT_CLEARANCE;
#endif

        if(viv_user_signal_create(conn, 0, &ctx->cmdbufi[x].sig_id) != 0 ||
           viv_user_signal_signal(conn, ctx->cmdbufi[x].sig_id, 1) != 0)
        {
#ifdef DEBUG
            fprintf(stderr, "Cannot create user signal\n");
#endif
            return NULL;
        }
#ifdef DEBUG
        fprintf(stderr, "Allocated buffer %i: phys=%08x log=%08x bytes=%08x [signal %i]\n", x,
                (uint32_t)ctx->cmdbuf[x]->physical, (uint32_t)ctx->cmdbuf[x]->logical, ctx->cmdbuf[x]->bytes, ctx->cmdbufi[x].sig_id);
#endif
    }

    /* Allocate command queue */
    if((rv = etna_queue_create(&ctx->base, &ctx->queue)) != ETNA_OK)
    {
#ifdef DEBUG
        fprintf(stderr, "Error allocating kernel command queue: %d\n", rv);
#endif
        return NULL;
    }

    /* Set current buffer to ETNA_NO_BUFFER, to signify that we need to switch to buffer 0 before
     * queueing of commands can be started.
     */
    ctx->base.cur_buf = ETNA_NO_BUFFER;
    ctx->base.end = (COMMAND_BUFFER_SIZE - END_COMMIT_CLEARANCE)/4;

    /* Make sure there is an active buffer */
    if((rv = switch_next_buffer(ctx)) != ETNA_OK)
    {
        fprintf(stderr, "%s: can't switch to next command buffer: %i\n", __func__, rv);
        abort(); /* Buffer is in invalid state XXX need some kind of recovery.
                    This could involve waiting and re-uploading the context state. */
    }

    return &ctx->base;
}

void etna_cmd_stream_del(struct etna_cmd_stream *ctx_)
{
    struct etna_cmd_stream_priv *ctx = etna_cmd_stream_priv(ctx_);
    /* Free kernel command queue */
    etna_queue_free(ctx->queue);
    /* Free command buffers */
    for(int x=0; x<NUM_COMMAND_BUFFERS; ++x)
    {
        viv_user_signal_destroy(ctx->base.conn, ctx->cmdbufi[x].sig_id);
        etna_bo_del_ext(ctx->cmdbufi[x].bo, NULL);
        ETNA_FREE(ctx->cmdbuf[x]);
    }
    viv_user_signal_destroy(ctx->base.conn, ctx->sig_id);
    gpu_context_free(ctx);

    ETNA_FREE(ctx);
}

/* internal (non-inline) part of etna_cmd_stream_reserve
 * - commit current command buffer (if there is a current command buffer)
 * - signify when current command buffer becomes available using a signal
 * - switch to next command buffer
 */
void _etna_cmd_stream_reserve_internal(struct etna_cmd_stream *ctx_, size_t n)
{
    int status;
    struct etna_cmd_stream_priv *ctx = etna_cmd_stream_priv(ctx_);
#ifdef DEBUG
    fprintf(stderr, "Buffer full\n");
#endif
    if((ctx->base.offset*4 + END_COMMIT_CLEARANCE) > COMMAND_BUFFER_SIZE)
    {
        fprintf(stderr, "%s: Command buffer overflow! This is likely a programming error in the GPU driver.\n", __func__);
        abort();
    }
#if 0
    fprintf(stderr, "Submitting old buffer %i\n", ctx->base.cur_buf);
#endif
    /* Otherwise, if there is something to be committed left in the current command buffer, commit it */
    if((status = etna_flush(ctx_)) != ETNA_OK)
    {
        fprintf(stderr, "%s: reserve failed: %i\n", __func__, status);
        abort(); /* buffer is in invalid state XXX need some kind of recovery */
    }
}

/** Unreference bos, make sure async cleanup is done after the submit only.
 * Also update buffer synchronization timestamps.
 */
static void unref_bos(struct etna_cmd_stream_priv *priv, uint32_t timestamp)
{
    uint32_t i;
    assert(priv->queue);
    for (i=0; i<priv->nr_bos; ++i) {
#ifdef DEBUG_BO
        printf("%s: releasing bo %p at index %d\n", __func__, priv->bos[i], i);
#endif
        priv->bos[i]->current_stream = NULL;
        /* TODO actually remember reloc flags which ones to set */
        priv->bos[i]->timestamp_write = timestamp;
        priv->bos[i]->timestamp_any = timestamp;
        etna_bo_del_ext(priv->bos[i], priv->queue);
    }
    priv->nr_bos = 0;
}

int etna_flush(struct etna_cmd_stream *ctx_)
{
    struct etna_cmd_stream_priv *ctx = etna_cmd_stream_priv(ctx_);
    int status = ETNA_OK;
    uint32_t fence;
    if(ctx == NULL)
        return ETNA_INVALID_ADDR;
    if(ctx->base.cur_buf == ETNA_NO_BUFFER || (ctx->base.offset*4 <= (ctx->cmdbuf[ctx->base.cur_buf]->startOffset + BEGIN_COMMIT_CLEARANCE)))
        return ETNA_OK; /* Nothing to do */

    {
        int signal;
        /* Need to lock the fence mutex to make sure submits are ordered by
         * fence number.
         */
        pthread_mutex_lock(&ctx->base.conn->fence_mutex);
        do {
            /*   Get next fence ID */
            if((status = _viv_fence_new(ctx->base.conn, &fence, &signal)) != VIV_STATUS_OK)
            {
                fprintf(stderr, "%s: could not request fence\n", __func__);
                goto unlock_and_return_status;
            }
        } while(fence == 0); /* don't return fence handle 0 as it is interpreted as error value downstream */
        /*   Queue the signal. This can call in turn call this function (but
         * without fence) if the queue was full, so we should be able to handle
         * that. In that case, we will exit from this function with only
         * this fence in the queue and an empty command buffer.
         */
        if((status = etna_queue_signal(ctx->queue, signal, VIV_WHERE_PIXEL)) != ETNA_OK)
        {
            fprintf(stderr, "%s: error %i queueing fence signal %i\n", __func__, status, signal);
            goto unlock_and_return_status;
        }
    }
    /***** Start fence mutex locked */
    /* Unreference bos */
    unref_bos(ctx, fence);

    /* Queue signal to signify when buffer is available again */
    if((status = etna_queue_signal(ctx->queue, ctx->cmdbufi[ctx->base.cur_buf].sig_id, VIV_WHERE_COMMAND)) != ETNA_OK)
    {
        fprintf(stderr, "%s: queue signal for old buffer failed: %i\n", __func__, status);
        abort(); /* buffer is in invalid state XXX need some kind of recovery */
    }

    /* Make sure to unlock the mutex before returning */
    gcoCMDBUF cur_buf = ctx->cmdbuf[ctx->base.cur_buf];

    cur_buf->offset = ctx->base.offset*4; /* Copy over current end offset into CMDBUF, for kernel */
#ifdef DEBUG
    fprintf(stderr, "Committing command buffer %i startOffset=%x offset=%x\n", ctx->cur_buf,
            cur_buf->startOffset, ctx->offset*4);
#endif
#ifdef DEBUG_CMDBUF
    etna_dump_cmd_buffer(ctx);
#endif
    if((status = viv_commit(ctx->base.conn, cur_buf, ctx->ctx, _etna_queue_first(ctx->queue))) != 0)
    {
        fprintf(stderr, "Error committing command buffer\n");
        abort();
    }
    _viv_fence_mark_pending(ctx->base.conn, fence);
    ctx->submit_fence = fence;
    pthread_mutex_unlock(&ctx->base.conn->fence_mutex);
    /***** End fence mutex locked */

    /* Move on to next buffer */
    if((status = switch_next_buffer(ctx)) != ETNA_OK)
    {
        fprintf(stderr, "%s: can't switch to next command buffer: %i\n", __func__, status);
        abort(); /* Buffer is in invalid state XXX need some kind of recovery.
                    This could involve waiting and re-uploading the context state. */
    }

    /* Reset context */
    ctx->reset_notify(ctx_, ctx->reset_notify_priv);

#ifdef DEBUG
    fprintf(stderr, "  New start offset: %x New offset: %x\n", cur_buf->startOffset, cur_buf->offset);
#endif
    return ETNA_OK;

unlock_and_return_status: /* Unlock fence mutex (if necessary) and return status */
    pthread_mutex_unlock(&ctx->base.conn->fence_mutex);
    return status;
}

void etna_cmd_stream_finish(struct etna_cmd_stream *ctx_)
{
    struct etna_cmd_stream_priv *ctx = etna_cmd_stream_priv(ctx_);
    int status;
    /* Submit event queue with SIGNAL, fromWhere=gcvKERNEL_PIXEL (wait for pixel engine to finish) */
    if(etna_queue_signal(ctx->queue, ctx->sig_id, VIV_WHERE_PIXEL) != 0)
    {
        fprintf(stderr, "%s: Internal error while queing signal.\n", __func__);
        abort();
    }
    if((status = etna_flush(ctx_)) != ETNA_OK)
    {
        fprintf(stderr, "%s: Internal error %d while flushing.\n", __func__, status);
        abort();
    }
#ifdef DEBUG
    fprintf(stderr, "finish: Waiting for signal...\n");
#endif
    /* Wait for signal */
    if(viv_user_signal_wait(ctx->base.conn, ctx->sig_id, VIV_WAIT_INDEFINITE) != 0)
    {
        fprintf(stderr, "%s: Internal error while waiting for signal.\n", __func__);
        abort();
    }
}

void etna_dump_cmd_buffer(struct etna_cmd_stream *ctx_)
{
    struct etna_cmd_stream_priv *ctx = etna_cmd_stream_priv(ctx_);
    uint32_t start_offset = ctx->cmdbuf[ctx->base.cur_buf]->startOffset/4 + 8;
    uint32_t *buf = &ctx->base.buf[start_offset];
    size_t size = ctx->base.offset - start_offset;
    fprintf(stderr, "cmdbuf %u offset %u:\n", ctx->base.cur_buf, start_offset);
    for(unsigned idx=0; idx<size; idx+=8)
    {
        for (unsigned i=idx; i<size && i<idx+8; ++i)
        {
            fprintf(stderr, "%08x ", buf[i]);
        }
        fprintf(stderr, "\n");
    }
}

static uint32_t append_bo(struct etna_cmd_stream *stream, struct etna_bo *bo)
{
	struct etna_cmd_stream_priv *priv = etna_cmd_stream_priv(stream);
	uint32_t idx;

	idx = APPEND(priv, bos);
	priv->bos[idx] = etna_bo_ref(bo);
	return idx;
}

/* add (if needed) bo, return idx: */
static uint32_t bo2idx(struct etna_cmd_stream *stream, struct etna_bo *bo,
		uint32_t flags)
{
	struct etna_cmd_stream_priv *priv = etna_cmd_stream_priv(stream);
	uint32_t idx;

	pthread_mutex_lock(&idx_lock);

	if (!bo->current_stream) {
		idx = append_bo(stream, bo);
		bo->current_stream = stream;
		bo->idx = idx;
	} else if (bo->current_stream == stream) {
		idx = bo->idx;
	} else {
		/* slow-path: */
		for (idx = 0; idx < priv->nr_bos; idx++)
			if (priv->bos[idx] == bo)
				break;
		if (idx == priv->nr_bos) {
			/* not found */
			idx = append_bo(stream, bo);
		}
	}
	pthread_mutex_unlock(&idx_lock);
	return idx;
}

void etna_cmd_stream_reloc(struct etna_cmd_stream *cmdbuf, const struct etna_reloc *reloc)
{
    uint32_t gpuaddr = 0;
    if (reloc && reloc->bo) {
        gpuaddr = etna_bo_gpu_address(reloc->bo) + reloc->offset;
        uint32_t idx = bo2idx(cmdbuf, reloc->bo, reloc->flags);
#ifdef DEBUG_BO
        printf("%s: added bo %p as idx %d\n", __func__, reloc->bo, idx);
#endif
    }
    etna_cmd_stream_emit(cmdbuf, gpuaddr);
}

void etna_cmd_stream_ref(struct etna_cmd_stream *stream, struct etna_bo *bo)
{
    bo2idx(stream, bo, 0);
}

uint32_t etna_cmd_stream_timestamp(struct etna_cmd_stream *stream)
{
    struct etna_cmd_stream_priv *priv = etna_cmd_stream_priv(stream);
    return priv->submit_fence;
}

void etna_cmd_stream_flush(struct etna_cmd_stream *stream)
{
    etna_flush(stream);
}

void etna_cmd_stream_flush2(struct etna_cmd_stream *stream, int in_fence_fd,
			    int *out_fence_fd)
{
    /* in_fence_fd? */
    assert(in_fence_fd == -1);
    assert(!out_fence_fd);
    etna_flush(stream);
}
