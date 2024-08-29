#!/usr/bin/env python3
import os
import sys

def _main():
    AMP_PATHS = sys.argv[1:]
    AMP_ELF_FILES = []
    for amp_path in AMP_PATHS:
        for root, dirs, files in os.walk(amp_path):
            for file in files:
                if file.endswith(".elf"):
                    if os.path.join(root, file) not in AMP_ELF_FILES:
                        AMP_ELF_FILES.append(os.path.join(root, file))

    # Step 2: sort the ELF files by their names
    AMP_ELF_FILES.sort()

    # Step 3: pack the ELF files into a single binary file
    packed_file = open("packed.bin", "wb")
    offset = 0
    for elf_file in AMP_ELF_FILES:
        with open(elf_file, "rb") as f:
            elf_data = f.read()
            packed_file.write(elf_data)
            print(f"{elf_file} starts at offset {offset}")
            if len(elf_data) %4:
                print("\033[31m elf file size is error\033[0m")
                packed_file.close()
                exit()
                
            offset += len(elf_data)
    packed_file.close()


if __name__ == '__main__':
    _main()