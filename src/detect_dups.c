// add any other includes in the detect_dups.h file
#include "detect_dups.h"

// define any other global variable you may need over here
#define MAX_LEN 4096

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
    
    if (nftw(argv[1], render_file_info, 20, 0)==-1) {
        fprintf(stderr, "%s is not a valid directory\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // initialize the other global variables you have, if any

    // add the nftw handler to explore the directory
    // nftw should invoke the render_file_info function
}

// render the file information invoked by nftw
static int render_file_info(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
    // perform the inode operations over here

    switch (tflag) {
		case FTW_F:
			printf("Regular File:\n");
			if ( S_ISBLK(sb->st_mode) ) {
				printf(" (Block Device)");
			} else if ( S_ISCHR(sb->st_mode) ) {
				printf(" (Character Device)");    
			}
			break;
		case FTW_D:
			printf(" (Directory) \n");
			printf("level=%02d, size=%07ld path=%s filename=%s\n",
			ftwbuf->level, sb->st_size, fpath, fpath + ftwbuf->base);
			break; 
		case FTW_SL:
			printf(" (Symbolic Link) \n");
			break;
		case FTW_NS:
			printf(" (Unreadable) \n");
			break;
		case FTW_DNR:
			printf(" (Directory cannot be read) \n");
			break;
		case FTW_SLN:
			printf(" (Symbolic link refers to non-existent file)\n");
			break;
		default:
			// if (S_ISFIFO(sb->st_mode)) {
			// 	printf(" (FIFO)");
			// }
			break;

    // invoke any function that you may need to render the file information

	// we have fpath (filename/path)
	int i;  
	unsigned char md5_value[EVP_MAX_MD_SIZE];  
	unsigned int md5_len;  
	int err;  
	EVP_MD_CTX *mdctx;  
	
	// we must create context for the MD5 algorithm 
	mdctx = EVP_MD_CTX_new();  

	// error handling, if it doesn't work for some reason 
	if (!mdctx) {  
		fprintf(stderr, "%s::%d::Error allocating MD5 context %d\n", __func__, __LINE__, errno);  
		exit(EXIT_FAILURE);  
	}  
	
	md5_len = 0;
	
	err = compute_file_hash(fpath,mdctx,md5_value,&md5_len);
	if (err < 0) {  
		fprintf(stderr, "%s::%d::Error computing MD5 hash %d\n", __func__, __LINE__, errno);  
		exit(EXIT_FAILURE);
	}  
	
	
	printf("\tMD5 Hash: ");  
	for (i = 0; i < md5_len; i++) {  
		printf("%02x", md5_value[i]);  
	}  
	// printf("\n");  
	
	// free the malloc'd space for the digest, but make sure we don't call free() again... or risk memory corruption 
	EVP_MD_CTX_free(mdctx); 
	
	return 0;  
}

// add any other functions you may need over here

/**
 * path: Path to the file to hash
 * mdctx: Digest context for MD5
 * md_value: Buffer to store the computed hash
 * md5_len: Pointer to store the length of the hash
 */
int compute_file_hash(const char *path, EVP_MD_CTX *mdctx, unsigned char *md_value, unsigned int *md5_len) {  
    
    // // open the file in read-binary mode
	FILE *fd = fopen(path, "rb");  
	
    // handle errors 
	if (fd == NULL) {  
		fprintf(stderr, "%s::%d::Error opening file %d: %s\n", __func__, __LINE__, errno, path);  
		return -1;  
	}  
	
    // create a buffer to read in the file contents
	char buff[MAX_LEN];  
    // a flag to maintain the status of the read 
	size_t n;  

    // initializing the EVP digest operation with specified md5 algorithm 
	EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
	 
	// read each byte and update the digest
	while ((n = fread(buff, 1, MAX_LEN, fd))) {  
		EVP_DigestUpdate(mdctx, buff, n);  
	}  

	EVP_DigestFinal_ex(mdctx, md_value, md5_len);  
	EVP_MD_CTX_reset(mdctx);  

	fclose(fd);  
	return 0;
}  