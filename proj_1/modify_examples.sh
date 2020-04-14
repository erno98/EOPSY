#!/bin/bash

# first, let's create example files.
create_files(){
	mkdir test
	touch "test/Test1" 		# ordinary file
	touch "test/Test2.txt" 	# ordinary file with extension
	touch "test/TesSsT3"		# another file with different capitalization
}

create_files

echo "Before:"
ls test

# test -l flag
./modify.sh -l "test/Test1"  # expected output: test/test1
./modify.sh -l "test/Test2.txt"  # expected output: test/test2.txt
./modify.sh -l "test/TesSsT3"  # expected output: test/tessst3

echo "After -l flag:"
ls test

# discard changes to files
rm -r test
create_files
echo '=========================='

# test -u flag
echo "Before:"
ls test

./modify.sh -u "test/Test1"  # expected output: test/TEST1
./modify.sh -u "test/Test2.txt"  # expected output: test/TEST2.TXT
./modify.sh -u "test/TesSsT3"  # expected output: tessst/TESSST3

echo "After -u flag:"
ls test

# discard changes to files
rm -r test
create_files
echo '=========================='

# -l -u incorrect usage
echo './modify.sh -u -l "test/Test1" '
./modify.sh -u -l "test/Test1"  # expected output: error 
echo 
echo './modify.sh -u -l "test/Test1"'
./modify.sh -l -u "test/Test1"  # expected output: error 

echo '=========================='


# sed pattern test
echo "Before:"
ls test
./modify.sh -s 's/Test/my_Test/' "test/Test1"  # expected output: test/XXX_1
./modify.sh -s 's/Test/sed_works_/' test/my_Test1 test/Test2.txt # expected output: my_sed_works_1 sed_works_2.txt

echo "After sed:"
ls test

echo '=========================='
echo '     RECURSION TESTS'
echo '=========================='
echo

rm -r test
create_files

echo "Before:"
ls test

# test -l flag
./modify.sh -r l "test/Test1"  # expected output: test/test1
./modify.sh -l -r "test/Test2.txt"  # expected output: test/test2.txt
./modify.sh -r -l "test/TesSsT3"  # expected output: test/tessst3

echo "After -l flag:"
ls test

# discard changes to files
rm -r test
create_files
echo '=========================='

# test -u flag
echo "Before:"
ls test

./modify.sh -r -u "test/Test1"  # expected output: test/TEST1
./modify.sh -u -r "test/Test2.txt"  # expected output: test/TEST2.TXT
./modify.sh -r -u "test/TesSsT3"  # expected output: tessst/TESSST3

echo "After -u flag:"
ls test

# discard changes to files
rm -r test
create_files
echo '=========================='

# -l -u incorrect usage
echo './modify.sh -u -l "test/Test1" '
./modify.sh -r -u -l "test/Test1"  # expected output: error 
echo 
echo './modify.sh -u -l "test/Test1"'
./modify.sh -l -r -u "test/Test1"  # expected output: error 

echo '=========================='


# sed pattern test
echo "Before:"
ls test
./modify.sh -r -s 's/Test/my_Test/' test/Test1  # expected output: test/XXX_1
./modify.sh -r -s 's/Test/sed_works_/' test/my_Test1 test/Test2.txt # expected output: my_sed_works_1 sed_works_2.txt

echo "After sed:"
ls test

echo '=================================='
echo 'INCORRECT USAGE AND ERROR MESSAGES'
echo '=================================='
echo

rm -r test
create_files

echo "no arguments"
echo "./modify.sh"
./modify.sh
echo
echo 'invalid flag -x'
echo "./modify.sh -r -u -x 's/Test/my_Test/' test/Test1"
./modify.sh -r -u -x 's/Test/my_Test/' test/Test1
echo
echo 'both -u and -l flags'
echo './modify.sh -u -l "test/Test1"'
./modify.sh -u -l "test/Test1" 