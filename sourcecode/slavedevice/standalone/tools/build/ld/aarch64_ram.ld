#include "sdkconfig.h"

MEMORY
{
    MEMORY_SPACE (rwx) : ORIGIN = CONFIG_IMAGE_LOAD_ADDRESS, LENGTH = CONFIG_IMAGE_MAX_LENGTH
}

ENTRY(_vector_table)



SECTIONS
{
    .text : {
        KEEP (*(.vectors))
        *(.boot)
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)
        *(.plt)
        *(.gnu_warning)
        *(.gcc_execpt_table)
        *(.glue_7)
        *(.glue_7t)
        *(.ARM.extab)
        *(.gnu.linkonce.armextab.*)
    } > MEMORY_SPACE

    .got : {
    *(.got)
    } > MEMORY_SPACE

    .got1 : {
    *(.got1)
    } > MEMORY_SPACE

    .got2 : {
    *(.got2)
    } > MEMORY_SPACE

    .note.gnu.build-id : {
    KEEP (*(.note.gnu.build-id))
    } > MEMORY_SPACE

    .init (ALIGN(64)): {
    KEEP (*(.init))
    } > MEMORY_SPACE

    .fini (ALIGN(64)): {
    KEEP (*(.fini))
    } > MEMORY_SPACE

    .interp : {
    __interp_start = .;
    KEEP (*(.interp))
    __interp_end = .;
    } > MEMORY_SPACE

    .note-ABI-tag : {
    __note-ABI-tag_start = .;
    KEEP (*(.note-ABI-tag))
    __note-ABI-tag_end = .;
    } > MEMORY_SPACE

    .preinit_array : {
    __preinit_array_start = .;
    KEEP (*(SORT(.preinit_array.*)))
    KEEP (*(.preinit_array))
    __preinit_array_end = .;
    } > MEMORY_SPACE

    .init_array : {
    __init_array_start = .;
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    __init_array_end = .;
    } > MEMORY_SPACE
    
    .fini_array : {
    __fini_array_start = .;
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array))
    __fini_array_end = .;
    } > MEMORY_SPACE

    /* used in exception handling */
    .ARM.exidx : {
    __exidx_start = .;
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    __exidx_end = .;
    } > MEMORY_SPACE

    .rodata (ALIGN(64)): {
    __rodata_start = .;
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
    __rodata_end = .;
    } > MEMORY_SPACE

    .le_shell  (ALIGN(64)):{
        . = ALIGN(4096);
        _shell_command_start = .;
        KEEP (*(shellCommand))
        _shell_command_end = .;
        . = ALIGN(4096); /* align to page size */
        _shell_code_end = .;
    }> MEMORY_SPACE
#ifdef  CONFIG_USE_CHERRY_USB
    .cherryusb (ALIGN(64)):{
        /* section information for usbh class */
        . = ALIGN(4);
        __usbh_class_info_start__ = .;
        KEEP(*(.usbh_class_info))
        __usbh_class_info_end__ = .;
    }> MEMORY_SPACE
#endif
    .rodata1 (ALIGN(64)): {
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

    .gcc_except_table : {
    *(.gcc_except_table)
    . = ALIGN(16);
    __rom_end__ = .;
    } > MEMORY_SPACE


    .data :{
        __data_start__ = .;
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)
        *(.jcr)
        *(.got)
        *(.got.plt)
        . = ALIGN(4096); /* align to page size */
        __data_end__ = .;
    } > MEMORY_SPACE

    .sbss (NOLOAD) : {
    __sbss_start__ = .;
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    __sbss_end__ = .;
    } > MEMORY_SPACE

    .bss (NOLOAD) : {
    . = ALIGN(64);
    __bss_start__ = .;
    *(.bss)
    *(.bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(64);
    __bss_end__ = .;
    } > MEMORY_SPACE


    .heap (NOLOAD):{
        . = ALIGN(64);
        _heap = .;
        HeapBase = .;
        _heap_start = .;
        . = . + CONFIG_HEAP_SIZE* 1M;
        _heap_end = .;
        HeapLimit = .;
    } > MEMORY_SPACE

    .fpu_stack(NOLOAD):{
        . = ALIGN(64);
        _fpu_stack_end = .;
        . += CONFIG_FPU_STACK_SIZE;
        __fpu_stack = .;
    } > MEMORY_SPACE

    .stack (NOLOAD) : {
        . = ALIGN(64);
        _el1_stack_end = .;
        . += CONFIG_STACK_SIZE;
        __el1_stack = .;
    } > MEMORY_SPACE

#ifdef  CONFIG_USE_OPENAMP
#include "ft_openamp.ld"
#endif
    
}
