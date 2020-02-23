#include "api.h"

enum msg_type api_msg_get_type(mpack_node_t msg_root)
{
	mpack_node_t node_type = mpack_node_array_at(msg_root, 0);
	if (mpack_node_is_nil(node_type)) {
		return NVIM_RPC_UNKNOWN;
	}
	return mpack_node_int(node_type);
}

bool api_parse_to_resp(mpack_node_t root, struct msg_response *resp)
{
	assert(parse_type(root) == NVIM_RPC_RESPONSE);

	mpack_error_t err;
	mpack_node_t node;

	// Extract msgid
	{
		node = mpack_node_array_at(root, 1);
		err = mpack_node_error(node);
		if (err != mpack_ok) {
			return false;
		}
		resp->msgid = mpack_node_u32(node);
	}

	// Extract err
	{
		node = mpack_node_array_at(root, 2);
		err = mpack_node_error(node);
		if (err != mpack_ok) {
			return false;
		}
		resp->error = node;
	}

	// Extract result
	{
		node = mpack_node_array_at(root, 3);
		err = mpack_node_error(node);
		if (err != mpack_ok) {
			return false;
		}
		resp->result = node;
	}

	return true;
}

size_t api_notif_method_len(mpack_node_t root)
{
	assert(parse_type(root) == NVIM_RPC_NOTIFICATION);
	mpack_error_t err;
	mpack_node_t node;

	node = mpack_node_array_at(root, 1);
	err = mpack_node_error(node);
	if (err != mpack_ok) {
		return 0;
	}
	return mpack_node_strlen(node) + 1;
}

bool api_parse_to_notif(mpack_node_t root, struct msg_notification *notif,
			char *method_buffer, size_t method_buff_sz)
{
	assert(parse_type(root) == NVIM_RPC_NOTIFICATION);

	mpack_error_t err;
	mpack_node_t node;

	// Extract method
	{
		node = mpack_node_array_at(root, 1);
		err = mpack_node_error(node);
		if (err != mpack_ok) {
			return false;
		}

		mpack_node_copy_cstr(node, method_buffer, method_buff_sz);
		err = mpack_node_error(node);
		if (err != mpack_ok) {
			return false;
		}
	}

	// Extract params
	{
		node = mpack_node_array_at(root, 2);
		err = mpack_node_error(node);
		if (err != mpack_ok) {
			return false;
		}
		notif->params = node;
	}

	return true;
}
