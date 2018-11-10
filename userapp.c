#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include "cryptctl.h"

#define MAIN_DEVICE "/dev/cryptctl"
#define ENCRYPT_DEVICE "/dev/cryptEncrypt"
#define DECRYPT_DEVICE "/dev/cryptDecrypt"


int ioctl_create(int file_desc) {

	int ret_val;
	ret_val = ioctl(file_desc, IOCTL_CREATE);
	if (ret_val < 0){
		printf("Failed to create");
		return -1;
	}
	printf("Created the pair#: %d",ret_val);
	return ret_val;

}


int ioctl_destroy(int file_desc, int id){
	int ret_val;
	ret_val = ioctl(file_desc, IOCTL_DESTROY, id);
	if (ret_val != 0){
		printf("Failed to destroy");
		return -1;
	}
	return ret_val;
}

int ioctl_key(int file_desc, int id, char buff[KEY_SIZE]){

	struct idKey param;
//	param.id = id;
	param.id = id;
	strcpy(param.key, buff);
	int ret_val;
	ret_val = ioctl(file_desc, IOCTL_KEY, &param);
	if(ret_val != 0){
		printf("failed to change key");
		return -1;
	}				
	
	printf("key is now set.");
	return ret_val;
}

int ioctl_encrypt(int file_desc, int id, char buff[MESSAGE_SIZE]) {
		
	struct idBuff param;
	param.id = id;
	strcpy(param.buff, buff);
	int ret_val;
	ret_val = ioctl(file_desc, IOCTL_ENCRYPT, &param);
	if (ret_val != 0) {
		printf("failed to print\n");
	}
	printf("value is now encrypted\n");
	printf("%s\n", param.buff);
	return ret_val;
}

int ioctl_decrypt(int file_desc, int id, char buff[MESSAGE_SIZE]) {
	
	struct idBuff param;
	param.id = id;	
	strcpy(param.buff, buff);
	int ret_val;
	ret_val = ioctl(file_desc, IOCTL_DECRYPT, &param);
	if (ret_val != 0) {
		printf("failed to print\n");
		
	}
	printf("value is now decrypted\n");
	printf("decrypted value: %s\n", param.buff);
	return ret_val;
}

int main(){
        int i, fd, fdtemp;
        char ch, key_buf[100], write_buf[1000], read_buf[1000];

        fd = open(MAIN_DEVICE, O_RDWR);

        if(fd == -1){
                printf("file %s DNE or is locked \n", DEVICE_NAME);
                return -1;
        }
        	printf ("\nc = create device pair \nd = destroy to device \nk = set key \nn = encrypt \nm = decrypt \nz = exit user app \nenter command: ");
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
			case 'z':
				printf("Good bye");
				close(fd);
				return 0;
                	case 'c':
				printf("Creating \n");
        	                ioctl_create(fd);
	                        break;
        	        case 'd':
				printf("Destorying: \nEnter the ID number of the pair you want to destroy: \n");
				scanf("%d", &i);
				ioctl_destroy(fd, i);
				break;
	                case 'k':
 				printf("Changing key. \n Enter the ID number of the key you want to set or change: \n");
				scanf("%d", &i);
				printf("enter key (Max 100 characters, ALL CAPS): ");
                	        scanf(" %[^\n]", key_buf);
				ioctl_key(fd,i, key_buf);
                       		
				//write(fd, write_buf, sizeof(write_buf));
                        	
				break;
	                case 'n':
 				printf("Encrypting: \nEnter the ID number: \n");
				scanf("%d", &i);
				printf("enter message (max 1000 characters, ALL CAPS):\n ");
                	        scanf(" %[^\n]", write_buf);
			        // ioctl_encrypt(fd, i, write_buf);
				ioctl_encrypt(fd, i, write_buf);
			/*	fdtemp = open(ENCRYPT_DEVICE "%d",i, O_RDWR);
				read(fdtemp,read_buf, sizeof(read_buf));

                	        printf("cryptEncrypt%d: %s\n", i, read_buf);
                        	close(fdtemp); */
				break;
			case 'm':
				printf("Decrypting: \nEnter the ID number: \n");
				scanf("%d", &i);
				ioctl_decrypt(fd, i, read_buf);
				//ioctl_decrypt(fd, i, read_buf);
				break;
		        default:
        	                printf("Ya done fucked up Ay ay ron. %c \n",ch);
                	        return -1;
				break;
        	}
		printf("\n==================================================================\n");
        
	close(fd);
	return 0;
}

