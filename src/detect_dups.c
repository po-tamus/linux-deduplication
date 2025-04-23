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
const EVP_MD *EVP_md5(); // use md5 hash!!

int main(int argc, char *argv[]) {
    // perform error handling, "exit" with failure incase an error occurs 

    if (argc!=2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // need to handle if "directory is not a valid directory"
    // what makes a directory invalid? if the call with nftw fails? 
    
	// printf("top of main method: root is %s\n", argv[1]);

    if (nftw(argv[1], render_file_info, 20, FTW_PHYS)==-1) {
        fprintf(stderr, "%s is not a valid directory\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // initialize the other global variables you have, if any

    // add the nftw handler to explore the directory
    // nftw should invoke the render_file_info function

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
		//printf("(Directory)\n");
		//printf("level=%02d, size=%07ld path=%s filename=%s\n",
		//ftwbuf->level, sb->st_size, fpath, fpath + ftwbuf->base);
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

		// check
		// if so, proceed 
		// if not, add

		// md5_value contains raw binary data 
		// since it's an array of unsigned bytes, it can be accessed as a char array
		if (find_file(md5_value)==NULL) {
			
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

			// print the newly added file
			// printf("****NEW FILE MD5 IS:****\n");  
			// for (int i = 0; i < md5_len; i++) {  
			// 	printf("%02x", md5_value[i]);  
			// }  
			// printf("\n"); 

			// file_node *added_file;
			
			// // this is printing the file that was added, 
			// for (added_file=files; added_file!=NULL; added_file=added_file->hh.next) {
			// 	printf("****SHOULD PRINT OUT THE HASH OF THE ADDED****\n");
			// 	unsigned int length = added_file->md5_len; 
			// 	unsigned char *hash = (unsigned char*)malloc(sizeof(unsigned char)*length);
			// 	hash = added_file->md5_hash;
			// 	for (int i = 0; i < length; i++) {  
			// 		printf("%02x", hash[i]);  
			// 	}  
			// 	printf("\n");
			// }

			// added_file=find_file(new_file->md5_hash);
			// if (added_file==NULL) {
			// 	printf("****FIND ISSUE****\n");
			// } else {
			// 	printf("****FINDING WITH THE ADDED FILE WORKS****\n");
			// }
		}

		// get the hard link hash map from the files hash map 
		file_node *curr_file = find_file(md5_value); 
		if (curr_file==NULL) {
			fprintf(stderr, "Null struct in the hash map?\n");
			return -1;
		}

		if (tflag==FTW_F) {

			// printf("Regular File: %s\n", fpath);
			
			
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


				// if (curr_file->hard_links==NULL) {
				// 	printf("****HARD LINK IS NULL HERE****\n"); 
				// } else {
				// 	printf("****HARD LINK+****\n");
				// }
				
			} else {
				// increment the count, append to the path 
				
				curr_hard_link->num_files++; 
			}

			// new path linked list
			file_name_LL *new_path = (file_name_LL*)malloc(sizeof(file_name_LL));
			new_path->name=malloc(strlen(fpath));
			strcpy(new_path->name, fpath);
			//printf("path name is: %s\n", fpath);
			if (curr_hard_link->head==NULL) {
				curr_hard_link->head = new_path;
			} else {
				new_path->next=(curr_hard_link->head); 
				curr_hard_link->head=new_path;
			}

			
		} else {

			//printf("(Symbolic Link)\n");
			struct stat target_stat;

			// Follow the symlink to get info about its target
			if (stat(fpath, &target_stat) == 0) {
				
				/**
				 * 1. determine the file the symlink points to 
				 * 2. iterate through the hard link hash map of the current file
				 * 3. iterate through each of their path linked lists to find the file
				 * 4. if found, add the soft link under that hard link
				 * 5. else, handle the new hard link first 
				 */

				char buf[PATH_MAX];
				ssize_t path_length = readlink(fpath, buf, PATH_MAX);
				buf[path_length]='\0';

				// char symlink_abs[PATH_MAX], target_abs[PATH_MAX];
				// realpath(fpath, symlink_abs); // resolves the symlink itself
				// realpath(buf, target_abs); // resolves the target path

				// char* symlink_dir = dirname(strdup(symlink_abs));

				// char* relative = get_relative_path(symlink_dir, target_abs);
				// printf("Relative path from symlink to target: %s\n", relative);

				// free(relative);

				
				// const char* cwd = getcwd(NULL, 0);
				// if (cwd!=NULL) {
				// 	printf("****HERE****\n");
				// }
				// if (path_length==-1) {
				// 	printf("****HEREYEAH****\n");
				// }
				// printf("****cwd: %s****\n", cwd);
				
				// size_t cwd_len = strlen(cwd);
				
				// checking if the first cwd_len characters match
				// if (strncmp(buf, cwd, cwd_len) == 0) {
					
				// 	// Strip off cwd to get relative path
				// 	// by moving forward in memory 
				// 	const char* relative_path = buf + cwd_len;
				// 	if (*relative_path == '/') relative_path++;  // remove leading slash
				// 	printf("Relative path: %s\n", relative_path);
				// }
				
				// hard_link_node *target_hard_link = find_file_in_paths(buf, curr_file, target_stat.st_ino);
				
				hard_link_node *target_hard_link = find_hard_link(&(target_stat.st_ino), curr_file);
				
				if (target_hard_link==NULL) {
					/**
					 * what do we need to handle the hard link now?
					 * 
					 */
					
					create_hard_link(buf, curr_file); 
					
					target_hard_link = find_file_in_paths(buf, curr_file, target_stat.st_ino);
					
					
				} 
				
				// target_hard_link contains the hard link
				// under which we should enter the soft link
				// enter it in the soft links hash map in target_hard_link 

				
				// here
				
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
					// increment the count, append to the path 
					//dont need to do anything here 
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


	// // perform the inode operations over here
	// unsigned char md5_value[EVP_MAX_MD_SIZE];  
	// unsigned int md5_len = 0; 
	// int err;  

	// // we must create context for the MD5 algorithm 
	// EVP_MD_CTX *mdctx = EVP_MD_CTX_new();  

	// // error handling, if it doesn't work for some reason 
	// if (!mdctx) {  
	// 	fprintf(stderr, "%s::%d::Error allocating MD5 context %d\n", __func__, __LINE__, errno);  
	// 	exit(EXIT_FAILURE);  
	// }  
	
	// // compute the md5 hash
	// err = compute_file_hash(fpath, mdctx, md5_value, &md5_len);

	// if (err < 0) {  
	// 	fprintf(stderr, "%s::%d::Error computing MD5 hash %d\n", __func__, __LINE__, errno);  
	// 	exit(EXIT_FAILURE);
	// } 

	// // check
	// // if so, proceed 
	// // if not, add

	// // md5_value contains raw binary data 
	// // since it's an array of unsigned bytes, it can be accessed as a char array
	// if (find_file(md5_value)==NULL) {
	// 	file_node *new_file = (file_node*)malloc(sizeof(file_node));
	// 	new_file->md5_hash = malloc(sizeof(unsigned char)*md5_len);
	// 	if (new_file->md5_hash == NULL) {
	// 		fprintf(stderr, "Memory allocation failed\n");
	// 		exit(EXIT_FAILURE);
	// 	}
	// 	memcpy(new_file->md5_hash, md5_value, md5_len);
	// 	new_file->number=file_number;
	// 	new_file->hard_links=NULL;
	// 	new_file->md5_len=md5_len;
	// 	add_file(new_file);
	// 	file_number++;
	// }

	// // get the hard link hash map from the files hash map 
	// file_node *curr_file = find_file(md5_value); 
	// if (curr_file==NULL) {
	// 	fprintf(stderr, "Null struct in the hash map?\n");
	// 	return -1;
	// }
    

    // switch (tflag) {

	// 	// if it's a file perform operations
	// 	// here it's a hard link? 
	// 	case FTW_F:
	// 		// printf("Regular File:\n");
	// 		// if ( S_ISBLK(sb->st_mode) ) {
	// 		// 	printf(" (Block Device)");
	// 		// } else if ( S_ISCHR(sb->st_mode) ) {
	// 		// 	printf(" (Character Device)");    
	// 		// }

	// 		// get the hard link
	// 		hard_link_node *curr_hard_link = find_hard_link(sb->st_ino, curr_file->hard_links);
	// 		if (curr_hard_link==NULL) {

	// 			// new hard link node i.e. entry point to the hash map
	// 			hard_link_node *new_hard_link = (hard_link_node*)malloc(sizeof(hard_link_node));
				
	// 			// set the inode and num_files of the hard link entry
	// 			new_hard_link->inode = (unsigned long)sb->st_ino; 
	// 			new_hard_link->num_files=1;
	// 			new_hard_link->soft_links=NULL;
	// 			curr_hard_link = new_hard_link;
	// 			add_hard_link(new_hard_link, curr_file->hard_links);
	// 		} else {
	// 			// increment the count, append to the path 
	// 			curr_hard_link->num_files++;
	// 		}

	// 		// new path linked list
	// 		file_name_LL *new_path = (file_name_LL*)malloc(sizeof(file_name_LL));
	// 		new_path->name=fpath;
	// 		curr_hard_link->head = new_path;
	// 		break;
		
	// 	// if it's a directory, nftw will handle it
	// 	case FTW_D:
	// 		printf(" (Directory) \n");
	// 		printf("level=%02d, size=%07ld path=%s filename=%s\n",
	// 		ftwbuf->level, sb->st_size, fpath, fpath + ftwbuf->base);
	// 		break; 

	// 	// if it's a symlink, we find the file it hashes to and add to its symlinks 
	// 	case FTW_SL:
	// 		printf(" (Symbolic Link) \n");

	// 		struct stat target_stat;

	// 		// Follow the symlink to get info about its target
	// 		if (stat(fpath, &target_stat) == 0) {

	// 			/**
	// 			 * 1. determine the file the symlink points to 
	// 			 * 2. iterate through the hard link hash map of the current file
	// 			 * 3. iterate through each of their path linked lists to find the file
	// 			 * 4. if found, add the soft link under that hard link
	// 			 * 5. else, handle the new hard link first 
	// 			 */

	// 			char* buf[PATH_MAX];
	// 			ssize_t path_length = readlink(fpath, buf, PATH_MAX);
	// 			buf[path_length]='\0';
	// 			hard_link_node *target_hard_link = find_file_in_paths(buf, curr_file, (unsigned long)target_stat.st_ino);

	// 			if (target_hard_link == NULL) {
	// 				/**
	// 				 * what do we need to handle the hard link now?
	// 				 * 
	// 				 */

	// 				create_hard_link(buf, curr_file->hard_links);
	// 				target_hard_link = find_file_in_paths(buf, curr_file, (unsigned long)target_stat.st_ino);
	// 			} 
				
	// 			// target_hard_link contains the hard link
	// 			// under which we should enter the soft link
	// 			// enter it in the soft links hash map in target_hard_link 

	// 			soft_link_node *soft_links = target_hard_link->soft_links;
	// 			// here
				
	// 			soft_link_node *curr_soft_link = find_soft_link(sb->st_ino, soft_links);
	// 			if (curr_soft_link==NULL) {
					
	// 				// set the inode and num_files of the hard link entry
	// 				soft_link_node *new_soft_link = (soft_link_node*)malloc(sizeof(soft_link_node));
	// 				new_soft_link->number = target_hard_link->number_of_soft_links;
	// 				new_soft_link->inode = (unsigned long)sb->st_ino;
	// 				file_name_LL *head = (file_name_LL*)malloc(sizeof(file_name_LL)); 
	// 				new_soft_link->head = head;
	// 				add_soft_link(new_soft_link, soft_links);	
	// 			} else {
	// 				// increment the count, append to the path 
	// 				//dont need to do anything here 
	// 				// curr_soft_link->num_files++;
	// 			}

	// 			// new path linked list
	// 			file_name_LL *new_path = (file_name_LL*)malloc(sizeof(file_name_LL));
	// 			new_path->name=fpath;
	// 			curr_soft_link->head = new_path;
				

				
	// 		} else {
	// 			perror("stat failed on symlink target");
	// 		}

	// 		break;

	// 	/**
	// 	 * for all below, just skip them? 
	// 	 */
	// 	case FTW_NS:
	// 		// printf(" (Unreadable) \n");
	// 		break;
	// 	case FTW_DNR:
	// 		// printf(" (Directory cannot be read) \n");
	// 		break;
	// 	case FTW_SLN:
	// 		// printf(" (Symbolic link refers to non-existent file)\n");
	// 		break;
	// 	default:
	// 		// if (S_ISFIFO(sb->st_mode)) {
	// 		// 	printf(" (FIFO)");
	// 		// }
	// 		break;
	// }

    // invoke any function that you may need to render the file information
	
	/**
	 * here we are accessing the contents ofthe md5_value array, which is an int array
	 * that is why it must be accessed like this 
	 * to grab the entire thing, could we just take that slice from memory? 
	 * in the sample output, not all values from the md5 hash are ints, some are chars (ascii?)
	 * test... 
	 */
	 
	
	// free the malloc'd space for the digest, but make sure we don't call free() again... or risk memory corruption 
	EVP_MD_CTX_free(mdctx); 
	
	return 0;  
}

// Helper: Get relative path from 'from' to 'to'
char* get_relative_path(const char* from, const char* to) {
    char from_copy[PATH_MAX];
    char to_copy[PATH_MAX];

    strncpy(from_copy, from, PATH_MAX);
    strncpy(to_copy, to, PATH_MAX);

    char *from_tokens[PATH_MAX], *to_tokens[PATH_MAX];
    int from_len = 0, to_len = 0;

    char *token = strtok(from_copy, "/");
    while (token) {
        from_tokens[from_len++] = token;
        token = strtok(NULL, "/");
    }

    token = strtok(to_copy, "/");
    while (token) {
        to_tokens[to_len++] = token;
        token = strtok(NULL, "/");
    }

    // Find common prefix
    int i = 0;
    while (i < from_len && i < to_len && strcmp(from_tokens[i], to_tokens[i]) == 0) {
        i++;
    }

    // Go up for remaining from
    int up = from_len - i;
    char* result = malloc(PATH_MAX);
    result[0] = '\0';

    for (int j = 0; j < up; j++) {
        strcat(result, "../");
    }

    for (int j = i; j < to_len; j++) {
        strcat(result, to_tokens[j]);
        if (j < to_len - 1) strcat(result, "/");
    }

    return result;
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
	// printf("****HERE****\n");
	hard_link_node *curr_hard_link = find_hard_link(&inode, curr_file);
	// printf("****HERE****\n");
	hard_link_node *ptr=curr_hard_link; 

	// printf("hard link path string: %s\n", fpath);
	
	// go through each hard link for the file 

	while (ptr!=NULL) {
		

		// file_name_LL *head = ptr->head;
		// file_name_LL *curr_path = head;

		struct stat new_sb;
		if (stat(fpath, &new_sb) != -1) {
			// printf("found the hard link for the file\n");
			return ptr;
		}
		
		// while (curr_path!=NULL) {
		// 	printf("comparing to: %s\n", curr_path->name);
		// 	if (strcmp())
		// 	curr_path=curr_path->next;
		// }

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
	// printf("****HERE****\n");
	// handle, hash map head, target, length, return value 
	HASH_FIND(hh, curr_file->hard_links, inode, sizeof(ino_t), target);
	// printf("****HERE****\n"); 
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
