#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h> 
#include <openssl/evp.h> // includes definition of MD5 hashing mechanism 
#include "uthash.h"

/**
 * name of the file 
 * next file in the linked list 
 */
typedef struct {
    char *name; 
    struct file_name_LL *next;
} file_name_LL; 

/**
 * number of the soft link
 * unique inode number of the soft link 
 * linked list of the files 
 */
typedef struct {
    unsigned int number; 
    unsigned int num_files;
    ino_t inode;
    file_name_LL *head; 
    UT_hash_handle hh; 
} soft_link_node;

/**
 * inode number 
 * number of files 
 * linked list of the files 
 * access to a unique soft link hash map 
 */
typedef struct {
    ino_t inode; 
    unsigned int num_files; 
    unsigned int number_of_soft_links;
    file_name_LL *head; 
    soft_link_node *soft_links; 
    UT_hash_handle hh; 
} hard_link_node; 

/**
 * 
 */
typedef struct {
    unsigned int number; 
    unsigned char md5_hash[16];
    unsigned int md5_len;
    hard_link_node *hard_links;
    UT_hash_handle hh;
} file_node;


/**
 * This defines the callback which we will provide to the nftw function - process nftw files using this function
 * sb is the stat struct which holds the metadata to the file node 
 */
static int render_file_info(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf);

// add any other function you may need over here

void create_hard_link(char* path, file_node *curr_file);
hard_link_node *find_file_in_paths(char *fpath, file_node *curr_file, ino_t inode);
int compute_file_hash( const char* path, EVP_MD_CTX *mdctx, unsigned char *md_value, unsigned int *md5_len);
void add_file(file_node *new_file);
file_node *find_file(char *md5_hash);
void add_hard_link(hard_link_node *new_hard_link, file_node *curr_files);
hard_link_node *find_hard_link(ino_t *inode, file_node *curr_file);
void add_soft_link(soft_link_node *new_soft_link, hard_link_node *curr_hard_link);
soft_link_node *find_soft_link(ino_t *inode, hard_link_node *target_hard_link);
char* get_relative_path(const char* from, const char* to);