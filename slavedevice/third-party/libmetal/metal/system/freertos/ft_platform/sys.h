


#ifndef __METAL_SYS__H__
#define __METAL_SYS__H__

#ifdef __cplusplus
extern "C" {
#endif

#define METAL_INTERNAL

#if 1

#define XLNX_MAXIRQS XSCUGIC_MAX_NUM_INTR_INPUTS

static inline void sys_irq_enable(unsigned int vector)
{
	InterruptUmask(vector);
}

static inline void sys_irq_disable(unsigned int vector)
{
	InterruptMask(vector);
}

#endif /* METAL_INTERNAL */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_GENERIC_ZYNQMP_A53_SYS__H__ */
