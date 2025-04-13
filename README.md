## Goal 
- Write a C program to detect duplicate files in a file system sub-tree 
- This will be done by using a hashing algorithm to hash the value of a file. That is, even if there is a difference of 1 bit in 1 byte, the hash function outputs should be **very different**. 
	- Here the MD5 hashing algorithm will be used as a standard that employs mechanisms to avoid collision 
### False Duplicates
- A true duplicate is one where the blocks on the storage media are actual duplications
- Even if two files appear the same, they may not be a true duplicate, but rather share the same underlying block in the storage system 
## Key Definitions
- **Nodes**: can be a regular file (byte array), a directory (parent of other nodes), devices (type of [[Nodes (Linux)#Special Files|special file]]), or kernel variables (another type of special file)
- **Deduplication**: 
![[Deduplication (Linux)#Overview]] 
## Key Support Files 
- `uthash.h`: hash table implementation 
- `detect_dups.c`, `detect_dups.h`: code and function signatures 
## Input
```bash
./detect_dups example_dir 
``` 
## Output
```c
File <number>:  
	MD5 Hash: <MD5 hash>  
	Hard Link (<reference count>): <Inode number>  
		Paths: <Path 1>  
			...  
			<Path N>  
	
	Soft Link <number>(<reference count>): <Inode number>  
		Paths: <Path 1>  
			...  
			<Path N>
```
- **File**: if file1.txt and file2.txt contain the same contents, they will appear under `File n`, else they will appear under `File n` and `File n+1` 
- **Hard Link**: if file1.txt and file2.txt were created by the user with the same contents, they have a different `<Inode number>`. If they are hard links pointing to the same inode number, the `<reference_count>` would increase and their `<Inode number>` would be the same 
	- Different Inode Number,  
		- Hard Link (1): 38974691  Paths: file1.txt  
		- Hard Link (1): 28736422  Paths: file1.txt   
	- Same Inode Number - reference count increased,  
		- Hard Link (2): 38974691  Paths: file1.txt  file2.txt
- **Soft Link**: link to a unique "Path". Since a soft link is unique to each file, it will be **under** each Hard Link
### A `printf()` Template
```c
"File <number>:"  
"\tMD5 Hash: <hash>"  
"\t\tHard Link (<count>): <inode>  
"\t\t\tPaths:\t<Path 1>"  
"\t\t\t\t<Path N>"  
"\t\t\tSoft Link <number>(<count>): <inode>"  
"\t\t\t\tPaths:\t<Path 1>"  
"\t\t\t\t\t<Path N>
```
