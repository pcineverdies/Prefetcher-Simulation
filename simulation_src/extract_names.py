# This program generate the list of al the files that will be analyzed

import os

def list_files(folder_path, output_file):
    with open(output_file, 'w') as file:
        for root, _, files in os.walk(folder_path):
            os.mkdir("results/" + root.replace("images/", ""))
            for file_name in files:
                file_path = os.path.join(root, file_name)
                relative_path = os.path.relpath(file_path, folder_path)
                file.write(relative_path + '\n')

# Define the folder path
folder_path = 'images/'

# Define the output file name
output_file = 'file_list.txt'

list_files(folder_path, output_file)

print(f"File names have been written to '{output_file}'")