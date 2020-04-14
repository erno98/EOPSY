
#!/bin/bash

# --------- FUNCTIONS DECLARATIONS --------- 
file_exists(){
# function checking whether given file exists
	if [ -f "$1" ]; then
		return 0
	else
		return 1
	fi
}

get_files(){
# function returning filenames (clearing flags and sed from input)
# it returns only those files which exist, as non-existing files should be skipped

	for var in "$@"
	do
		if file_exists "$var"; then
			echo "$var"
		fi
	done
}


to_upper(){
# function transforming given filename into uppercase, doesn't change the directory name

	filename=$(basename "$1")
	if [ "${filename%.*}" = "${filename##*.}" ]; then
		ext=""
	else
		ext=".${filename##*.}"
	fi
	filename=$(echo "${filename%.*}" | tr [a-z] [A-Z])

	dname=$(dirname "$1")
	echo "$dname/$filename$ext" 
}

files_to_upper(){
# function transforming given files to uppercase, see to_upper() function
	for file in $@
	do
		mv $file $(to_upper $file) 2>/dev/null
	done
}

files_to_upper_r(){
# function transforming given files to uppercase recursively
	files=$@
	for file in $files
	do
		if [ -z "$file" ]; then
			break
		fi

		mv $file $(to_upper $file) 2>/dev/null
		files_to_upper_r $(echo $files | awk '{$1=""; print $0;}')

	done
}

to_lower(){
# function transforming given filename into lowercase, doesn't change the directory name
	filename=$(basename "$1")
	if [ "${filename%.*}" = "${filename##*.}" ]; then
		ext=""
	else
		ext=".${filename##*.}"
	fi
	filename=$(echo "${filename%.*}" | tr [A-Z] [a-z])

	dname=$(dirname "$1")
	echo "$dname/$filename$ext" 
}

files_to_lower(){
# function transforming given files to lowercase, see to_lower() function
	for file in $@
	do
		mv $file $(to_lower $file) 2>/dev/null
	done
}

files_to_lower_r(){
# function transforming given files to lowercase recursively
	files=$@
	for file in $files
	do
		if [ -z "$file" ]; then
			break
		fi

		mv $file $(to_lower $file) 2>/dev/null
		files_to_upper_r $(echo $files | awk '{$1=""; print $0;}')

	done
}

files_sed(){
# function transforming given file names with given sed pattern

	sed_p=$1

	files=$(echo $@ | awk '{$1=""; print $0;}')
	for file in $files
	do
		fname=$(basename "$file")
		dname=$(dirname "$file")

		fname=$(echo "$fname" | sed "$sed_p")
		new_name="$dname/$fname" 

		mv $file $new_name 2>/dev/null
	done

}

files_sed_r(){
# function transforming given file names with given sed pattern recursively

	sed_p=$1
	files=$(echo $@ | awk '{$1=""; print $0;}')

	for file in $files
	do
		if [ -z "$file" ]; then
			break
		fi

		fname=$(basename "$file")
		dname=$(dirname "$file")

		fname=$(echo "$fname" | sed "$sed_p")
		new_name="$dname/$fname" 

		mv $file $new_name 2>/dev/null

		files_sed_r $sed_p $(echo $files | awk '{$1=""; print $0;}')

	done

}

help_text(){
	echo "Script 'modify' that modifies given filenames."
	echo "Syntax: modify [-r] [-l|-u] <file1 file2 ...> or modify [-r] <sed pattern> <file1 file2 ...>" 
	echo "	-r		recursive modification of given files"
	echo "	-l		convert filenames to lowercase (doesn't impact extenion)"
	echo "	-u		convert filenames to uppercase (doesn't impact extenion)"
	echo "	-h		display this help and exit"
	echo "	-s		sed pattern to modify the files (given <sed pattern> after flag -s), can impact extension."
	echo "Exit Codes:"
	echo "	0	if OK,"
	echo "	2	if script ran into serious trouble (e.g. invalid flag, no arguments passed)"
	echo "Script done for 2020 summer semester of Operating Systems course at Warsaw University of Technology, Faculty of Electronics and Information Technology"
	echo "Made by Ernest Pokropek"
}

# --------- PROGRAM ---------

# check whether there were arguments passed (last equal first)
if [ "${@: -1}" = "${0}" ]; then
	echo "No arguments passed to the script."
	exit 2
fi

# only files to be modified
FILES=$(get_files "$@")

lower=false
upper=false
recursive=false
sed_p=false

while getopts "hrlus:" flag; do
	case "$flag" in
		h)	help_text ;;

		u)	upper=true ;;

		l)  lower=true ;;

		r)	recursive=true ;;
		
		s)	sed_p=true
			
			pattern=${OPTARG}
			first_file=$(echo $FILES | awk '{print $1;}')

			if [ "$pattern" = "$first_file" ]; then
				echo "No sed pattern given."
				exit 2
			fi
			;;
		
		*)	echo "Error: invalid flag (available: -h -u -l -r -s)."
			exit 2 ;;
	esac

done

# if given files do not exist, exit with success
if [ -z "$FILES" ]; then
	echo "No files changed."
	exit 0
fi

if $lower && $upper; then
	echo "Error: cannot set both -u and -l flag."
	exit 2
fi

# recursive
if $recursive; then
	if $lower; then  # to lower
		files_to_lower_r $FILES

	elif $upper; then  # to upper
		files_to_upper_r $FILES

	elif $sed_p; then  # sed pattern
		files_sed_r $pattern $FILES
	fi 
# iterative
else
	if $lower; then  # to lower
		files_to_lower $FILES

	elif $upper; then  # to upper
		files_to_upper $FILES

	elif $sed_p; then  # sed pattern
		files_sed $pattern $FILES
	fi 
fi 


