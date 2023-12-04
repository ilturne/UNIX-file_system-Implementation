#include "fs.h"

struct superblock* sb = NULL; //Initiate the superblock once
struct inode* inodes = NULL; //Initiate the root inode once
struct inode* root = NULL; //Initiate the root inode once

void mapfs(int fd){
  if ((fs = mmap(NULL, FSSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL){
      perror("mmap failed");
      exit(EXIT_FAILURE);
  }
}


void unmapfs(){
  munmap(fs, FSSIZE);
}

void formatfs() {
    // Initialize the superblock
    sb = (struct superblock*)fs;
    sb->magic = 0xdeadbeef;
    sb->nblocks = 10000000 / 512; // Total file system size divided by block size
    sb->ninodes = 100;            // Number of inodes

    // Calculate the bitmap size and initialize the free block list
    int bitmap_size = sb->nblocks / 8;
	if (sb->nblocks % 8 != 0) bitmap_size++;
    unsigned char* free_block_list = fs + sizeof(struct superblock);
    memset(free_block_list, 0, bitmap_size); // Set all blocks to free

    // Initialize the inode table
    inodes = (struct inode*)(fs + sizeof(struct superblock) + bitmap_size);
    for (int i = 0; i < sb->ninodes; i++) {
        inodes[i].type = UNUSED;
        inodes[i].size = 0;
        memset(inodes[i].direct_blocks, -1, sizeof(inodes[i].direct_blocks)); // Invalidate all direct block references
    }

    // Set up the root inode
    root = &inodes[1];
    root->type = INODE_DIRECTORY;
}



void loadfs() {
    // Verify the superblock's magic number to ensure file system is formatted
    if (sb == NULL || sb->magic != 0xdeadbeef) {
        fprintf(stderr, "File system is not formatted correctly.\n");
        exit(EXIT_FAILURE);
    }

    // The inode table should already be initialized, but you can re-establish the pointer if needed
    int bitmap_size = sb->nblocks / 8;
    if (sb->nblocks % 8 != 0) bitmap_size++;
    inodes = (struct inode*)(fs + sizeof(struct superblock) + bitmap_size);

    // Re-establish the root inode pointer
    root = &inodes[0]; // Assuming the first inode is the root
}


void lsfs() {
    if (root == NULL || root->type != INODE_DIRECTORY) {
        printf("Root directory not found or is not a directory.\n");
        return;
    }

    // Assuming directory entries are stored in the direct blocks of the inode
    for (int i = 0; i < MAX_DIRECT_BLOCKS; i++) {
        if (root->direct_blocks[i] == -1) {
            break; // No more data blocks
        }

        dir_entry* entries = (dir_entry*)(fs + root->direct_blocks[i] * BLOCK_SIZE);
        for (int j = 0; j < ENTRIES_PER_BLOCK; j++) {
            if (strlen(entries[j].name) == 0) {
                break; // No more entries in this block
            }

            // Print the directory entry
            printf("%s\n", entries[j].name);

            // If it's a directory, you might want to list its contents as well
            // For simplicity, this example only lists the root directory contents
        }
    }
}

void addfilefs(char* fname){

}


void removefilefs(char* fname){

}


void extractfilefs(char* fname){

}
