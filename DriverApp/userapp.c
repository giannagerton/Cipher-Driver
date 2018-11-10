#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE "/dev/TestingDriver"

int main(){

	int i, fd;
	char ch, write_buf[100], read_buf[100];

	fd = open(DEVICE,O_RDWR);

	if(fd == -1){
		printf("file %s DNE or is locked \n", DEVICE);
		exit(-1);
	}
	printf ("r = read from device \n w = write to device \n enter command: ");
	scanf("%c", &ch);
	/*
		Let's plan this out. I mean really I would like the following to be the flow:

		ioctl call to return number of pairs available:
		if zero: would you like to create a new pair. Now we have E0 D0.

		C create, D destory, set key, encrypt, decrypt.
		Create, returns id# of created pair
		Destroy, returns id# of destroyed pair
		Set key: give id# of key to set, edit key, thats it. no return necessary.
		Encrypt: give ID number, and give message. return encrypted message.
		Decrypt: give ID,  return decrypted message. 



	*/
	switch (ch){
		case 'w':
			printf("enter data: ");
			scanf(" %[^\n]", write_buf);
			write(fd, write_buf, sizeof(write_buf));
			break;
		case 'r':
			read(fd,read_buf, sizeof(read_buf));
			printf("device: %s\n", read_buf);
			break;
		default:
			printf("Ya gone fucked up Ay ay ron.");
			break;
	}
	
	close(fd);
	return 0;

}

