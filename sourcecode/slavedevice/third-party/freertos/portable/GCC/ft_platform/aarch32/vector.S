.globl _boot
.globl _vector_table

.globl FiqInterruptHandler
.globl DataAbortInterruptHandler
.globl PrefetchAbortInterruptHandler
.globl UndefineInterruptHandler
.globl FIQHandler


.set EXCEPTION_FRAME_SIZE, (17*4)

/* allocatable and executable */
.section .vectors, "a"

_vector_table:
    B	_boot
	B	Undefined
	B	FreeRTOS_SWI_Handler
	B	PrefetchAbortHandler
	B	DataAbortHandler
	NOP	/* Placeholder for address exception vector*/
	B	FreeRTOS_IRQ_Handler
	B	FIQHandler


.macro PushExceptionReg
    sub     sp, sp, #EXCEPTION_FRAME_SIZE         /* Sizeof(ExceptionFrame)  */
    stmia   sp, {r0 - r12}          /* Calling r0-r12                  */
    mov     r0, sp
    mrs     r6, spsr                /* Save CPSR                       */
    str     lr, [r0, #15*4]         /* Push PC                         */
    str     r6, [r0, #16*4]         /* Push CPSR                       */
    cps     #0x13                   /* C运行环境所运行的模式             */
    str     sp, [r0, #13*4]         /* Save calling SP                 */
    str     lr, [r0, #14*4]         /* Save calling PC                 */
.endm


Undefined:  /* Undefined handler */
    PushExceptionReg
    cps #0x1B
    bl UndefineInterruptHandler
    bl .

PrefetchAbortHandler:
    PushExceptionReg
    bl PrefetchAbortInterruptHandler
    bl .

DataAbortHandler:
    PushExceptionReg
    bl DataAbortInterruptHandler
    bl .

FIQHandler:					/* FIQ vector handler */
    PushExceptionReg
    bl FiqInterruptHandler
    bl .