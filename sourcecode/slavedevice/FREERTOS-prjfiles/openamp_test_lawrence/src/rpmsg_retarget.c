/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <metal/mutex.h>
#include <metal/spinlock.h>
#include <metal/utilities.h>
#include <openamp/open_amp.h>
#include <openamp/rpmsg_retarget.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

/*************************************************************************
 *	Description
 *	This file contains rpmsg based redefinitions for C RTL system calls
 *	such as _open, _read, _write, _close.
 *************************************************************************/
static struct rpmsg_rpc_data *rpmsg_default_rpc;

static int rpmsg_rpc_ept_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			    uint32_t src, void *priv)
{
	struct rpmsg_rpc_syscall *syscall;

	(void)priv;
	(void)src;

	if (data && ept) {
		syscall = data;
		if (syscall->id == TERM_SYSCALL_ID) {
			rpmsg_destroy_ept(ept);
		} else {
			struct rpmsg_rpc_data *rpc;

			rpc = metal_container_of(ept,
						 struct rpmsg_rpc_data,
						 ept);
			metal_spinlock_acquire(&rpc->buflock);
			if (rpc->respbuf && rpc->respbuf_len != 0) {
				if (len > rpc->respbuf_len)
					len = rpc->respbuf_len;
				memcpy(rpc->respbuf, data, len);
			}
			atomic_flag_clear(&rpc->nacked);
			metal_spinlock_release(&rpc->buflock);
		}
	}

	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	struct rpmsg_rpc_data *rpc;

	rpc = metal_container_of(ept, struct rpmsg_rpc_data, ept);
	rpc->ept_destroyed = 1;
	rpmsg_destroy_ept(ept);
	atomic_flag_clear(&rpc->nacked);
	if (rpc->shutdown_cb)
		rpc->shutdown_cb(rpc);
}

int rpmsg_rpc_init(struct rpmsg_rpc_data *rpc,
		   struct rpmsg_device *rdev,
		   const char *ept_name, uint32_t ept_addr,
		   uint32_t ept_raddr,
		   void *poll_arg, rpmsg_rpc_poll poll,
		   rpmsg_rpc_shutdown_cb shutdown_cb)
{
	int ret;

	if (!rpc || !rdev)
		return -EINVAL;
	metal_spinlock_init(&rpc->buflock);
	metal_mutex_init(&rpc->lock);
	rpc->shutdown_cb = shutdown_cb;
	rpc->poll_arg = poll_arg;
	rpc->poll = poll;
	rpc->ept_destroyed = 0;
	rpc->respbuf = NULL;
	rpc->respbuf_len = 0;
	atomic_init(&rpc->nacked, 1);
	ret = rpmsg_create_ept(&rpc->ept, rdev,
			       ept_name, ept_addr, ept_raddr,
			       rpmsg_rpc_ept_cb, rpmsg_service_unbind);
	if (ret != 0) {
		metal_mutex_release(&rpc->lock);
		return -EINVAL;
	}
	while (!is_rpmsg_ept_ready(&rpc->ept)) {
		if (rpc->poll)
			rpc->poll(rpc->poll_arg);
	}
	return 0;
}

void rpmsg_rpc_release(struct rpmsg_rpc_data *rpc)
{
	if (!rpc)
		return;
	if (rpc->ept_destroyed == 0)
		rpmsg_destroy_ept(&rpc->ept);
	metal_mutex_acquire(&rpc->lock);
	metal_spinlock_acquire(&rpc->buflock);
	rpc->respbuf = NULL;
	rpc->respbuf_len = 0;
	metal_spinlock_release(&rpc->buflock);
	metal_mutex_release(&rpc->lock);
	metal_mutex_deinit(&rpc->lock);
}

int rpmsg_rpc_send(struct rpmsg_rpc_data *rpc,
		   void *req, size_t len,
		   void *resp, size_t resp_len)
{
	int ret;

	if (!rpc)
		return -EINVAL;
	metal_spinlock_acquire(&rpc->buflock);
	rpc->respbuf = resp;
	rpc->respbuf_len = resp_len;
	metal_spinlock_release(&rpc->buflock);
	(void)atomic_flag_test_and_set(&rpc->nacked);
	ret = rpmsg_send(&rpc->ept, req, len);
	if (ret < 0)
		return -EINVAL;
	if (!resp)
		return ret;
	while ((atomic_flag_test_and_set(&rpc->nacked))) {
		if (rpc->poll)
			rpc->poll(rpc->poll_arg);
	}
	return ret;
}

void rpmsg_set_default_rpc(struct rpmsg_rpc_data *rpc)
{
	if (!rpc)
		return;
	rpmsg_default_rpc = rpc;
}
