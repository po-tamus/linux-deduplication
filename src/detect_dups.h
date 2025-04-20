#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h> 
#include <openssl/evp.h> // includes definition of MD5 hashing mechanism 
#include "uthash.h"

// define the structure required to store the file paths

typedef struct {
    int inode;
    UT_hash_handle hh;
} file;


/**
 * This defines the callback which we will provide to the nftw function - process nftw files using this function
 * sb is the stat struct which holds the metadata to the file node 
 */
static int render_file_info(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf);

// add any other function you may need over here