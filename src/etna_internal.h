#ifndef H_ETNA_INTERNAL
#define H_ETNA_INTERNAL

#include "etnaviv_drmif.h"
#include "xf86atomic.h"
#include "viv.h"

struct etna_gpu {
    struct etna_device *dev;
    unsigned int core;
};

struct etna_pipe {
    struct etna_gpu *gpu;
    enum etna_pipe_id id;
};

enum etna_bo_type {
    ETNA_BO_TYPE_VIDMEM,    /* Main vidmem */
    ETNA_BO_TYPE_VIDMEM_EXTERNAL, /* Main vidmem, external handle */
    ETNA_BO_TYPE_USERMEM,   /* Mapped user memory */
    ETNA_BO_TYPE_CONTIGUOUS,/* Contiguous memory */
    ETNA_BO_TYPE_PHYSICAL,  /* Mmap-ed physical memory */
    ETNA_BO_TYPE_DMABUF     /* dmabuf memory */
};

/* Structure describing a block of video or user memory */
struct etna_bo {
    struct etna_device *conn;
    enum etna_bo_type bo_type;
    size_t size;
    enum viv_surf_type type;
    viv_node_t node;
    viv_addr_t address;
    void *logical;
    viv_usermem_t usermem_info;
    atomic_t        refcnt;
    /* in the common case, a bo won't be referenced by more than a single
     * command stream.  So to avoid looping over all the bo's in the
     * reloc table to find the idx of a bo that might already be in the
     * table, we cache the idx in the bo.  But in order to detect the
     * slow-path where bo is ref'd in multiple streams, we also must track
     * the current_stream for which the idx is valid.  See bo2idx().
     */
    struct etna_cmd_stream *current_stream;
    uint32_t idx;
};

#define NUM_COMMAND_BUFFERS 5

struct etna_cmdbuf {
    /* sync signal for command buffer */
    int sig_id;
    struct etna_bo *bo;
};

static inline struct etna_cmd_stream_priv *
etna_cmd_stream_priv(struct etna_cmd_stream *stream)
{
    return (struct etna_cmd_stream_priv *)stream;
}

struct etna_cmd_stream_priv {
    struct etna_cmd_stream base;
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
    /* track bo's used in current submit */
    struct etna_bo **bos;
    uint32_t nr_bos, max_bos;
};

#endif
