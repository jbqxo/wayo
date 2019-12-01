#pragma once

#include <threads.h>

#include <uv.h>

typedef uint32_t nvim_rpc_msgid;
struct nvim_api {
  nvim_rpc_msgid msg_counter;
  mtx_t msg_counter_lock;

  uv_stream_t *in;
  uv_stream_t *out;

  void (*destroy_func)(struct nvim_api *);
};

// See: https://github.com/msgpack-rpc/msgpack-rpc/blob/e6a28c4b71638b61ea11469917b030df45ef8081/spec.md
enum nvim_rpc_type {
  NVIM_RPC_REQUEST = 0x0,
  NVIM_RPC_RESPONSE = 0x1,
  NVIM_RPC_NOTIFICATION = 0x2,
};

void nvim_init(struct nvim_api *);
void nvim_destroy(struct nvim_api *);
void nvim_connect_stdio(uv_loop_t *, struct nvim_api *);
void nvim_on_read(uv_stream_t *, ssize_t nread, const uv_buf_t *);
nvim_rpc_msgid nvim_next_msgid(struct nvim_api *api);

