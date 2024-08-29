#include "sdkconfig.h"

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_boot)


MEMORY
{
    MEMORY_SPACE (rwx) : ORIGIN = CONFIG_IMAGE_LOAD_ADDRESS, LENGTH = CONFIG_IMAGE_MAX_LENGTH
}

SECTIONS
{
    .vectors : {
        *(.boot)
        . = ALIGN(64);
        KEEP(*(.vectors))

    } > MEMORY_SPACE

    .text : {
        *(.text*)
        *(.gnu.linkonce.t.*)
        *(.plt)
        *(.gnu_warning)
        *(.gcc_execpt_table)
        *(.glue_7)               /* glue arm to thumb code */
        *(.glue_7t)              /* glue thumb to arm code */        
        *(.vfp11_veneer)
        *(.ARM.extab)
        *(.gnu.linkonce.armextab.*)
        *(.note.gnu.build-id)
     } > MEMORY_SPACE /* .text should linked to MEMORY_SPACE */
    . = ALIGN(8);

    .interp : {
        KEEP (*(.interp))
    } > MEMORY_SPACE

    .note-ABI-tag : {
        KEEP (*(.note-ABI-tag))
    } > MEMORY_SPACE

    .rodata :{
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
        __rodata_end = .;
    } > MEMORY_SPACE

    .rodata1 : {
        __rodata1_start = .;
        *(.rodata1)
        *(.rodata1.*)
        __rodata1_end = .;
    } > MEMORY_SPACE

    /* 定义 amp_img section */
    .amp_img :
    {
        amp_img_start = .;
        KEEP(*(.my_amp_img))
        amp_img_end = .;
    } > MEMORY_SPACE

    #include "image_in.ld"

    .le_shell  (ALIGN(64)):{
        . = ALIGN(4096);
        _shell_command_start = .;
        KEEP (*(shellCommand))
        _shell_command_end = .;
        . = ALIGN(4096); /* align to page size */
        _shell_code_end = .;
    } > MEMORY_SPACE
#ifdef  CONFIG_USE_CHERRY_USB
    .cherryusb (ALIGN(64)):{
        /* section information for usbh class */
        . = ALIGN(4);
        __usbh_class_info_start__ = .;
        KEEP(*(.usbh_class_info))
        __usbh_class_info_end__ = .;
    }> MEMORY_SPACE
#endif
    .init :{
        KEEP (*(SORT_NONE(.init)))
    } > MEMORY_SPACE

    .fini :{
        KEEP (*(SORT_NONE(.fini)))
    } > MEMORY_SPACE

    .eh_frame :{
        KEEP (*(.eh_frame))
    } > MEMORY_SPACE

    .ARM.extab :{
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > MEMORY_SPACE

    .ARM.exidx :{
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > MEMORY_SPACE

    /* to support c++ global variable constructor */
    .init_array : {
        . = ALIGN(8);
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;
    } > MEMORY_SPACE

    /* to support c++ global variable de-constructor */
    .fini_array :{
        . = ALIGN(8);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array ))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } > MEMORY_SPACE

    .dtors :{
        KEEP (*crtbegin.o(.dtors))
        KEEP (*crtbegin?.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
        . = ALIGN(16);
    } > MEMORY_SPACE
    
    .gcc_except_table : {
        . = ALIGN(8);
        *(.gcc_except_table)
        . = ALIGN(16);
    } > MEMORY_SPACE

    /* data segment, contains data that can be modified by the program at run-time,
       global and static variables that have a non-zero initial value will normally go here */
    .data : {
        __data_start__ = .; /* .data section will be redise at runtime */
        *(.data)
        . = ALIGN(4096); /* align to page size */
        __data_end__ = .;  
        __rom_end__ = .;      
    } > MEMORY_SPACE /* link it all to RAM */

    /* unitialized data segment, variables explicitly initialized with zero to BSS */
    .bss (NOLOAD): {
        __bss_start__ = .;
        _bss_start = __bss_start__;        
        *(.bss)
        __bss_end__ = .;
        _bss_end = __bss_end__;                
    } > MEMORY_SPACE

    .sbss (NOLOAD) : {
    __sbss_start__ = .;
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    __sbss_end__ = .;
    } > MEMORY_SPACE

    .heap (NOLOAD):{
        . = ALIGN(16);
        __end__ = .;
        PROVIDE(end = .);
        HeapBase = .;
        _heap_start = .;
        . += CONFIG_HEAP_SIZE* 1M;
        _heap_end = .;
        HeapLimit = .; /* Add for _sbrk */
    } > MEMORY_SPACE

    .stack (NOLOAD):{

        . = ALIGN(16); 
        _supervisor_stack_end = .;
        . += CONFIG_SVC_STACK_SIZE;
        _supervisor_stack = .;
        __supervisor_stack = _supervisor_stack;

        . = ALIGN(16);
        _irq_stack_end = .;
        . += CONFIG_IRQ_STACK_SIZE;
        __irq_stack = .;

        __sys_stack = .;
        . += CONFIG_SYS_STACK_SIZE;
        . = ALIGN(16);
        __sys_stack = .;

        _abort_stack_end = .;
        . += CONFIG_ABORT_STACK_SIZE;
        . = ALIGN(16);
        __abort_stack = .;

        _fiq_stack_end = .;
        . += CONFIG_FIQ_STACK_SIZE;
        . = ALIGN(16);
        __fiq_stack = .;

        _undef_stack_end = .;
        . += CONFIG_UNDEF_STACK_SIZE;
        . = ALIGN(16);
        __undef_stack = .;

    } > MEMORY_SPACE

#ifdef  CONFIG_USE_OPENAMP
#include "ft_openamp.ld"
#endif
}
