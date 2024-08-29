
/* 定义 image_info section */
.image_info :
{
    img_start = .;
    KEEP(*(.my_image_info))
    img_end = .;
} > MEMORY_SPACE

