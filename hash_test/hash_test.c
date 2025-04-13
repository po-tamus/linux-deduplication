#include "../src/uthash.h"

// char *key; 

// typedef struct {
//     char hash_key[16]; //key
//     UT_hash_handle hh; //this is the hash handle 
// } record_t;

// record_t *records = NULL;

// record_t *data_w = malloc(sizeof(record_t)); 
// data_w->key = "qwegjioa9hhsuhs";

// //HASH_ADD(handle, )
// HASH_ADD(hh, records, data->key, 16, data_w); 
/*********************/

/*

 * Example usage with int as key

*/

struct my_files {

    int inode; /* we'll use this field as the key */

    /* ... other inode related data ... */

    UT_hash_handle hh; /* makes this structure hashable */

};




struct my_files *files = NULL; // global variable to access the data from anywhere

void add_file(struct my_files *write) {

    HASH_ADD_INT(files, id, write);

}



struct my_files *find_file(int inode) {

    struct my_files *read;

    HASH_FIND_INT(files, &inode, read);

    return f;

}




/*

    * Example usage with char as key

    */

typedef struct {

    char hash_key[16]; // 16 bytes long

    /* ... other file related data ... */

    UT_hash_handle hh; /* makes this structure hashable */

} record_t;




struct record_t *records = NULL; // global variable to access the data from anywhere

void add_record(struct record_t *data_w) {

    /* HASH_ADD(handle, hash_map, key, key_len, write_data_ptr); */

    /* handle -> hh (defined in your struct), key -> key to be hashed, key_len = 16 since md5 is only 16 bytes long */

    HASH_ADD(hh, records, data->hash_key, 16, data_w); // writes data_w into the hashmap

}

    
    
struct record_t *find_record(char *skey) {

    struct record_t *data_r; // read data already in memory, only obtaining the pointer reference

    /* HASH_FIND(handle, hash_map, key, key_len, read_data_ptr);*/

    /* handle -> hh (defined in your struct), key -> the key to find, key_len = 16 since md5 is only 16 bytes long */

    HASH_ADD(hh, records, skey, 16, data_r); // writes data into data_r

    return data_r;
    
}

struct my_struct {
    int id;            /* we'll use this field as the key */
    char name[10];
    UT_hash_handle hh; /* makes this structure hashable */
};

struct my_struct *users = NULL;

void add_user(struct my_struct *s) {
    HASH_ADD_INT( users, id, s );
}

struct my_struct *find_user(int user_id) {
    struct my_struct *s;

    HASH_FIND_INT( users, &user_id, s );
    return s;
}

void delete_user(struct my_struct *user) {
    HASH_DEL( users, user);
}
