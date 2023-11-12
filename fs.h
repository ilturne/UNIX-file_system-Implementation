#ifndef __FS_H__
#define __FS_H__
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define FSSIZE 10000000 //This is the size of the file system in bytes (10MB)
#define UNUSED 0 //This is the value of an unused block
#define MAX_DIRECT_BLOCKS 100 //This is the maximum number of direct blocks

typedef enum {
	INODE_FREE,
	INODE_FILE,
	INODE_DIRECTORY,
} Inodetype;

struct superblock {
	unsigned int magic; //This is the magic number, which identifies the file system type. 
	unsigned int nblocks; //Number of blocks
	unsigned int ninodes; //Number of inodes
	unsigned int pad[125]; // Padding to make it 512 bytes
};

struct inode {
	Inodetype type; //This is the type of the inode
	unsigned int size; //Size of the file in bytes
	unsigned int type; //Type of the file (0 = file, 1 = directory)
	int direct_blocks[MAX_DIRECT_BLOCKS]; //This is the array of direct blocks
};

/*Notes about padding: The 'pad' array is used to ensure that the size of the superblock aligns
with the size of a block (512 bytes). This is because the superblock is always stored at the
beginning of the file system. The size of the superblock is 512 bytes, so we need to add 125
integers to the superblock to make it 512 bytes.*/

extern unsigned char* fs;

void mapfs(int fd);
void unmapfs();
void formatfs();
void loadfs();
void lsfs();
void addfilefs(char* fname);
void removefilefs(char* fname);
void extractfilefs(char* fname);

#endif
