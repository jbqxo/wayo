/* Copyright (c) 2019 Maxim Lyapin 
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy 
 *  of this software and associated documentation files the (Software""), to deal 
 *  in the Software without restriction, including without limitation the rights 
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 *  copies of the Software, and to permit persons to whom the Software is 
 *  furnished to do so, subject to the following conditions: 
 *   
 *  The above copyright notice and this permission notice shall be included in all 
 *  copies or substantial portions of the Software. 
 *   
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 *  SOFTWARE.)
 */

#pragma once

#include <stdint.h>

#include <mpack.h>

// Protocol Specs: https://github.com/msgpack-rpc/msgpack-rpc/blob/e6a28c4b71638b61ea11469917b030df45ef8081/spec.md
enum msg_type {
	NVIM_RPC_REQUEST = 0x0,
	NVIM_RPC_RESPONSE = 0x1,
	NVIM_RPC_NOTIFICATION = 0x2,
};

struct msg_request {
	uint32_t msgid;
	char *method;
	mpack_node_t params;
};

struct msg_response {
	uint32_t msgid;
	mpack_node_t error;
	mpack_node_t result;
};

struct msg_notification {
	char *method;
	mpack_node_t params;
};
