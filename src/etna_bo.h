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
/* etna: memory management functions */
#ifndef H_ETNA_BO
#define H_ETNA_BO

#include "etnaviv_drmif.h"

#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>

struct etna_queue;
struct etna_bo;

/** Missing in etnaviv_drmif.h */

int etna_bo_del_ext(struct etna_bo *mem, struct etna_queue *queue);

/* Map user memory (which may be write protected) into GPU memory space */
struct etna_bo *etna_bo_from_usermem_prot(struct etna_device *conn, void *memory, size_t size, int prot);

/* Map user memory into GPU memory space */
struct etna_bo *etna_bo_from_usermem(struct etna_device *conn, void *memory, size_t size);

/* Buffer object from framebuffer range */
struct etna_bo *etna_bo_from_fbdev(struct etna_device *conn, int fd, size_t offset, size_t size);

/* Temporary: get GPU address of buffer */
uint32_t etna_bo_gpu_address(struct etna_bo *bo);

#endif

