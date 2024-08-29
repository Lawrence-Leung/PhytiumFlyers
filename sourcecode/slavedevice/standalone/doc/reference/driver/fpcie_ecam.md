# FPCIE_ECAM 驱动程序

## 1. 概述

- 本驱动是对PCIE 中ECAM ( Enhanced Configuration Access Mechanism) 的软件抽象，主要实现了对PCI 配置空间访问的功能，并且提供了访问Capability struct 的功能。本模块作为PCIE 的通用模块，通过修改 ecam 的参数即可适配不同soc 上的PCIE 模块
- 本模块主要实现的特性如下：

1. 基于ECAM 的枚举功能
2. 基于bus、device 、function 访问配置空间的接口
3. 支持单设备连接的intx 中断上报功能
4. 支持capabilities 特性的查询功能

## 2. 功能

- 驱动相关的源文件如下
- drivers/pcie/fpcie_ecam

```
.
├── fpcie_ecam.c
├── fpcie_ecam.h
├── fpcie_ecam_caps.c
├── fpcie_ecam_common.h
├── fpcie_ecam_debug.c
├── fpcie_ecam_g.c
├── fpcie_ecam_intx.c
└── fpcie_ecam_sinit.c
```

## 3. 配置方法

此模块可以按照以下步骤进行初始化与使用：

1. 使用 FPcieEcamCfgInitialize 接口完成实例初始化，如果需要限制枚举的范围，可以注册need_skip_fun 函数
2. 使用 FPcieEcamEnumerateBus 从特定bus 进行枚举
3. 当枚举完成之后，枚举之后的端点将会被缓存中实例的scans_bdf 中，开发者可以通过读取其中的数据获取总线上的设备
4. 通过FPcieEcamReadConfigSpace 与 FPcieEcamWriteConfigSpace 接口即可访问特定端点

## 4. 应用示例

### [fpcie_ecam](../../../example/peripherals/pcie/)

## 5. API 数据结构

### 5.1 数据结构

- PCIE 地址空间数据集结构

```c
struct FPcieRegion
{
    FPcieAddr bus_start;      /* Start on the bus */
    FPciePhysAddr phys_start; /* Start in physical address space */
    FPcieSize size;           /* Size */
    unsigned long flags;      /* Resource flags */
    FPcieAddr bus_lower;       /* Lower address */
    u32 exist_flg; /* exist flg */
};

```

- intx中断功能

```c
/* Structure defining the PCIe interrupt callback function */
typedef struct
{
    void (*IntxCallBack)(void *args); /* Pointer to a callback function to be invoked on a PCIe interrupt event*/
    void *args;                       /* Arguments to be passed to the callback function*/
    u8 bus;                           /* PCIe bus number*/
    u8 device;                        /* PCIe device number*/
    u8 function;                      /* PCIe function number*/
} FPcieIntxFun;
```

- 驱动模块需要配置的参数

```c
/* Structure defining the PCIe ECAM (Enhanced Configuration Access Mechanism) configuration */
typedef struct
{
    u32 instance_id;                    /* ID of the device */
    uintptr_t ecam;                     /* Base address for the ECAM to access PCIe configuration space */
    u32 io_base_addr;                   /* IO base address */
    u32 io_size;                        /* IO size */
    u32 npmem_base_addr;                /* Non-prefetchable memory base address */
    u32 npmem_size;                     /* Non-prefetchable memory size */
    u64 pmem_base_addr;                 /* Prefetchable memory base address */
    u64 pmem_size;                      /* Prefetchable memory size */

    u8 inta_irq_num;                    /* IRQ number for INTA */
    u8 intb_irq_num;                    /* IRQ number for INTB */
    u8 intc_irq_num;                    /* IRQ number for INTC */
    u8 intd_irq_num;                    /* IRQ number for INTD */

    u32 max_bus_num;                    /* Maximum bus number */
    u32 max_dev_num;                    /* Maximum device number */
    u32 max_fun_num;                    /* Maximum function number */

    u32 feature;                        /* Feature flags, such as whether FPCI ECAM INTX requires EOI (End of Interrupt) */

#ifdef FPCI_ECAM_INTX_NEED_EOI
    uintptr_t control_intx_base[FPCI_ECAM_INTX_CONTROL_STATUS_REG_NUM]; /* Array of base addresses to control INTX */
    uintptr_t config_intx_base[FPCI_ECAM_INTX_CONFIG_ISTATUS_NUM];     /* Array of base addresses for INTX configuration */
#endif

} FPcieEcamConfig;
```

- 实例数据结构

```c
/* Main structure defining the PCIe ECAM properties */
typedef struct
{
    u32 is_ready;                       /* Flag indicating whether the device has been initialized and is ready */
    FPcieEcamConfig config;             /* Configuration data for PCIe ECAM */

    FPcieEcamNeedSkip need_skip_fun;    /* Data structure indicating functions that need to be skipped */

    struct FPcieRegion mem;             /* Memory region */
    struct FPcieRegion mem_prefetch;    /* Prefetchable memory region */
    struct FPcieRegion mem_io;          /* IO memory region */

    u32 bus_max;                        /* Maximum bus number currently in use */

    FPcieIntxFun inta_fun[FPCIE_MAX_SCAN_NUMBER]; /* Array of callback functions for INTA */
    s32 inta_num;                       /* Count of INTA callback functions */

    u32 is_scaned;                      /* Flag indicating whether the device has been scanned */
    u32 last_bus;                       /* Last scanned bus number */

    FPcieScanBdf scans_bdf[FPCIE_MAX_SCAN_NUMBER]; /* Array of scanned BDFs */
    u32 scans_bdf_count;                /* Count of scanned BDFs */
} FPcieEcam;
```

### 5.2 错误码定义

- FT_SUCCESS 成功
- FPCIE_EXCEED_BUS PCIe总线号超出了预期范围
- FPCIE_READCONFIG_ERROR  读取PCIe配置空间时发生错误
- FPCIE_CONFIG_REGION_ERROR PCIe配置区域存在问题
- FPCIE_TYPE_NOT_FIT    PCIe操作中类型不匹配或不兼容
- FPCIE_DEVICE_NOT_FOUND 未找到请求的PCIe设备
- FPCIE_INTX_OVER_RANGE 中断号(INTx)超出了预期范围

### 5.3 用户API接口

#### FPcieEcamCfgInitialize

- 用于从全局配置数据中获取数据并初始化 `FPcieEcam`实例

```c
FError FPcieEcamCfgInitialize(FPcieEcam *instance_p, FPcieEcamConfig *config_p, FPcieEcamNeedSkip need_skip_fun)
```

Note:

- 此函数会初始化配置空间和PCIe桥。调用此函数后，提供的 `FPcieEcam`实例将会被初始化并包含指定的配置。

Input:

- FPcieEcam *instance_p: 指向 `FPcieEcam`实例的指针。
- FPcieEcamConfig *config_p: 指向 `FPcieEcamConfig`的指针，用于为 `FPcieEcam`实例提供配置数据。
- FPcieEcamNeedSkip need_skip_fun: 一个函数，用于决定是否跳过特定的配置项。

Return:

- 成功返回 `FT_SUCCESS`

#### FPcieEcamEnumerateBus

- 此函数启动针对特定总线的枚举过程。

```c
FError FPcieEcamEnumerateBus(FPcieEcam *instance_p, u8 bus)
```

Note:

- 通过调用此函数，用户可以启动特定PCI总线的枚举过程。枚举结束后，`FPcieEcam`实例将包含该总线上的设备信息。

Input:

- FPcieEcam *instance_p: 指向 `FPcieEcam`实例的指针。
- u8 bus: 需要枚举的目标PCI总线号。

Return:

- FError: 错误码，表示枚举过程的成功或失败。

#### FPcieEcamReadConfigSpace

- 读取PCIe配置空间寄存器的值。

```c
FError FPcieEcamReadConfigSpace(FPcieEcam *instance_p, u8 bus, u8 device, u8 function, u16 offset, u32 *data)
```

Note:

- 此函数用于读取PCIe配置空间寄存器的值。用户需要提供特定的总线、设备、函数编号和寄存器偏移量以读取相应的寄存器值。
- 如果提供的总线号超出了最大支持的总线号范围，将返回 `FPCIE_EXCEED_BUS`错误码。
- 若总线号为0且设备号和函数号都为0，将返回 `FPCIE_CCR_INVALID_DATA`，表示未定义的寄存器数据。
- 若成功读取寄存器的值，函数将通过 `data`参数返回该值，并返回 `FT_SUCCESS`。

Input:

- FPcieEcam *instance_p: 指向 `FPcieEcam`实例的指针。
- u8 bus: 总线号。
- u8 device: 设备号。
- u8 function: 函数号。
- u16 offset: 寄存器的偏移量，使用 `FPCIE_CCR_XXX_REGS`系列寄存器。
- u32 *data: 用于存储读取结果的变量的指针。

Return:

- FError ret: 函数的执行结果。它返回一个错误码，该码描述了函数执行的状态。

#### FPcieEcamWriteConfigSpace

- 写入PCIe配置空间寄存器的值。

```c
FError FPcieEcamWriteConfigSpace(FPcieEcam *instance_p, u8 bus, u8 device, u8 function, u16 offset, u32 data)
```

Note:

- 此函数用于将特定的值写入PCIe配置空间寄存器。用户需要提供特定的总线、设备、函数编号和寄存器偏移量以写入相应的寄存器。
- 如果提供的总线号为0，将直接返回 `FT_SUCCESS`。
- 如果提供的总线号超出了最大支持的总线号范围，将返回 `FPCIE_EXCEED_BUS`错误码。
- 若成功写入寄存器的值，函数将返回 `FT_SUCCESS`。

Input:

- FPcieEcam *instance_p: 指向 `FPcieEcam`实例的指针。
- u8 bus: 总线号。
- u8 device: 设备号。
- u8 function: 函数号。
- u16 offset: 寄存器的偏移量。
- u32 data: 要写入的数据。

Return:

- FError ret: 函数的执行结果。它返回一个错误码，该码描述了函数执行的状态。

#### FPcieEcamLookupConfig

- 获取FPCIE ECAM驱动的默认配置参数。

```c
FPcieEcamConfig *FPcieEcamLookupConfig(u32 instance_id)
```

Note:

- 用户可以通过此接口获取驱动默认配置的副本。这个函数根据给定的实例ID在 `FPcieEcamConfigTable`中查找相应的配置，并返回该配置的指针。
- 如果在 `FPcieEcamConfigTable`中找不到匹配的实例ID，该函数将返回 `NULL`。

Input:

- u32 instance_id: 选择的FPcieEcam控制器实例号。

Return:

- FPcieEcamConfig *: 返回的默认驱动配置的指针。返回NULL表示没有找到相应的配置。

#### FPcieEcamHasExtendCapability

- 检查PCIe设备的扩展能力列表中是否存在特定的扩展能力。

```c
u32 FPcieEcamHasExtendCapability(FPcieEcam *instance_p, u8 bus, u8 device, u8 function, u16 capid)
```

Note:

- 此函数检查PCIe设备的扩展能力列表中是否存在特定的扩展能力。用户可以通过该接口验证某个具体的PCIe设备是否支持指定的扩展能力。
- 该函数将在扩展能力列表中循环查找给定的能力ID。如果找到，它将返回1，否则返回0。

Input:

- FPcieEcam *instance_p: 指向FPcieEcam实例的指针。
- u8 bus: PCIe总线号。
- u8 device: PCIe设备号。
- u8 function: PCIe函数号。
- u16 capid: 要检查的扩展能力的ID。

Return:

- u32: 如果扩展能力存在则返回1，否则返回0。

#### FPcieEcamHasCapability

- 检查PCIe设备的能力列表中是否存在特定的能力。

```c
u32 FPcieEcamHasCapability(FPcieEcam *instance_p, u8 bus, u8 device, u8 function, u8 capid)
```

Note:

- 此函数检查PCIe设备的能力列表中是否存在特定的能力。用户可以通过此接口验证某个具体的PCIe设备是否支持指定的能力。
- 该函数将在能力列表中循环查找给定的能力ID。如果找到，则它将返回1，否则返回0。

Input:

- FPcieEcam *instance_p: 指向FPcieEcam实例的指针。
- u8 bus: PCIe总线号。
- u8 device: PCIe设备号。
- u8 function: PCIe函数号。
- u8 capid: 要检查的能力的ID。

Return:

- u32: 如果能力存在则返回1，否则返回0。

#### FPcieEcamIntxRegister

- 注册特定PCIe设备功能的INTx中断处理程序。

```c
FError FPcieEcamIntxRegister(FPcieEcam *instance_p,u8 bus, u8 device, u8 function,FPcieIntxFun *func_p)
```

Note:

- 此函数允许用户为特定的PCIe设备功能注册INTx中断处理程序。

Input:

- FPcieEcam *instance_p: 指向FPcieEcam实例的指针。
- u8 bus: PCIe总线号。
- u8 device: PCIe设备号。
- u8 function: PCIe函数号。
- FPcieIntxFun *func_p: 指向中断处理函数及其关联参数的指针。

Return:

- FError: 表示操作成功或失败的错误代码。

#### FPcieEcamIntxIrqHandler

- PCIe ECAM驱动程序中的INTx中断处理程序。

```c
void FPcieEcamIntxIrqHandler(s32 vector, void *args)
```

Note:

- 当PCIe设备触发INTx中断时，这个中断处理函数(`FPcieEcamIntxIrqHandler`)将被调用。这个函数遍历系统中所有注册的PCIe设备，检查是否有INTx中断触发，然后调用相应的中断回调函数处理该中断。

Input:

- s32 vector: 中断向量号。
- void *args: 指向FPcieEcam实例的指针。
