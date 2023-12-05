#include "fs.h"
#include <string.h>

unsigned char* fs;
struct superblock* sb = NULL; //Initiate the superblock once
struct inode* inodes = NULL; //Initiate the root inode once
struct inode* root = NULL; //Initiate the root inode once
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


void formatfs(){
   if (!is_mapped) {
        fprintf(stderr, "File system is not memory-mapped. Call mapfs first.\n");
        exit(EXIT_FAILURE);
   }

   // Initialize the superblock
   sb = (struct superblock*)fs;
   sb->nblocks = FSSIZE / BLOCK_SIZE; // Total file system size divided by block size 
   sb->ninodes = 100;// Number of inodes
   sb->magic = 0xdeadbeef;

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
  sb = (struct superblock*)fs;
  if (!is_mapped) {
    fprintf(stderr, "File system is not memory-mapped. Call mapfs first.\n");
    exit(EXIT_FAILURE);
  }

  // Correct the order of checks here
  if (sb == NULL) {
    fprintf(stderr, "Superblock not found. Call formatfs first.\n");
    exit(EXIT_FAILURE);
  }

  if (sb->magic != 0xdeadbeef) {
    fprintf(stderr, "Magic number not found. Call formatfs first.\n");
    exit(EXIT_FAILURE);
  }

  // Initialize the inode table
  inodes = (struct inode*)(fs + sizeof(struct superblock) + sb->nblocks / 8);
  root = &inodes[0]; // Set up the root inode
}



void lsfs(){

  if (root == NULL || root->type != INODE_DIRECTORY) {
        printf("Root directory not found or is not a directory.\n");
        return;
    }

    // Iterate through the root directory's direct blocks
    for (int i = 0; i < MAX_DIRECT_BLOCKS; i++) {
        if (root->direct_blocks[i] == -1) break; // No more entries in this block
        dir_entry* entries = (dir_entry*)(fs + root->direct_blocks[i] * BLOCK_SIZE);
        for (int j = 0; j < ENTRIES_PER_BLOCK; j++) {
            if (entries[j].inode_num == -1) break; // No more entries in this block
            printf("%s\n", entries[j].name);
        }
    }
}

void addfilefs(char* fname){
    sb = (struct superblock*)fs;
    char components[256][MAX_FILENAME_LENGTH + 1];
    char filename[MAX_FILENAME_LENGTH + 1];
    int num_components;

    // Parse the path
    parse_path(fname, components, &num_components, filename);

    if (num_components == 0) {
        printf("Invalid path.\n");
        return;
    }

    // Find the parent directory
    struct inode* parent = root;
    for (int i = 0; i < num_components - 1; i++) {
        int found = 0;
        for (int j = 0; j < MAX_DIRECT_BLOCKS && parent->direct_blocks[j] != -1; j++) {
            // Ensure the block number is valid
            if (parent->direct_blocks[j] < 0 || parent->direct_blocks[j] >= sb->nblocks) {
                continue;
            }
            dir_entry* entries = (dir_entry*)(fs + parent->direct_blocks[j] * BLOCK_SIZE);
            for (int k = 0; k < ENTRIES_PER_BLOCK; k++) {
                if (strcmp(entries[k].name, components[i]) == 0 && entries[k].inode_num != -1) {
                    parent = &inodes[entries[k].inode_num];
                    found = 1;
                    break;
                }
            }
            if (found) break;
        }
        if (!found) {
            printf("Directory %s not found.\n", components[i]);
            return;
        }
    }

    // Check if file already exists in the parent directory
    int fileExists = 0;
    for (int i = 0; i < MAX_DIRECT_BLOCKS && parent->direct_blocks[i] != -1; i++) {
        // Ensure the block number is valid
        if (parent->direct_blocks[i] < 0 || parent->direct_blocks[i] >= sb->nblocks) {
            continue;
        }
        dir_entry* entries = (dir_entry*)(fs + parent->direct_blocks[i] * BLOCK_SIZE);
        for (int j = 0; j < ENTRIES_PER_BLOCK; j++) {
            if (strcmp(entries[j].name, filename) == 0 && entries[j].inode_num != -1) {
                printf("File already exists.\n");
                return;
            }
        }
    }

    if (fileExists) {
        return;
    }

    // Find a free inode for the file
    int inode_index = find_free_inode();
    if (inode_index == -1) {
        printf("No free inodes available.\n");
        return;
    }

    struct inode* file_inode = &inodes[inode_index];

    // Initialize the inode for the file
    file_inode->type = INODE_FILE;
    file_inode->size = 0;
    memset(file_inode->direct_blocks, -1, sizeof(file_inode->direct_blocks));

    // Add the file to the parent directory
    int added = 0;
    for (int i = 0; i < MAX_DIRECT_BLOCKS; i++) {
        if (parent->direct_blocks[i] == -1 || parent->direct_blocks[i] >= sb->nblocks) {
            int new_block = find_free_block();
            if (new_block == -1) {
                printf("No free blocks available.\n");
                return;
            }
            parent->direct_blocks[i] = new_block;
            memset(fs + new_block * BLOCK_SIZE, 0, BLOCK_SIZE); // Initialize the new block
        }
        dir_entry* entries = (dir_entry*)(fs + parent->direct_blocks[i] * BLOCK_SIZE);
        for (int j = 0; j < ENTRIES_PER_BLOCK; j++) {
            if (entries[j].inode_num == -1) {
                strncpy(entries[j].name, filename, MAX_FILENAME_LENGTH);
                entries[j].name[MAX_FILENAME_LENGTH] = '\0'; // Null terminate the string
                entries[j].inode_num = inode_index;
                added = 1;
                break;
            }
        }
        if (added) break;
    }

    if (!added) {
        printf("Failed to add file to directory.\n");
    }
}


void removefilefs(char* fname) {
    char components[256][MAX_FILENAME_LENGTH + 1];
    char filename[MAX_FILENAME_LENGTH + 1];
    int num_components;

    // Parse the path
    parse_path(fname, components, &num_components, filename);

    if (num_components == 0) {
        printf("Invalid path.\n");
        return;
    }

    // Find the parent directory
    struct inode* parent = root;
    for (int i = 0; i < num_components - 1; i++) {
        int found = 0;
        for (int j = 0; j < MAX_DIRECT_BLOCKS && parent->direct_blocks[j] != -1; j++) {
            if (parent->direct_blocks[j] < 0 || parent->direct_blocks[j] >= sb->nblocks) {
                continue; // Skip invalid block numbers
            }
            dir_entry* entries = (dir_entry*)(fs + parent->direct_blocks[j] * BLOCK_SIZE);
            for (int k = 0; k < ENTRIES_PER_BLOCK; k++) {
                if (entries[k].inode_num != -1 && strcmp(entries[k].name, components[i]) == 0) {
                    parent = &inodes[entries[k].inode_num];
                    found = 1;
                    break;
                }
            }
            if (found) break;
        }
        if (!found) {
            printf("Directory %s not found.\n", components[i]);
            return;
        }
    }

    // Find and remove the file entry from the parent directory
    int removed = 0;
    for (int i = 0; i < MAX_DIRECT_BLOCKS && parent->direct_blocks[i] != -1; i++) {
        if (parent->direct_blocks[i] < 0 || parent->direct_blocks[i] >= sb->nblocks) {
            continue; // Skip invalid block numbers
        }
        dir_entry* entries = (dir_entry*)(fs + parent->direct_blocks[i] * BLOCK_SIZE);
        for (int j = 0; j < ENTRIES_PER_BLOCK; j++) {
            if (entries[j].inode_num != -1 && strcmp(entries[j].name, filename) == 0) {
                // Invalidate the directory entry
                entries[j].inode_num = -1;
                memset(entries[j].name, 0, MAX_FILENAME_LENGTH);
                removed = 1;
                break;
            }
        }
        if (removed) break;
    }

    if (!removed) {
        printf("File %s not found.\n", fname);
    }
}

void extractfilefs(char* fname) {
    char components[256][MAX_FILENAME_LENGTH + 1];
    char filename[MAX_FILENAME_LENGTH + 1];
    int num_components;

    // Parse the path
    parse_path(fname, components, &num_components, filename);

    if (num_components == 0) {
        printf("Invalid path.\n");
        return;
    }

    // Traverse the path to find the file inode
    struct inode* current = root;
    for (int i = 0; i < num_components; i++) {
        int found = 0;
        for (int j = 0; j < MAX_DIRECT_BLOCKS && current->direct_blocks[j] != -1; j++) {
            dir_entry* entries = (dir_entry*)(fs + current->direct_blocks[j] * BLOCK_SIZE);
            for (int k = 0; k < ENTRIES_PER_BLOCK; k++) {
                if (entries[k].inode_num != -1 && strcmp(entries[k].name, components[i]) == 0) {
                    current = &inodes[entries[k].inode_num];
                    found = 1;
                    break;
                }
            }
            if (found) break;
        }
        if (!found) {
            printf("File or directory %s not found.\n", components[i]);
            return;
        }
    }

    // Check if the inode is a file
    if (current->type != INODE_FILE) {
        printf("The path does not lead to a file.\n");
        return;
    }

    // Read file content from file system and write to stdout
    for (int i = 0; i < MAX_DIRECT_BLOCKS && current->direct_blocks[i] != -1; i++) {
        fwrite(fs + current->direct_blocks[i] * BLOCK_SIZE, BLOCK_SIZE, 1, stdout);
    }
}


//USER ADDED FUNCTIONS
void parse_path(const char* path, char components[256][MAX_FILENAME_LENGTH + 1], int* num_components, char* filename) {
    // Check for an empty or too long path
    size_t path_length = strlen(path);
    if (path_length == 0 || path_length > MAX_FILENAME_LENGTH) {
        *num_components = 0;
        filename[0] = '\0';
        return;
    }

    // Initialize the components array
    for (int i = 0; i < 256; i++) {
        memset(components[i], 0, MAX_FILENAME_LENGTH + 1);
    }

    // Initialize the filename
    memset(filename, 0, MAX_FILENAME_LENGTH + 1);

    // Copy the path into a temporary buffer
    char path_copy[MAX_FILENAME_LENGTH + 1];
    strncpy(path_copy, path, MAX_FILENAME_LENGTH);
    path_copy[MAX_FILENAME_LENGTH] = '\0'; // Ensure null termination

    // Tokenize the path
    char* token = strtok(path_copy, "/");
    int i = 0;
    while (token != NULL && i < 256) {
        strncpy(components[i], token, MAX_FILENAME_LENGTH);
        components[i][MAX_FILENAME_LENGTH] = '\0'; // Ensure null termination
        token = strtok(NULL, "/");
        i++;
    }

    // Set the number of components
    *num_components = i;

    // Copy the last component into the filename, if available
    if (i > 0) {
        strncpy(filename, components[i - 1], MAX_FILENAME_LENGTH);
    }
}

int find_free_inode() {
    for (int i = 0; i < sb->ninodes; i++) {
        if (inodes[i].type == 255) {
            return i;
        }
    }
    return -1;
}


int find_free_block() {
    unsigned char* free_block_list = fs + sizeof(struct superblock);
    int bitmap_size = sb->nblocks / 8;
    if (sb->nblocks % 8 != 0) bitmap_size++;
    for (int i = 0; i < bitmap_size; i++) {
        for (int j = 0; j < 8; j++) {
            if ((free_block_list[i] & (1 << j)) == 0) {
                return i * 8 + j;
            }
        }
    }
    return -1;
}
