#include "cryptctl.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int ioctl_create_key(int file_desc, char* key) {
	int ret_val;

	ret_val = ioctl(file_desc, IOCTL_CREATE, key);
	
	if (ret_val!=0) {
		printf("failed to create files/key\n");
		exit(-1);
	}
	return ret_val;
}

int ioctl_destroy_files(int file_desc) {
	int ret_val;

	ret_val = ioctl(file_desc, IOCTL_DESTROY);

	if (ret_val != 0) {
		printf("failed to destroy files\n");
		exit(-1);
	}
	return ret_val;
}

int main() {
	int file_desc, files_id;
	char* key = "key";
	file_desc = open("cryptctl", 0);
	
	if (file_desc < 0) {
		printf("error opening file\n");
		exit(-1);
	}

	files_id = ioctl_create_key(file_desc, key);
	int encrypt_fd, decrypt_fd;
	char* encryptFile;
	sprintf(encryptFile, "cryptEncrypt%d", files_id);
	encrypt_fd = open(encryptFile, 0);
	int num_bytes = 10;
	if (write(encrypt_fd, "testingab", num_bytes) != num_bytes) {
		printf("error: not enough bytes written\n");
		exit(-1);
	}
	char* buff = malloc(10);
	if (read(encrypt_fd, buff, num_bytes) != num_bytes) {
		printf("error: not enough bytes read\n");
		exit(-1);
	}
	printf("read: %s\n", buff);
	close(encrypt_fd);
	ioctl_destroy_files(file_desc);
	return 0; 
}
