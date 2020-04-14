# PROJECT 1 - MODIFY 

## modify.sh
Script modifying the given filenames (relative or absolute path) either by lowerizing, uppercasing, or given sed pattern. The script can perform all of the mentioned modification either recursively (-r) or iteratively (default).
Usage:

        modify [-r] [-l|-u] <file_1 file_2 ... file_n>
        modify [-r] [-s <sed pattern>] <file1 file2 ... file_n>

Note, that the order of given arguments doesn't matter (**except the 'sed pattern' after -s flag**).

### Key concepts
If the file does not exist or the given path is wrong, the script **will ommit such entries**. If no files were changed, it will be signalized by the script, although with code 0 (success).  

Script can't modify **n** groups of files in **n** different ways, i.e. such procedure is not possible:

        modify -l file_1 file_2 -u file_3 file_4  # NOT POSSIBLE
Instead, user should make two separate calls:

        modify -l file_1 file_2 && modify -u file_3 file_4
Script will modify only the basename of the file. For example, given:
        
        modify -u dir_1/file_1           # will result in 'dir_1/FILE_1'
        modify -l DIR_2/subdir_1/File_4  # will result in 'DIR_2/subdir_1/file_4'


## modify_examples.sh
Script for testing of modify.sh  


Created for course of Operating Systems, Warsaw University of Technology, Faculty of Electronics and Information Technology, 2020. Made by Ernest Pokropek
