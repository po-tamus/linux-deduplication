#include "../src/detect_dups.h"
  
#define MAX_LEN 4096  

// function to compute a file hash
int compute_file_hash(const char *path, EVP_MD_CTX *mdctx, unsigned char *md_value, unsigned int *md5_len) {  
	FILE *fd = fopen(path, "rb");  
	
	if (fd == NULL) {  
		fprintf(stderr, "%s::%d::Error opening file %d: %s\n", __func__, __LINE__, errno, path);  
		return -1;  
	}  
	
	char buff[MAX_LEN];  
	size_t n;  
	EVP_DigestInit_ex(mdctx, EVP_md5(), NULL); 
	 
	while ((n = fread(buff, 1, MAX_LEN, fd))) {  
		EVP_DigestUpdate(mdctx, buff, n);  
	}  
	EVP_DigestFinal_ex(mdctx, md_value, md5_len);  
	EVP_MD_CTX_reset(mdctx);  
	fclose(fd);  
	return 0;  
}  
  
int main(int argc, char *argv[]) {  
	int i;  
	const char *filename ;  
	unsigned char md5_value[EVP_MAX_MD_SIZE];  
	unsigned int md5_len;  
	int err;  
	EVP_MD_CTX *mdctx;  
	
	if (argc != 2) {  
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);  
	return 1;  
	}  
	filename = argv[1];  
	// create md5 structure  
	mdctx = EVP_MD_CTX_new();  
	if (!mdctx) {  
		fprintf(stderr, "%s::%d::Error allocating MD5 context %d\n", __func__, __LINE__, errno);  
		exit(EXIT_FAILURE);  
	}  
	
	md5_len = 0 ;
	
	err = compute_file_hash(filename,mdctx,md5_value,&md5_len);  
	if (err < 0) {  
		fprintf(stderr, "%s::%d::Error computing MD5 hash %d\n", __func__, __LINE__, errno);  
		exit(EXIT_FAILURE);
	}  
	
	
	printf("\tMD5 Hash: ");  
	for ( i = 0; i < md5_len; i++) {  
		printf("%02x", md5_value[i]);  
	}  
	printf("\n");  
	
	EVP_MD_CTX_free(mdctx); // don't create a leak!  
	

	add_file(NULL);

	return 0;  
}

void add_file(file_node *new_file) {
	char s1[32] = "hello, what's up?";
	char s2[32] = "hello, whats up?"; 
	// int strncmp(const char *s1, const char *s2, size_t n);
	int result = strncmp(s1, s2, 6);
	printf("HERE: %d\n", result);
}