#include "fs.h"

unsigned char* fs;

void mapfs(int fd){
  if ((fs = mmap(NULL, FSSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL){
      perror("mmap failed");
      exit(EXIT_FAILURE);
  }
}


void unmapfs(){
  munmap(fs, FSSIZE);
}


void formatfs(){
	struct superblock* sb = (struct superblock*)fs;
	sb->magic = 0xdeadbeef;
	sb->nblocks = 10000000 / 512; // 10000000 bytes / 512 bytes per block
	sb->ninodes = 100;

	unsigned char* free_block_list = fs + sizeof(struct superblock); //This is the address of the first free block
	int bitmap_size = sb->nblocks / 8; //This is the size of the bitmap in bytes
	if (sb->nblocks % 8 != 0) bitmap_size++; //If the number of blocks is not a multiple of 8, we need to add an extra byte to the bitmap
		memset(free_block_list, 0, bitmap_size); //This sets all the free blocks to 0
	
		struct inode* inodes = (struct inode*)(fs + sizeof(struct superblock) + bitmap_size); //This is the address of the first inode
		for (int i = 0; i < 100; i++) {
			inodes[i].size = 0;
			inodes[i].type = UNUSED;
			memset(inodes[i].direct_blocks, -1, sizeof(inodes[i].direct_blocks)); //This sets all the direct blocks to -1 which means that they are unused
		}
}


void loadfs(){

}


void lsfs(){
  
}

void addfilefs(char* fname){

}


void removefilefs(char* fname){

}


void extractfilefs(char* fname){

}
