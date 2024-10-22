/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	generic/zynqmp_a53/sys.h
 * @brief	generic zynqmp_a53 system primitives for libmetal.
 */

#ifndef __METAL_GENERIC_SYS__H__
#error "Include metal/sys.h instead of metal/generic/ft_platform/sys.h"
#endif

#include <metal/system/generic/xlnx_common/sys.h>
#include "xscugic.h"

#ifndef __METAL_GENERIC_ZYNQMP_A53_SYS__H__
#define __METAL_GENERIC_ZYNQMP_A53_SYS__H__

#ifdef __cplusplus
extern "C" {
#endif

#if 1

#define XLNX_MAXIRQS XSCUGIC_MAX_NUM_INTR_INPUTS

static inline void sys_irq_enable(unsigned int vector)
{
	XScuGic_EnableIntr(XPAR_SCUGIC_0_DIST_BASEADDR, vector);
}

static inline void sys_irq_disable(unsigned int vector)
{
	XScuGic_DisableIntr(XPAR_SCUGIC_0_DIST_BASEADDR, vector);
}

#endif /* METAL_INTERNAL */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_GENERIC_ZYNQMP_A53_SYS__H__ */
