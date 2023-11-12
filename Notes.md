##Innodes 
You can access the inode whith ls -i in the terminal. 
The linux kernal command "stat" will give you more information about the inode.

##Superblocks
These are key blocks of data that contain information about the file system. Information such as
the size of the file system, the block size, the empty and full blocks, and the number of free inodes.

##Implementation 
1. Define the File System Structure
Before you start coding, you should define the layout of your file system. This includes:

Superblock: Contains metadata about the file system (e.g., size of the file system, number of blocks, number of inodes, etc.).
Free Block List: A bitmap or list indicating which blocks in the file system are free or used.
Inode Table: A list of inodes, where each inode represents a file or directory.
Data Blocks: The actual storage space for file and directory data.
2. Implement the formatfs Function
The formatfs function will initialize and format the file system structure within your file system file. Here's what it should do:

Initialize the Superblock: Set the initial values for the file system parameters.
Initialize the Free Block List: Mark all blocks as free initially.
Initialize the Inode Table: Set up the inodes, marking them as unused initially.
Clear the Data Blocks: Optionally, clear the data blocks.
3. Steps for Implementation
Start with the Superblock:

Define a struct for the superblock in fs.h.
In fs.c, allocate the first few bytes of the fs array to store the superblock.
Implement the Free Block List:

Decide on the implementation (bitmap or list).
Allocate space in the fs array after the superblock for this list.
Set Up the Inode Table:

Define a struct for inodes in fs.h.
Allocate space for the inode table in the fs array after the free block list.
Data Blocks Initialization:

The remaining space in the fs array will be your data blocks.
You might not need to do anything specific for initialization here, as they will be used when files/directories are added.
Writing the Format Function:

In the formatfs function, initialize these structures with default values.
For example, mark all inodes as free, set the size parameters in the superblock, and initialize the free block list.
Testing:

After implementing formatfs, test it to ensure that it correctly initializes the file system structure.
Next Steps
After completing the formatfs function, you'll have a solid foundation for your file system. The next steps will involve implementing functions to create, read, write, and delete files and directories, as well as managing the inode table and free block list as files are added and removed.