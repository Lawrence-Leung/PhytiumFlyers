import os
import re

def parse_header_file(file_path):
    # Read .h file
    with open(file_path, 'r') as f:
        lines = f.readlines()

    macro_dict = {}
    for line in lines:
        # Strip whitespaces at the beginning and at the end of the line
        line = line.strip()

        if line.startswith("#define"):
        # Remove #define and split macro name and value
            parts = re.sub('#define ', '', line).split()

            if len(parts) == 2:
                macro_name, macro_value = parts
                if macro_name in ["CONFIG_SOC_NAME", "CONFIG_TARGET_TYPE_NAME", "CONFIG_ARCH_EXECUTION_STATE"]:
                    macro_dict[macro_name] = macro_value.strip('\"')  # remove quotation marks if it's string
            else:
                # print(f"Warning: Macro definition without value in line: {line}")
                pass

    # Check whether all required macros are found
    if  "CONFIG_TARGET_TYPE_NAME" not in macro_dict:
        if len(macro_dict) == 2:
            return macro_dict["CONFIG_SOC_NAME"]+ '_' + macro_dict["CONFIG_ARCH_EXECUTION_STATE"]
    else: 
    
        if len(macro_dict) == 3:
            return macro_dict["CONFIG_SOC_NAME"] + macro_dict["CONFIG_TARGET_TYPE_NAME"] + '_' + macro_dict["CONFIG_ARCH_EXECUTION_STATE"]
        else:
            print(f"Error: Not all required macros are found in {file_path}")
            return None


def check_soc_names(project_dict):
    soc_info_dict = {}
    for project_name, file_path in project_dict.items():
        if not os.path.exists(file_path):
            print(f"Error: File {file_path} does not exist")
            continue

        soc_name = parse_header_file(file_path)
        if soc_name is not None:
            soc_info_dict[project_name] = soc_name

    # Compare each soc_name with each other
    project_names = list(soc_info_dict.keys())
    error_flg = 0
    
    for i in range(len(project_names)):
        for j in range(i + 1, len(project_names)):
            if soc_info_dict[project_names[i]] != soc_info_dict[project_names[j]]:

                error_message = f"""
                        Error: Mismatch in SoC names.
                        - Project: {project_names[i]}
                        SoC Name: {soc_info_dict[project_names[i]]}
                        - Project: {project_names[j]}
                        SoC Name: {soc_info_dict[project_names[j]]}
                        """
                print(error_message)
                
                error_flg+=1
    
    if error_flg:
        exit(1)
    
    print("soc check is ok")
    return



if __name__ == '__main__':

    pass