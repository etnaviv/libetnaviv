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

/* Buffer building and submission, abstracts away specific kernel interface
 * as much as practically possible.
 */
#ifndef H_ETNA
#define H_ETNA

#include "etnaviv_drmif.h"

#include <etna_util.h>
#include <viv.h>

#include <stdint.h>
#include <stdlib.h>
#ifdef DEBUG
#include <stdio.h>
#endif
#include <string.h> /* for memcpy */

/* Special command buffer ids */
#define ETNA_NO_BUFFER (-1)

/* Constraints to command buffer layout:
 *
 * - Keep 8 words (32 bytes) at beginning of commit (for kernel to add optional PIPE switch)
 * - Keep 6 words (24 bytes) between end of one commit and beginning of next, or at the end of a buffer (for kernel to add LINK)
 * These reserved areas can be left uninitialized, as they are written by the kernel.
 *
 * Synchronization:
 *
 * - Create N command buffers, with a signal for each buffer
 * - Before starting to write to a buffer, make sure it is free by waiting for buffer's sync signal
 * - After a buffer is full, queue the buffer's sync signal and switch to next buffer
 *
 */
#define BEGIN_COMMIT_CLEARANCE 32
#define END_COMMIT_CLEARANCE 32

/** Structure definitions */

/* Etna error (return) codes */
enum etna_status {
    ETNA_OK             = 0,    /* = VIV_STATUS_OK */
    ETNA_INVALID_ADDR   = 1000, /* Don't overlap with VIV_STATUS_* */
    ETNA_INVALID_VALUE  = 1001,
    ETNA_OUT_OF_MEMORY  = 1002,
    ETNA_INTERNAL_ERROR = 1003,
    ETNA_ALREADY_LOCKED = 1004
};

struct _gcoCMDBUF;
struct etna_queue;
struct etna_cmd_stream;
struct etna_bo;

/* Create new etna context.
 * Return error when creation fails.
 */
int etna_create(struct etna_device *conn, struct etna_cmd_stream **ctx);

/* Send currently queued commands to kernel.
 * @return OK on success, error code otherwise
 */
int etna_flush(struct etna_cmd_stream *ctx);

/* print command buffer for debugging */
void etna_dump_cmd_buffer(struct etna_cmd_stream *ctx);

#endif
