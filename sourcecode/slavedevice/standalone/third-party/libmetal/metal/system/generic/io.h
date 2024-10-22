/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	generic/io.h
 * @brief	Generic specific io definitions.
 */

#ifndef __METAL_IO__H__
#error "Include metal/io.h instead of metal/generic/io.h"
#endif

#ifndef __METAL_GENERIC_IO__H__
#define __METAL_GENERIC_IO__H__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if 1

/**
 * @brief memory mapping for an I/O region
 */
void metal_sys_io_mem_map(struct metal_io_region *io);

/**
 * @brief memory mapping
 */
void *metal_machine_io_mem_map(void *va, metal_phys_addr_t pa,
			       size_t size, unsigned int flags);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __METAL_GENERIC_IO__H__ */
