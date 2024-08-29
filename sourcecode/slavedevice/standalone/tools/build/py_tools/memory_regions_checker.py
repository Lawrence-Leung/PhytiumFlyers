#!/usr/bin/env python3
import itertools

def collect_memory_regions():
    """
    收集多个地址范围的字典
    """
    memory_regions = []

    apu_dict = {"name":"apu","CONFIG_ROM_START_UP_ADDR":0x80100000,"CONFIG_ROM_SIZE_MB":2,
                "CONFIG_RAM_START_UP_ADDR":0x80300000,"CONFIG_RAM_SIZE_MB":64}
    rpu_dict = {"name":"rpu","CONFIG_ROM_START_UP_ADDR":0x80100000,"CONFIG_ROM_SIZE_MB":1,
                "CONFIG_RAM_START_UP_ADDR":0x80300000,"CONFIG_RAM_SIZE_MB":64}
    xpu_dict = {"name":"xpu","CONFIG_ROM_START_UP_ADDR":0xc0100000,"CONFIG_ROM_SIZE_MB":1,
                "CONFIG_RAM_START_UP_ADDR":0x80200000,"CONFIG_RAM_SIZE_MB":64}
    memory_regions.append(apu_dict)
    memory_regions.append(rpu_dict)
    memory_regions.append(xpu_dict)

    return memory_regions


def check_memory_regions(memory_regions):
    """
    检查地址范围是否有重叠的情况
    """
    overlapping_regions = []
    for region1, region2 in itertools.combinations(memory_regions, 2):
        rom_end1 = region1["CONFIG_IMAGE_LOAD_ADDRESS"] + region1["CONFIG_IMAGE_MAX_LENGTH"]
        rom_end2 = region2["CONFIG_IMAGE_LOAD_ADDRESS"] + region2["CONFIG_IMAGE_MAX_LENGTH"]
        if region1["CONFIG_IMAGE_LOAD_ADDRESS"] < rom_end2 and region2["CONFIG_IMAGE_LOAD_ADDRESS"] < rom_end1:
            overlapping_regions.append((region1["name"], region2["name"], "ROM overlap"))
    
    return overlapping_regions


def print_overlapping_regions(overlapping_regions, memory_regions):
    """
    使用ascii 图形展示出多个镜像重叠的位置，以及修改建议
    """
    print("The following memory regions overlap:")
    for region1, region2, reason in overlapping_regions:
        print("+{}+".format("-" * 77))
        print("|{:^77}|".format("Overlap between {} and {}".format(region1, region2)))
        print("|{:^77}|".format("Reason: {}".format(reason)))

        # 找到重叠的区域
        for region in memory_regions:
            if region["name"] == region1:
                region1_start_rom = region["CONFIG_IMAGE_LOAD_ADDRESS"]
                region1_size_rom = region["CONFIG_IMAGE_MAX_LENGTH"]
            elif region["name"] == region2:
                region2_start_rom = region["CONFIG_IMAGE_LOAD_ADDRESS"]
                region2_size_rom = region["CONFIG_IMAGE_MAX_LENGTH"]


        # 打印出重叠的区域
        if reason == "ROM overlap":
            print("|{:^77}|".format("ROM overlap between {} and {}".format(region1, region2)))
            print("|{:^77}|".format("Region {} - start: 0x{:x}, end: 0x{:x}".format(region1, region1_start_rom, region1_start_rom + region1_size_rom)))
            print("|{:^77}|".format("Region {} - start: 0x{:x}, end: 0x{:x}".format(region2, region2_start_rom, region2_start_rom + region2_size_rom)))
            print("|{:^77}|".format("Suggested modification:"))
            print("|{:^77}|".format("Reduce size of one of the ROM regions to avoid overlap"))

        print("+{}+".format("-" * 77))



if __name__ == "__main__":
    memory_regions = collect_memory_regions()
    overlapping_regions = check_memory_regions(memory_regions)

    if overlapping_regions:
        print_overlapping_regions(overlapping_regions, memory_regions)
    else:
        print("No overlapping regions found")
        
