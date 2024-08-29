.global _freertos_vector_table
.global vSynchronousInterruptHandler
.global vSynchronousInterruptHandlerSPx
.global vSErrorInterruptHandler
.global FreeRTOS_IRQ_Handler


.section .freertos_vectors, "a"


_freertos_vector_table:
.set	FREERTOS_VBAR, _freertos_vector_table
.align 11 /* VBAR_EL1 Vector Base Address is [55:48]  */
.org(FREERTOS_VBAR)

	b	vSynchronousInterruptHandler

.org (FREERTOS_VBAR + 0x80)
	b	FreeRTOS_IRQ_Handler

.org (FREERTOS_VBAR + 0x100)
	b	.

.org (FREERTOS_VBAR + 0x180)
	b	vSErrorInterruptHandler

.org (FREERTOS_VBAR + 0x200)
	b	vSynchronousInterruptHandlerSPx

.org (FREERTOS_VBAR + 0x280)
	b	FreeRTOS_IRQ_Handler

.org (FREERTOS_VBAR + 0x300)
	b	.

.org (FREERTOS_VBAR + 0x380)
	b	vSErrorInterruptHandler		/* SError/vSError */

.org (FREERTOS_VBAR + 0x400)
	b	.

.org (FREERTOS_VBAR + 0x480)
	b	.

.org (FREERTOS_VBAR + 0x500)
	b	.

.org (FREERTOS_VBAR + 0x580)
	b	.

.org (FREERTOS_VBAR + 0x600)
	b	.

.org (FREERTOS_VBAR + 0x680)
	b	.

.org (FREERTOS_VBAR + 0x700)
	b	.

.org (FREERTOS_VBAR + 0x780)
	b	.

.org (FREERTOS_VBAR + 0x800)
