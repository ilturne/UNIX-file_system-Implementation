#include "fs.h"
#include <string.h>

struct superblock* sb = NULL; //Initiate the superblock once
struct inode* inodes = NULL; //Initiate the root inode once
struct inode* root = NULL; //Initiate the root inode once
unsigned char* fs;

int is_mapped = 0;

void mapfs(int fd){
  if ((fs = mmap(NULL, FSSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL){
      perror("mmap failed");
      exit(EXIT_FAILURE);
  }
  else {
    is_mapped = 1;
  }
}


void unmapfs(){
  munmap(fs, FSSIZE);
  is_mapped = 0;
}

void formatfs() {
    // Verify that the file system is memory-mapped
    if (!is_mapped) {
        fprintf(stderr, "File system is not memory-mapped. Call mapfs first.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the superblock
    sb = (struct superblock*)fs;
    //sb->magic = 0xdeadbeef;
    sb->nblocks = FSSIZE / BLOCK_SIZE; // Total file system size divided by block size
    sb->ninodes = 100;// Number of inodes

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
    root = &inodes[0];
    root->type = INODE_DIRECTORY;
}

void loadfs() {
    // Verify that the file system is memory-mapped
    if (!is_mapped) {
        fprintf(stderr, "File system is not memory-mapped. Call mapfs first.\n");
        exit(EXIT_FAILURE);
    }
    // Verify the superblock's magic number to ensure file system is formatted
    if (sb == NULL) {
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

    // A stack to keep track of directories to be listed
    struct inode* stack[MAX_DIRECT_BLOCKS * sb->ninodes];
    int depth_stack[MAX_DIRECT_BLOCKS * sb->ninodes]; // To keep track of depth
    int stack_ptr = 0;

    // Push the root directory onto the stack
    stack[stack_ptr] = root;
    depth_stack[stack_ptr] = 0;
    stack_ptr++;

    if (root->type != INODE_DIRECTORY) {
        fprintf(stderr, "Root inode is not set correctly.\n");
        exit(EXIT_FAILURE);
    }

    while (stack_ptr > 0) {
        // Pop a directory from the stack
        stack_ptr--;
        struct inode* current_inode = stack[stack_ptr];
        int current_depth = depth_stack[stack_ptr];

        for (int i = 0; i < MAX_DIRECT_BLOCKS; i++) {
            if (current_inode->direct_blocks[i] == -1) {
                break; // No more data blocks in this directory
            }

            dir_entry* entries = (dir_entry*)(fs + current_inode->direct_blocks[i] * BLOCK_SIZE);
            for (int j = 0; j < ENTRIES_PER_BLOCK; j++) {
                if (strlen(entries[j].name) == 0) {
                    break; // No more entries in this block
                }

                // Print the directory entry with indentation
                for (int k = 0; k < current_depth; k++) {
                    printf("\t");
                }
                printf("%s\n", entries[j].name);

                // If it's a directory, push it onto the stack
                if (entries[j].inode_num != -1 && inodes[entries[j].inode_num].type == INODE_DIRECTORY) {
                    stack[stack_ptr] = &inodes[entries[j].inode_num];
                    depth_stack[stack_ptr] = current_depth + 1;
                    stack_ptr++;
                }
            }
        }
    }
}


void addfilefs(char* fname){
    // // Step 1: Parse the path
    // char components[256][MAX_FILENAME_LENGTH + 1];
    // char filename[MAX_FILENAME_LENGTH + 1];
    // int num_components;

    // parse_path(fname, components, &num_components, filename);
    // // Step 2: Ensure directories exist (to be implemented)
    // struct inode* current_dir = root;
    // for (int i = 0; i < num_components; i++) {
    //     struct inode* next_dir = find_directory(current_dir, components[i]);
    //     if (next_dir == NULL) {
    //         // Directory doesn't exist, create it
    //         next_dir = create_directory(current_dir, components[i]);
    //         if (next_dir == NULL) {
    //             printf("Failed to create directory %s\n", components[i]);
    //             return;
    //         }
    //     }
    //     current_dir = next_dir;
    // }

    // // Step 3: Find a free inode for the file
    // int file_inode_index = find_free_inode();
    // if (file_inode_index < 0) {
    //     printf("No free inodes available.\n");
    //     return;
    // }

    // // Step 4: Write the file data
    // struct inode* file_inode = &inodes[file_inode_index];
    // file_inode->type = INODE_FILE;
    // // Logic to write file data and update file_inode's direct block references

    // // Step 5: Update the parent directory's inode with the new file entry
    // // (Assuming you have a function to update a directory's contents)
}


void removefilefs(char* fname){
    
}


void extractfilefs(char* fname){

}

//USER ADDED FUNCTIONS

void parse_path(const char* path, char components[256][MAX_FILENAME_LENGTH + 1], int* num_components, char* filename) {
    char* path_copy = strdup(path); // Duplicate the path to avoid modifying the original
    char* token;
    char* rest = path_copy;

    *num_components = 0;
    while ((token = strtok_r(rest, "/", &rest))) {
        if (*num_components < 256 - 1) {
            strncpy(components[*num_components], token, MAX_FILENAME_LENGTH);
            components[*num_components][MAX_FILENAME_LENGTH] = '\0'; // Null-terminate
            (*num_components)++;
        }
    }

    if (*num_components > 0) {
        strncpy(filename, components[*num_components - 1], MAX_FILENAME_LENGTH);
        filename[MAX_FILENAME_LENGTH] = '\0';
        (*num_components)--; // Last component is the filename, not a directory
    }

    free(path_copy);
}

struct inode* find_directory(struct inode* parent, char* dir_name) {
    if (parent == NULL || parent->type != INODE_DIRECTORY) {
        return NULL;
    }

    for (int i = 0; i < MAX_DIRECT_BLOCKS; i++) {
        if (parent->direct_blocks[i] == -1) {
            break; // No more data blocks in this directory
        }

        dir_entry* entries = (dir_entry*)(fs + parent->direct_blocks[i] * BLOCK_SIZE);
        for (int j = 0; j < ENTRIES_PER_BLOCK; j++) {
            if (strlen(entries[j].name) == 0) {
                break; // No more entries in this block
            }

            if (strcmp(entries[j].name, dir_name) == 0) {
                return &inodes[entries[j].inode_num];
            }
        }
    }

    return NULL;
}

struct inode* create_directory(const struct inode* parent, const char* dir_name) {
    if (parent == NULL || parent->type != INODE_DIRECTORY) {
        return NULL;
    }

    // Find a free inode for the directory
    int dir_inode_index = find_free_inode();
    if (dir_inode_index < 0) {
        return NULL;
    }

    // Find a free block for the directory's data
    int dir_block_index = find_free_block();
    if (dir_block_index < 0) {
        return NULL;
    }

    // Update the parent directory's inode with the new directory entry
    // (Assuming you have a function to update a directory's contents)

    // Update the new directory's inode
    struct inode* dir_inode = &inodes[dir_inode_index];
    dir_inode->type = INODE_DIRECTORY;
    dir_inode->size = 0;
    dir_inode->direct_blocks[0] = dir_block_index;

    // Update the new directory's data block
    dir_entry* entries = (dir_entry*)(fs + dir_block_index * BLOCK_SIZE);
    memset(entries, 0, BLOCK_SIZE); // Clear the block
    strncpy(entries[0].name, ".", MAX_FILENAME_LENGTH);
    entries[0].name[MAX_FILENAME_LENGTH] = '\0';
    entries[0].inode_num = dir_inode_index;
    strncpy(entries[1].name, "..", MAX_FILENAME_LENGTH);
    entries[1].name[MAX_FILENAME_LENGTH] = '\0';
    entries[1].inode_num = parent - inodes;

    return dir_inode;
}

int find_free_inode() {
    for (int i = 0; i < sb->ninodes; i++) {
        if (inodes[i].type == UNUSED) {
            return i;
        }
    }

    return -1;
}

int find_free_block() {
    int bitmap_size = sb->nblocks / 8;
    if (sb->nblocks % 8 != 0) bitmap_size++;
    unsigned char* free_block_list = fs + sizeof(struct superblock);
    for (int i = 0; i < bitmap_size; i++) {
        if (free_block_list[i] != 0xFF) {
            for (int j = 0; j < 8; j++) {
                if ((free_block_list[i] & (1 << j)) == 0) {
                    return i * 8 + j;
                }
            }
        }
    }

    return -1;
}