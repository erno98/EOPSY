
#!/bin/bash


# TODO: rekursja i sed handling

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
	fname=$(basename "$1" | tr [a-z] [A-Z])
	dname=$(dirname "$1")
	echo "$dname/$fname" 
}

files_to_upper(){
# function transforming given files to uppercase, see to_upper() function
	for file in $@
	do
		mv $file $(to_upper $file)
	done
}

to_lower(){
# function transforming given filename into lowercase, doesn't change the directory name
	fname=$(basename "$1" | tr [A-Z] [a-z])
	dname=$(dirname "$1")
	echo "$dname/$fname" 
}

files_to_lower(){
# function transforming given files to lowercase, see to_lower() function
	for file in $@
	do
		mv $file $(to_lower $file) 2>/dev/null
	done
}

help_text(){
	echo "Script 'modify' that modifies given filenames."
	echo "Syntax: modify [-r] [-l|-u] <file1 file2 ...> or modify [-r] <sed pattern> <file1 file2 ...>" 
	echo "	-r		recursive modification of given files"
	echo "	-l		convert filenames to lowercase"
	echo "	-u		convert filenames to uppercase"
	echo "	-h		display this help and exit"
	echo "	<sed pattern>  	sed pattern to modify the files, to be wrapped in quotation marks."
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

while getopts "hrlu" flag; do
	case "$flag" in
		h)	help_text ;;

		u)	upper=true ;;

		l)  lower=true ;;

		r) recursive=true ;;
		
		*)	echo "Error: invalid flag $1 (available: -h -u -l -r)."
			exit 2 ;;
	esac

done

if $lower && $upper; then
	echo "Error: cannot set both -u and -l flag."
	exit 2
fi

if $lower; then
	files_to_lower $FILES

elif $upper; then
	files_to_upper $FILES
fi 


