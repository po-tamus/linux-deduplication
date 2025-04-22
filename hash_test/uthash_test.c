#include "../src/uthash.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int inode; 
    UT_hash_handle hh; 
} file_node; 

file_node *files = NULL; 

void add_file(file_node *new_file) {
    HASH_ADD_INT(files, inode, new_file);
}

int main() {
    file_node *test_node = (file_node*)malloc(sizeof(file_node));

    if (test_node ==NULL) {
        fprintf(stderr, "Memory not allocated\n");
        return -1;
    }

    test_node->inode = 121;
    add_file(test_node);

    file_node *thing;
    int inode_val = 121;

    HASH_FIND_INT(files, &inode_val, thing);
    if (thing!=NULL) {
        printf("the inode of thing is: %d\n", thing->inode);
    } else {
        printf("Doesn't Exist\n");
    }
    free(test_node);

} 
