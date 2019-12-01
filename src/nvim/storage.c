#include <assert.h>

#include "storage.h"
#include "util.h"

static size_t hash(struct request_storage *storage, nvim_rpc_msgid id) { 
  return id % storage->capacity;
}

void request_storage_init(struct request_storage *storage, int capacity) {
  assert(storage);
  assert(capacity > 0);

  storage->data = xmalloc(capacity * sizeof(*storage->data));
  storage->capacity = capacity;
  storage->length = 0;
  // TODO(Maxim Lyapin): Make the mutex plain.
  mtx_init(&storage->lock, mtx_recursive);

  for (size_t request_idx = 0; request_idx < capacity; request_idx++) {
    storage->data[request_idx].id = -1;
  }
}

void request_storage_destroy(struct request_storage *storage) {
  assert(storage);

  mtx_lock(&storage->lock);

  for (size_t request_idx = 0; request_idx < storage->capacity; request_idx++) {
    struct request *req = &storage->data[request_idx];
    assert(req);

    if (req->id >= 0) {
      request_storage_free_req(storage, request_idx);
    }
  }

  xfree(storage->data);

  mtx_unlock(&storage->lock);
  mtx_destroy(&storage->lock);
}

enum storage_rc request_storage_create_req(struct request_storage *storage,
                                           nvim_rpc_msgid id,
                                           request_callback callb,
                                           struct request **result) {
  assert(storage);

  size_t hash_key = hash(storage, id);

  mtx_lock(&storage->lock);

  if (storage->data[hash_key].id >= 0) {
    if (result != NULL) {
      *result = NULL;
    }
    if (storage->data[hash_key].msgid == id) {
      mtx_unlock(&storage->lock);
      return STORAGE_RC_DUPLICATE;
    } else {
      mtx_unlock(&storage->lock);
      return STORAGE_RC_NOSPACE;
    }
  }
  uv_write_t *uvreq = xmalloc(sizeof(*uvreq));
  storage->data[hash_key] =
      (struct request){.callb = callb, .uvreq = uvreq, .id = (int64_t)id};
  storage->length++;

  if (result != NULL) {
    *result = &storage->data[hash_key];
  }
  mtx_unlock(&storage->lock);
  return STORAGE_RC_OK;
}

enum storage_rc request_storage_find(struct request_storage *storage,
                                     nvim_rpc_msgid id,
                                     struct request **result) {
  assert(storage);
  assert(result);

  size_t hash_key = hash(storage, id);

  mtx_lock(&storage->lock);
  struct request *req = &storage->data[hash_key];

  if (req->id < 0 || req->msgid != id) {
    mtx_unlock(&storage->lock);
    return STORAGE_RC_NOTFOUND;
  }
  *result = req;
  mtx_unlock(&storage->lock);

  return STORAGE_RC_OK;
}

enum storage_rc request_storage_free_req(struct request_storage *storage,
                                         nvim_rpc_msgid id) {
  assert(storage);
  size_t hash_key = hash(storage, id);

  mtx_lock(&storage->lock);
  struct request *req = &storage->data[hash_key];

  if (req->id < 0 || req->msgid != id) {
    mtx_unlock(&storage->lock);
    return STORAGE_RC_NOTFOUND;
  }

  req->id = -1;
  xfree(req->uvreq);
  req->uvreq = NULL;
  req->callb = NULL;

  mtx_unlock(&storage->lock);
  return STORAGE_RC_OK;
}
