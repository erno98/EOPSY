#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


void print_help();

void copy(int from, int to, char how);

void copy_read_n_write(int from_descriptor, int to_descriptor);
void copy_memory_map(int from_descriptor, int to_descriptor);

int main(int argc, char** argv){

	// No arguments given
	if(argc == 1){
		print_help();
		return 0;
	}

	// file names
	char* file_from;
	char* file_to;

	// values to be returned from open() function
	int file_from_descriptor;
	int file_to_descriptor;
	
	// Getting options
	char how = 'r';  // short for read, I'd put 'rw' but well, it's C, and I don't want to make a string here.
	char flag;

	while((flag = getopt(argc, argv, "hm")) != -1){
		// handle flags
		switch(flag){
			case 'h':
				print_help();
				return 0;
			case 'm':
				how = 'm';
				break;
			case '?':
				fprintf(stderr, "Unknown flag '-%c', exiting.\n", optopt);
				exit(1);
			default:
				fprintf(stderr, "getopt() error, exiting.\n");
				exit(1);
		}
	}
	// user input handling 
	if (how == 'm' && argc > 4 || how == 'r' && argc > 3){
		fprintf(stderr, "Too many arguments. See usage:\n");
		print_help();
		exit(1);
	} else if(how == 'm' && argc < 4 || how == 'r' && argc < 3){
		fprintf(stderr, "Not enough arguments. See usage:\n");
		print_help();
		exit(1);
	}

	// user input correct, proceed to functions

	// set up the filenames from args
	if (how == 'm'){
		file_from = argv[2];
		file_to   = argv[3];
	} else{
		file_from = argv[1];
		file_to   = argv[2];
	}


	// Opening the file to be copied
	file_from_descriptor = open(file_from, O_RDONLY);
	if(file_from_descriptor == -1) {
		perror("Input file open error\n");
		exit(1);
	}

	// Getting input file access rights to copy them
	struct stat buf;
	if(fstat(file_from_descriptor, &buf) == -1) {
		perror("fstat error\n");
		exit(1);
	}

	// Opening the output file
	file_to_descriptor = open(file_to, O_RDWR | O_CREAT, buf.st_mode);
	if(file_from_descriptor == -1) {
		perror("Output file open error\n");
		exit(1);
	}

	// Copying
	copy(file_from_descriptor, file_to_descriptor, how);

	// Closing file descriptors
	close(file_from_descriptor);
	close(file_to_descriptor);
		
	return 0;
}

void copy(int file_from, int file_to, char how){
	if(how == 'm'){
		copy_memory_map(file_from, file_to);
	} else{
		copy_read_n_write(file_from, file_to);
	}
}

void copy_read_n_write(int from_descriptor, int to_descriptor) {
	// copying by method 'read and write'

	// get the file size
	size_t file_size;

	struct stat file_info;
	if(fstat(from_descriptor, &file_info) == -1){
		perror("fstat error\n");
		exit(1);
	}

	file_size = (size_t) file_info.st_size;

	// Buffer to move data through
	char* buffer = (char*) malloc(file_size*sizeof(char));
	ssize_t result;
	
	// Read data from input and write to output until EOF
	while((result = read(from_descriptor, buffer, file_size)) != 0) {
		if(result == -1) {
			perror("read error\n");
			exit(1);
		}
		
		if(write(to_descriptor, buffer, result) == -1) {
			perror("write error\n");
			exit(1);
		}
	}

	free(buffer);
	
	return;
}

void copy_memory_map(int from_descriptor, int to_descriptor){
	// coping by method of memmory mapping

	// Get size of input file
	struct stat file_info;
	if(fstat(from_descriptor, &file_info) == -1) {
		perror("fstat error\n");
		exit(1);
	}

	// Pointers to memory maps
	char* buffer_from;
	char* buffer_to;

	buffer_from = mmap(NULL, file_info.st_size, PROT_READ, MAP_SHARED, from_descriptor, 0);
	if(buffer_from == (void*) -1) {
		perror("mmap in error\n");
		exit(1);
	}
	
	// Changing output file size to input file size before mapping
	if(ftruncate(to_descriptor, file_info.st_size)){
		perror("ftruncate error\n");
		exit(1);
	}

	buffer_to = mmap(NULL, file_info.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, to_descriptor, 0);

	if(buffer_to == (void*) -1) {
		perror("mmap out error\n");
		exit(1);
	}

	// Copy
	buffer_to = memcpy(buffer_to, buffer_from, file_info.st_size);
	if(buffer_to == (void*) -1) {
		perror("mcpy error\n");
		exit(1);
	}

	return;
}

void print_help()
{
	printf("Program copying a file to another file, either read-write or by memory mapping.\n");
	printf("Usage:\n\n");
	printf("\tcopy [-m] <file_name> <new_file_name>\n");
	printf("\tcopy [-h]\n\n");
	printf("Option -m enables memory mapping, by default program is usying read-write methodology.\n\n");
	printf("<file_name>\t(first specified) is the name of the file to be copied\n");
	printf("<new_file_name>\t(second specified) is the target file.\n\n");
	printf("Running with -h or no arguments shows this message.\n");
	printf("Example usage:\n");
	printf("\t copy -m file_to_be_copied file_new\n");
	printf("\t copy file_to_be_copied file_new\n");
	printf("\t copy -h\n\n");
}

