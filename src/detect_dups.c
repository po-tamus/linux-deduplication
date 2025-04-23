// add any other includes in the detect_dups.h file
#include "detect_dups.h"

// define any other global variable you may need over here
#define MAX_LEN 4096
#define PATH_MAX 100
// holds the files hash table
file_node *files = NULL;
// holds the current file number 
int file_number = 1; 


// open ssl, this will be used to get the hash of the file
EVP_MD_CTX *mdctx;
const EVP_MD *EVP_md5(); 

int main(int argc, char *argv[]) {

    // perform error handling, "exit" with failure incase an error occurs 
    if (argc!=2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (nftw(argv[1], render_file_info, 20, FTW_PHYS)==-1) {
        fprintf(stderr, "%s is not a valid directory\n", argv[1]);
        exit(EXIT_FAILURE);
    }

	file_node *current_file; 
	for (current_file=files; current_file!=NULL; current_file=current_file->hh.next) {
		// file name and number
		printf("File %d\n", current_file->number);
		// md5 hash
		printf("\tMD5 Hash: ");  
		for (int i = 0; i < current_file->md5_len; i++) {  
			// last two digits of the hex value in each index i
			printf("%02x", current_file->md5_hash[i]);  
		}  
		printf("\n"); 

		// hard link count and inode 
		hard_link_node *curr_hard_link;
		for (curr_hard_link=current_file->hard_links; curr_hard_link!=NULL; curr_hard_link=curr_hard_link->hh.next) {
			printf("\tHard Link (%d): %lu\n", curr_hard_link->num_files, curr_hard_link->inode);
			
			// paths
			printf("\t\tPaths: %s\n", curr_hard_link->head->name); 
			file_name_LL *curr = curr_hard_link->head->next;
			while (curr!=NULL) {
				printf("\t\t\t%s\n", curr->name);
				curr=curr->next;
			}

			soft_link_node *curr_soft_link;
			for (curr_soft_link=curr_hard_link->soft_links; curr_soft_link!=NULL; curr_soft_link=curr_soft_link->hh.next) {
				printf("\t\tSoft Link %d(%d): %lu\n", curr_soft_link->number, curr_soft_link->num_files, curr_soft_link->inode);
				
				// soft link paths
				printf("\t\t\t\tPaths: %s\n", curr_soft_link->head->name); 
				file_name_LL *curr = curr_soft_link->head->next;
				while (curr!=NULL) {
					printf("\t\t\t\t\t%s\n", curr->name);
					curr=curr->next;
				}	
			}
		}
	}
}

// render the file information invoked by nftw
static int render_file_info(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {


	if (tflag==FTW_D) {
		return 0;
	} else if (tflag==FTW_F||tflag==FTW_SL) {

		

		// perform the inode operations over here
		unsigned char md5_value[EVP_MAX_MD_SIZE];  
		unsigned int md5_len = 16;
		int err;  

		// we must create context for the MD5 algorithm 
		EVP_MD_CTX *mdctx = EVP_MD_CTX_new();  

		// error handling, if it doesn't work for some reason 
		if (!mdctx) {  
			fprintf(stderr, "%s::%d::Error allocating MD5 context %d\n", __func__, __LINE__, errno);  
			exit(EXIT_FAILURE);  
		}  
		
		// compute the md5 hash
		err = compute_file_hash(fpath, mdctx, md5_value, &md5_len);

		if (err < 0) {  
			fprintf(stderr, "%s::%d::Error computing MD5 hash %d\n", __func__, __LINE__, errno);  
			exit(EXIT_FAILURE);
		} 


		// md5_value contains raw binary data 
		// since it's an array of unsigned bytes, it can be accessed as a char array
		if (find_file(md5_value)==NULL) {

			struct stat test; 

			if (tflag==FTW_SL&&stat(fpath, &test)==0) {
				if (S_ISDIR(test.st_mode)) {
					return 0;
				}
			}
			
			file_node *new_file = (file_node*)malloc(sizeof(file_node));
			// new_file->md5_hash = (unsigned char*)malloc(sizeof(unsigned char)*md5_len);
			if (new_file->md5_hash == NULL) {
				fprintf(stderr, "Memory allocation failed\n");
				exit(EXIT_FAILURE);
			}
			memcpy(new_file->md5_hash, md5_value, md5_len); 
			new_file->number=file_number; 
			new_file->hard_links=NULL; 
			new_file->md5_len=md5_len; 
			add_file(new_file); 
			file_number++; 

		}

		// get the hard link hash map from the files hash map 
		file_node *curr_file = find_file(md5_value); 
		if (curr_file==NULL) {
			fprintf(stderr, "Null struct in the hash map?\n");
			return -1;
		}

		if (tflag==FTW_F) {			
			
			hard_link_node *curr_hard_link = find_hard_link(&(sb->st_ino), curr_file);
			
			if (curr_hard_link==NULL) {

				// new hard link node i.e. entry point to the hash map
				hard_link_node *new_hard_link = (hard_link_node*)malloc(sizeof(hard_link_node));
				
				// set the inode and num_files of the hard link entry
				new_hard_link->inode = sb->st_ino; 
				new_hard_link->num_files=1;
				new_hard_link->soft_links=NULL; 
				new_hard_link->number_of_soft_links=0;
				new_hard_link->head=NULL;
				curr_hard_link = new_hard_link; 
				
				add_hard_link(new_hard_link, curr_file); 
				
			} else {
				// increment the count, append to the path 
				
				curr_hard_link->num_files++; 
			}

			// new path linked list
			file_name_LL *new_path = (file_name_LL*)malloc(sizeof(file_name_LL));
			new_path->name=malloc(strlen(fpath));
			strcpy(new_path->name, fpath);
			if (curr_hard_link->head==NULL) {
				curr_hard_link->head = new_path;
			} else {
				new_path->next=(curr_hard_link->head); 
				curr_hard_link->head=new_path;
			}

			
		} else {

			struct stat target_stat;

			// Follow the symlink to get info about its target
			if (stat(fpath, &target_stat) == 0) {

				if (S_ISDIR(target_stat.st_mode)) {
					// if (fpath!=NULL){
					// 	printf("PATH: %s\n", fpath);
					// }
					return 0;
				}

				char buf[PATH_MAX];
				ssize_t path_length = readlink(fpath, buf, PATH_MAX);
				buf[path_length]='\0';

				
				hard_link_node *target_hard_link = find_hard_link(&(target_stat.st_ino), curr_file);
				
				if (target_hard_link==NULL) {
					
					create_hard_link(buf, curr_file); 
					
					target_hard_link = find_file_in_paths(buf, curr_file, target_stat.st_ino);
					
					
				} 
								
				soft_link_node *curr_soft_link = find_soft_link(&(sb->st_ino), target_hard_link);
				if (curr_soft_link==NULL) {
					
					
					// set the inode and num_files of the hard link entry
					soft_link_node *new_soft_link = (soft_link_node*)malloc(sizeof(soft_link_node));
					target_hard_link->number_of_soft_links++;
					new_soft_link->number = target_hard_link->number_of_soft_links;
					
					new_soft_link->inode = sb->st_ino;
					new_soft_link->num_files=1;
					new_soft_link->head = NULL;
					
					add_soft_link(new_soft_link, target_hard_link);	
					curr_soft_link = new_soft_link;
				} else {
					curr_soft_link->num_files++;
				}
				
				// new path linked list
				file_name_LL *new_path = (file_name_LL*)malloc(sizeof(file_name_LL));
				new_path->name=malloc(strlen(fpath));
				strcpy(new_path->name, fpath);

				if (curr_soft_link->head==NULL) {
					curr_soft_link->head = new_path;
				} else {
					new_path->next=curr_soft_link->head; 
					curr_soft_link->head=new_path;
				}
				

				
			} else {
				perror("stat failed on symlink target");
			}
			
		}

	}	 
	
	// free the malloc'd space for the digest, but make sure we don't call free() again... or risk memory corruption 
	EVP_MD_CTX_free(mdctx); 
	
	return 0;  
}

// add any other functions you may need over here

void create_hard_link(char* path, file_node *curr_file) {
	// new hard link node i.e. entry point to the hash map
	hard_link_node *new_hard_link = (hard_link_node*)malloc(sizeof(hard_link_node));
	struct stat sb; 
	
	// this populates sb with the relevant information 
	if (stat(path, &sb) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    } 

	// set the inode and num_files of the hard link entry
	new_hard_link->inode = sb.st_ino; 
	new_hard_link->num_files=0;
	new_hard_link->soft_links=NULL;
	new_hard_link->number_of_soft_links=0;
	new_hard_link->head=NULL;
	
	add_hard_link(new_hard_link, curr_file); 
	
	
	// REFORMAT 
	// file_name_LL *new_path = (file_name_LL*)malloc(sizeof(file_name_LL));
	// new_path->name=malloc(strlen(path));
	// strcpy(new_path->name, path);
	// new_hard_link->head = new_path; 
	
}

/**
 * use this to find which hard link a file belongs to when 
 * exploring a soft link 
 */
hard_link_node* find_file_in_paths(char *fpath, file_node *curr_file, ino_t inode) {
	hard_link_node *curr_hard_link = find_hard_link(&inode, curr_file);
	hard_link_node *ptr=curr_hard_link; 


	while (ptr!=NULL) {
		


		struct stat new_sb;
		if (stat(fpath, &new_sb) != -1) {
			// printf("found the hard link for the file\n");
			return ptr;
		}
		

		ptr=ptr->hh.next;
	}

	return NULL;
}

/**
 * path: Path to the file to hash
 * mdctx: Digest context for MD5
 * md_value: Buffer to store the computed hash
 * md5_len: Pointer to store the length of the hash
 */

int compute_file_hash(const char* path, EVP_MD_CTX *mdctx, unsigned char *md_value, unsigned int *md5_len) {
	FILE *fd = fopen(path, "rb");

	if (fd==NULL) {
		fprintf(stderr, "%s::%d::Error opening file %d: %s\n", __func__, __LINE__, errno, path); 
		return -1; 
	}

	// buffer to read and flag for read status
	char buff[MAX_LEN]; 
	size_t n;

	EVP_DigestInit_ex(mdctx, EVP_md5(), NULL); 

	while ((n=fread(buff, 1, MAX_LEN, fd))) {
		EVP_DigestUpdate(mdctx, buff, n);
	}

	// end of updates to the hash value
	EVP_DigestFinal_ex(mdctx, md_value, md5_len);
	// reset the context 
	EVP_MD_CTX_reset(mdctx);

	fclose(fd);
	return 0;
}

void add_file(file_node *new_file) {
    HASH_ADD(hh, files, md5_hash, 16, new_file);
}

file_node *find_file(char *md5_hash) {
	// should return a pointer to a file_node struct 
	file_node *target; 
	// handle, hash map head, target, length, return value 
	HASH_FIND(hh, files, md5_hash, 16, target); 
	return target; 
} 

void add_hard_link(hard_link_node *new_hard_link, file_node *curr_file) {
	HASH_ADD(hh, curr_file->hard_links, inode, sizeof(ino_t), new_hard_link);
}

hard_link_node *find_hard_link(ino_t *inode, file_node *curr_file) {
	// should return a pointer to a file_node struct 
	hard_link_node *target; 
	// handle, hash map head, target, length, return value 
	HASH_FIND(hh, curr_file->hard_links, inode, sizeof(ino_t), target);
	return target; 
}

void add_soft_link(soft_link_node *new_soft_link, hard_link_node *curr_hard_link) {
	
	HASH_ADD(hh, curr_hard_link->soft_links, inode, sizeof(ino_t), new_soft_link);
	
}

soft_link_node *find_soft_link(ino_t *inode, hard_link_node *target_hard_link) {
	// should return a pointer to a file_node struct 
	soft_link_node *target; 
	// handle, hash map head, target, length, return value 
	HASH_FIND(hh, target_hard_link->soft_links, inode, sizeof(ino_t), target); 
	return target; 
}
