# Import necessary modules
import os
import re
import sys

# Define the name of the SDK config file
SDK_CONFIG_NAME = "sdkconfig.h"

# Define a function to check if two ranges intersect
def ranges_intersect(start1, length1, start2, length2):
    return not (start1 + length1 <= start2 or start2 + length2 <= start1)

# Define a function to get parameters from a file
def get_params_from_file(filename, params):
    # Open the file in read mode
    with open(filename, 'r') as f:
        content = f.read()
    # Use regular expressions to find the parameters and their values in the file
    return {param: int(re.search(f'#define\s+{param}\s+(0x[0-9a-fA-F]+)', content).group(1), 16) for param in params}

# Define a function to get the available space for images
def get_available_space(soc_params, image_params):
    # Initialize the available space with the two memory ranges of the SoC
    available_space = [(soc_params['CONFIG_F32BIT_MEMORY_ADDRESS'], soc_params['CONFIG_F32BIT_MEMORY_LENGTH']),
                       (soc_params['CONFIG_F64BIT_MEMORY_ADDRESS'], soc_params['CONFIG_F64BIT_MEMORY_LENGTH'])]

    # Iterate over the image parameters and check if their ranges intersect with the available space
    for image_param in image_params:
        image_start = image_param['CONFIG_IMAGE_LOAD_ADDRESS']
        image_length = image_param['CONFIG_IMAGE_MAX_LENGTH']
        for start, length in available_space:
            if ranges_intersect(start, length, image_start, image_length):
                # If the ranges intersect, split the available space into two parts
                if start < image_start:
                    available_space.append((start, image_start - start))
                if start + length > image_start + image_length:
                    available_space.append((image_start + image_length, start + length - image_start - image_length))
                # Remove the original available space
                available_space.remove((start, length))
    return available_space

# Define a function to write the available space to a header file
def write_to_header(available_space, filename):
    # Open the file in write mode
    with open(filename, 'w+') as f:
        # Iterate over the available space and write the start and end addresses to the file
        for i, (start, length) in enumerate(available_space):
            end = start + length - 1  # calculate end address
            f.write(f'#define AVAILABLE_SPACE_START_{i} 0x{start:x}\n')
            f.write(f'#define AVAILABLE_SPACE_END_{i} 0x{end:x}\n')

# Define a function to check if the kconfig file exists in the given path
def check_kconfig(path):
    kconfig_file = os.path.join(path, SDK_CONFIG_NAME)
    if not os.path.exists(kconfig_file):
        print("kconfig file does not exist in {}".format(path))
        return -1
    return 0

# Define the main function
def soc_boot_space(boot_path,amp_paths):
    # Get the SoC file and the paths to the AMPs from the command line arguments
    
    # Initialize the list of image files
    image_files = list()
    
    # Iterate over the AMP paths and check if the kconfig file exists
    for path in amp_paths:
        if check_kconfig(path) == -1:
            print("\033[31m{}\033[0m kconfig is not valid".format(path))
            sys.exit(-1)

        # Add the path to the SDK config file to the list of image files
        image_files.append(path + "/"+SDK_CONFIG_NAME)
        
    soc_file = image_files[0] 
    # Define the output file name
    output_file = boot_path+"/"+"available_space.h"

    # Get the SoC parameters and the image parameters from their respective files
    soc_params = get_params_from_file(soc_file, ['CONFIG_F32BIT_MEMORY_ADDRESS', 'CONFIG_F32BIT_MEMORY_LENGTH', 'CONFIG_F64BIT_MEMORY_ADDRESS', 'CONFIG_F64BIT_MEMORY_LENGTH'])
    image_params = []
    for image_file in image_files:
        image_params.append(get_params_from_file(image_file, ['CONFIG_IMAGE_LOAD_ADDRESS', 'CONFIG_IMAGE_MAX_LENGTH']))

    # Get the available space for the images
    available_space = get_available_space(soc_params, image_params)

    # Write the available space to the output file
    write_to_header(available_space, output_file)
    

    


