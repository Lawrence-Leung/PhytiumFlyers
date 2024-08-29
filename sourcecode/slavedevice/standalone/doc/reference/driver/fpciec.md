# FPCIEC 驱动程序

## 1. 概述

- 本驱动是针对E2000/D2000 中PCIE 控制器而设计，主要为ECAM ( Enhanced Configuration Access Mechanism) 之外的特性提供软件支持。开发者需要结合硬件原理图，确认对应的solt具体是哪一个PCIE控制器，方能正常使用本驱动模块。
- 本驱动主要实现了以下功能特性：

1. 支持PCIE 控制器工作与ep、rc 两种工作状态下
2. ep 支持 setbar、set config header 、map rc memory  等特性
3. 支持dma 传输与dma 中断

## 2. 功能

- 驱动相关的源文件如下
- drivers/pcie/fpciec

```
.
├── fpciec.c
├── fpciec.h
├── fpciec_dma.c
├── fpciec_dma.h
├── fpciec_ep.c
├── fpciec_g.c
├── fpciec_hw.h
├── fpciec_misc.c
└── fpciec_sinit.c
```

## 3. 配置方法

此模块可以按照以下步骤进行初始化与使用：

1. 使用FPcieCCfgInitialize 接口完成实例初始化
2. 如果为需要将控制器配置为endpoint,还需要调用FPcieCEpPioInit接口 ,确定endpoint能够映射的outbound范围

## 4. 应用示例

### [fpcie_ecam](../../../example/peripherals/pcie/)

## 5. API 数据结构

- E2000宏定义与文档中控制器的对应关系

| 宏定义                 | 控制器名 |
| ---------------------- | -------- |
| FPCIEC_INSTANCE0_INDEX | psu.c0   |
| FPCIEC_INSTANCE1_INDEX | psu.c1   |
| FPCIEC_INSTANCE2_INDEX | peu.c0   |
| FPCIEC_INSTANCE3_INDEX | peu.c1   |
| FPCIEC_INSTANCE4_INDEX | peu.c2   |
| FPCIEC_INSTANCE5_NUM   | peu.c3   |


### 5.1 数据结构

```c
/* A typedef for a callback function which takes a miscellaneous type and some arguments */
typedef void (*FPcieCMiscIrqCallBack)(u32 misc_type, void *args);

/* Enumeration of possible PCIe interrupts */
typedef enum
{
    FPCIE_INTERRUPT_UNKNOWN,
    FPCIE_INTERRUPT_INTA,
    FPCIE_INTERRUPT_INTB,
    FPCIE_INTERRUPT_INTC,
    FPCIE_INTERRUPT_INTD,
} FPcieCIntxNum;

typedef boolean bool;

/* Enumeration for Base Address Register (BAR) numbers in PCIe */
typedef enum
{
    FPCIE_NO_BAR = -1,
    FPCIE_BAR_0,
    FPCIE_BAR_1,
    FPCIE_BAR_2,
    FPCIE_BAR_3,
    FPCIE_BAR_4,
    FPCIE_BAR_5,
} FPcieCBarNo;

/* Structure defining a BAR in PCIe End Point */
typedef struct
{
    uintptr_t phys_addr;           /* Physical address of the BAR */
    uintptr_t virt_addr;           /* Virtual address of the BAR */
    FPcieCSize size;             /* Size of the BAR */
    FPcieCBarNo bar_no;            /* BAR number */
    u8 flags;                      /* Flags indicating type of memory (1 for 32bit mem, 2 for prefetch mem) */
} FPcieCEpBar;

/* Structure defining the header of a PCIe End Point */
typedef struct
{
    u16 vendor_id;
    u16 device_id;
    u8  revid;                     /* Revision ID */
    u8  progif_code;               /* Programming Interface Code */
    u8  subclass_code;             /* Subclass Code */
    u8  baseclass_code;            /* Base Class Code */
    u8  cache_line_size;           /* Cache Line Size */
    u16 subsys_vendor_id;          /* Subsystem Vendor ID */
    u16 subsys_id;                 /* Subsystem ID */
    FPcieCIntxNum interrupt_pin;   /* Interrupt Pin being used */
} FPcieCEpHeader;

/* Structure defining configuration of the PCIe device */
typedef struct
{
    u32 instance_id;               /* ID of the device */

    uintptr_t config_index;
    uintptr_t config_space;        /* Base address for the config space (hbp base) */
    uintptr_t control_space;       /* Base address for the control (control base) */

    uintptr_t dma_engine_base;     /* Base address for DMA engine */
    u32 dma_max_num;               /* Maximum number for DMA */

    u32 misc_irq_num;              /* Number of miscellaneous interrupts */
} FPcieCConfig;

/* Structure defining PCIe End Point memory ranges */
typedef struct 
{
    uintptr_t pref_start;          /* Start address of prefetch memory */
    uintptr_t pref_end;            /* End address of prefetch memory */
    uintptr_t mem_start;           /* Start address of regular memory */
    uintptr_t mem_end;             /* End address of regular memory */
} FPcieCEpPioMemRange;

/* Main structure defining the PCIe configuration */
typedef struct
{
    u32 is_ready;                  /* Flag indicating if the device is initialized and ready */
    FPcieCConfig config;           /* Configuration of the device */

    /* Miscellaneous interrupt */
    FPcieCMiscIrqCallBack misc_irq; /* Callback for DMA engine interrupts */
    void *args;                    /* Arguments to be passed to the callback */

    /* Set map */
    uintptr_t outbound_region_p[FPCIEC_MAX_OUTBOUND_NUM]; /* Outbound region pointers */
    unsigned long outbound_region_map;                    /* Map of the outbound region */

} FPcieC;


```

### 5.2 错误码定义

- FT_SUCCESS 成功
- FPCIEC_ENDPOINT_BOUND_SELECT_OVERRANGE PCIe端点的边界选择超出了允许的范围
- FPCIEC_MISC_IRQ_OUTRANGE PCIe的杂项中断数量超出了允许的范围

### 5.3 用户API接口

#### FPcieCCfgInitialize

- 使用提供的配置初始化FPcieC实例。

```c
FError FPcieCCfgInitialize(FPcieC *instance_p, FPcieCConfig *config_p)
```

Note:

- `FPcieCCfgInitialize`函数主要负责使用提供的配置初始化 `FPcieC`结构的实例。断言所提供的指针的有效性，清除 `FPcieC`实例中的旧值，然后将新配置复制到实例中。
- 复制配置后，函数将 `FPcieC`实例的 `is_ready`标志设置为 `FT_COMPONENT_IS_READY`，表示此实例现在已准备好使用。

Input:

- `FPcieC *instance_p`：这是指向您要初始化的 `FPcieC`实例的指针。它指向存储FPcieC结构的内存位置。
- `FPcieCConfig *config_p`：这是指向配置结构 `FPcieCConfig`的指针，其中包含初始化 `FPcieC`实例的配置详细信息。

Return:

- `FError`：如果初始化过程成功，则返回 `FT_SUCCESS`。如果在该函数的更完整版本中还有其他潜在的错误条件，它们会由其他错误代码表示。

#### FPcieCLookupConfig

- 查找并检索特定实例ID的配置结构。

```c
FPcieCConfig *FPcieCLookupConfig(u32 instance_id)
```

Note:

- `FPcieCLookupConfig`函数主要负责查找与给定的 `instance_id`匹配的配置。它遍历所有可能的配置实例，并检查其中的 `instance_id`与传入的ID是否匹配。
- 如果找到了匹配的配置，函数会返回对应配置的指针。如果没有找到，它会返回NULL。

Input:

- `u32 instance_id`：您要检索其配置的实例的实例ID。

Return:

- `FPcieCConfig *`：返回指向配置结构的指针，如果未找到则返回NULL。

#### FPcieCEpPioInit

- 为端点初始化PIO内存范围。

```c
void FPcieCEpPioInit(FPcieC *instance_p, FPcieCEpPioMemRange * mem_range)
```

Note:

- `FPcieCEpPioInit`函数主要负责初始化PCIe端点的PIO(程序输入/输出)内存范围。此决定的返回将会影响 FPcieCMapEpAddr 使用的范围

Input:

- `FPcieC *instance_p`：指向FPcieC实例的指针。
- `FPcieCEpPioMemRange *mem_range`：指向内存范围结构的指针。

#### FPcieCUnmapEpAddr

- 在EP功能的上下文中取消地址映射。

```c
FError FPcieCUnmapEpAddr(FPcieC *instance_p, u32 fun_num, uintptr_t addr)
```

Note:

- `FPcieCUnmapEpAddr`函数主要负责取消PCIe端点函数的特定地址映射。

Input:

- `FPcieC *instance_p`：指向FPcieC实例的指针。
- `u32 fun_num`：EP功能号。
- `uintptr_t addr`：要取消映射的地址。

Return:

- `FError`：成功返回FT_SUCCESS，否则返回错误码。

#### FPcieCMapEpAddr

- 将EP地址映射到RC地址以进行数据传输。

```c
FError FPcieCMapEpAddr(FPcieC *instance_p, u32 fun_num, uintptr_t ep_pci_addr, u64 rc_addr, FPcieCSize size)
```

Note:

- `FPcieCMapEpAddr`函数主要用于设置必要的配置，以将EP地址映射到RC地址，从而实现EP和RC之间的数据传输。

Input:

- `FPcieC *instance_p`：指向FPcieC实例的指针。
- `u32 fun_num`：EP的功能号。
- `uintptr_t ep_pci_addr`：要映射的EP PCI地址。
- `u64 rc_addr`：要映射到的RC地址。
- `FPcieCSize size`：映射的大小。

Return:

- `FT_SUCCESS`：如果映射成功，否则返回一个错误码。

#### FPcieCClearEpBar

- 清除Endpoint的基地址寄存器 (BAR) 配置。

```c
FError FPcieCClearEpBar(FPcieC *instance_p, u32 fun_num, FPcieCBarNo bar_no)
```

Note:

- `FPcieCClearEpBar`函数用于清除Endpoint的基地址寄存器 (BAR) 配置。
- 函数首先进行断言检查以确保传入的 `instance_p`不为空，并且其配置中的 `control_space`也不为空。

Input:

- `FPcieC *instance_p`：指向FPcieC实例的指针。
- `u32 fun_num`：EP的功能号。
- `FPcieCBarNo bar_no`：要清除的BAR号。

Return:

- `FT_SUCCESS`：成功清除时返回，否则返回一个错误码。

#### FPcieCSetEpBar

- 在PCIeC 驱动程序中配置端点的基址寄存器 (BAR)。

```c
FError FPcieCSetEpBar(FPcieC *instance_p, u32 fun_num, FPcieCEpBar *ep_bar)
```

Note:

- 此函数用于配置PCIe设备的端点基址寄存器（BAR）。它考虑了多种BAR类型，包括I/O、32位内存和64位内存。

Input:

- FPcieC *instance_p: 指向FPcieC实例的指针。
- u32 fun_num: EP功能号。
- FPcieCEpBar *ep_bar: 指向EP BAR配置的指针。

Return:

- FError: 如果成功，则返回FT_SUCCESS，否则返回错误代码。

#### FPcieCWriteEpHeader

- 在PCIe ECAM驱动程序中写入端点的PCI配置头的配置数据。

```c
FError FPcieCWriteEpHeader(FPcieC *instance_p, u32 fun_num, FPcieCEpHeader *header_p)
```

Note:

- 此函数用于向PCIe设备的端点PCI配置头写入配置数据。它首先执行一些基本的断言检查，然后写入与提供的配置头参数对应的值。此外，函数还确保某些特定的配置寄存器按照预期进行了配置。

Input:

- FPcieC *instance_p: 指向FPcieC实例的指针。
- u32 fun_num: EP功能号。
- FPcieCEpHeader *header_p: 指向EP配置头的指针。

Return:

- FError: 如果成功，则返回FT_SUCCESS，否则返回错误代码。

#### FPcieCDmaStatusGet

- 获取DMA通道的状态。

```c
u32 FPcieCDmaStatusGet(FPcieC *instance_p, u32 channel_index)
```

Note:

- 此函数用于查询PCIe设备的DMA通道的状态。它首先确保传入的参数有效，然后读取并返回所选DMA通道的状态。

Input:

- FPcieC *instance_p: 指向FPcieC实例的指针。
- u32 channel_index: DMA通道索引。

Return:

- u32: DMA通道的状态。

#### FPcieCDmaStart

- 在特定通道上启动DMA传输。

```c
FError FPcieCDmaStart(FPcieC *instance_p, u32 channel_index)
```

Note:

- 此函数用于在PCIe设备的特定DMA通道上启动数据传输。它首先确保传入的参数有效，然后通过设置控制寄存器中的特定标志来启动DMA传输。

Input:

- FPcieC *instance_p: 指向FPcieC实例的指针。
- u32 channel_index: DMA通道索引。

Return:

- FError: 如果操作成功，则为 `FT_SUCCESS`，否则为其他错误代码。

#### FPcieCDmaDirectConfig

- 用于直接数据传输方式配置DMA通道。

```c
FError FPcieCDmaDirectConfig(FPcieC *instance_p, u32 channel_index, uintptr_t src_addr, uintptr_t dest_addr, u32 length, u32 direct)
```

Note:

- 此函数用于配置DMA通道以进行直接数据传输，而不需要使用scatter-gather列表。它首先确保传入的参数是有效的，然后设置DMA控制、源地址、目标地址、传输长度和传输方向。

Input:

- `FPcieC *instance_p`: 指向FPcieC实例的指针。
- `u32 channel_index`: 要配置的DMA通道的索引。
- `uintptr_t src_addr`: 将从其中读取数据的地址。
- `uintptr_t dest_addr`: 将写入数据的地址。
- `u32 length`: 数据传输的长度。
- `u32 direct`: 传输的方向。这是 `FPCIEC_DMA_TYPE_READ`（从PCIe读取到其他位置）或 `FPCIEC_DMA_TYPE_WRITE`（从其他位置写入到PCIe）。

Return:

- `FError`: 如果配置成功，函数返回 `FT_SUCCESS`。否则，它返回一个错误代码。

#### FPcieCMiscIrqInit

- 初始化PCIe设备的杂项中断设置。

```c
void FPcieCMiscIrqInit(FPcieC *instance_p)
```

Note:

- 此函数初始化PCIe设备的中断设置，关闭所有中断并清除所有状态。它首先确保传入的参数有效，然后关闭所有中断，并清除中断状态寄存器。

Input:

- `FPcieC *instance_p`: 指向FPcieC实例的指针。

Return:

- 无返回值。

#### FPcieCMiscIrqRegister

- 为PCIe设备注册一个中断回调函数。

```c
FError FPcieCMiscIrqRegister(FPcieC *instance_p, FPcieCMiscIrqCallBack misc_irq, void *args)
```

Note:

- 此函数为PCIe设备的特定中断事件注册一个回调函数。注册后，当该中断事件发生时，系统将调用提供的回调函数。

Input:

- `FPcieC *instance_p`: 指向FPcieC实例的指针。
- `FPcieCMiscIrqCallBack misc_irq`: 中断回调函数。
- `void *args`: 传递给中断回调函数的参数。

Return:

- `FError`: 如果操作成功，则为 `FT_SUCCESS`，否则为其他错误代码。

#### FPcieCMiscIrq

- 处理PCIe设备的杂项中断事件。

```c
void FPcieCMiscIrq(s32 vector, void *args)
```

Note:

- 此函数是PCIe设备杂项中断的处理程序。当PCIe设备的某些特定中断事件被触发时，它会被调用。

Input:

- `s32 vector`: 中断向量，通常表示特定的中断源或ID。
- `void *args`: 传递给中断处理函数的参数，这里它是一个指向FPcieC实例的指针。
