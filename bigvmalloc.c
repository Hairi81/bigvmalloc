/* bigvmalloc Library by Hairi Abass, September 2019
 * hairi.abass@gmail.com
 * This function allocates Virtual Memory from file system
 * Useful when you are short on RAM
 *  * 
 * Arrays can be accessed normally or using pointer arithmetic
 * Don't forget to call bigvmfree to delete the files and clear page tables
 *
 * ==========================================================================
 * bigvmalloc(long size, char *uniqueid)
 * ==========================================================================
 * Default Usage :
 * Creates a 1 GB Virtual Memory mapped to a file in the current running folder named
 * somename_processpid.mem
 * 
 * 
 * char *mychar = bigvmalloc(1024*1024*1024*sizeof(char), "somename");
 *
 * 
 * 
 * Path Usage :
 * Creates a 1 GB Virtual Memory mapped to a file to the /tmp folder named
 * somename_processpid.mem
 * 
 * 
 * char *mychar = bigvmalloc(1024*1024*1024*sizeof(char), "/tmp/somename");
 *
 * return code of -1 means failure allocation memory
 * 
 * ===========================================================================
 * bigvmfree(void *pointer, char *uniqueidentifider")
 * ===========================================================================
 * Usage :
 * Free 1 GB Virtual Memory from pointer mychar mapped to file /tmp/somename_processpid.mem
 * 
 * bigvmfree(mychar,  "/tmp/somename");
 *
 *
 * ===========================================================================
 * bigvmsetroot(char* folder)
 * ===========================================================================
 * Usage :
 * Set the default prefix root folder for all bigvmalloc allocations
 * If you want to set path, call this function before calling bigvmalloc
 * 
 * bigvmsetroot("/mount/ssd/tmp")
 * 
 * 
 * ============================================================================
 * 
 * 
 *
 *
 *  * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include "bigvmalloc.h"
#include <string.h>



char *defaultfolder = "placeholder";

struct entry_s {
    char *key;
    char *value;
    int fd;
    long vmsize;


    struct entry_s *next;
};

typedef struct entry_s entry_t;

//C99 compability, strdup replacement
char *dupstr(const char *s) {
    char *const result = malloc(strlen(s) + 1);
    if (result != NULL) {
        strcpy(result, s);
    }
    return result;
}

struct hashtable_s {
    int size;
    struct entry_s **table;
};

typedef struct hashtable_s hashtable_t;

hashtable_t *hashtable;

/* Create a new hashtable. */
hashtable_t *ht_create(int size) {

    hashtable_t *hashtable = NULL;
    int i;

    if (size < 1) return NULL;

    /* Allocate the table itself. */
    if ((hashtable = malloc(sizeof ( hashtable_t))) == NULL) {
        return NULL;
    }

    /* Allocate pointers to the head nodes. */
    if ((hashtable->table = malloc(sizeof ( entry_t *) * size)) == NULL) {

        free(hashtable);
        return NULL;
    }
    for (i = 0; i < size; i++) {
        hashtable->table[i] = NULL;
    }

    hashtable->size = size;

    return hashtable;
}


/* Hash a string for a particular hash table. */
// A slightly better hash function.

int ht_hash(hashtable_t *hashtable, char *key) {


    int ht_size = hashtable->size;

    // unsigned int hash = 0, g;
    unsigned int hash = 0;

    register char *p;

    for (p = key; *p; p++) {
        hash = (hash << 4) + *p;
        /* assumes a 32 bit int size */
        unsigned int g;

        if ((g = hash & 0xf0000000)) {
            hash ^= g >> 24;
            hash ^= g;
        }
    }


    return hash % ht_size;
}

/* Create a key-value pair. */
entry_t *ht_newpair(char *key, char *value, int fd, long vmsize) {
    entry_t *newpair;

    if ((newpair = malloc(sizeof ( entry_t))) == NULL) {

        free(newpair);
        return NULL;
    }

    if ((newpair->key = dupstr(key)) == NULL) {

        free(newpair);

        return NULL;
    }

    if ((newpair->value = dupstr(value)) == NULL) {

        free(newpair);

        return NULL;
    }

    newpair->fd = fd;
    newpair->vmsize = vmsize;



    newpair->next = NULL;

    return newpair;
}

/* Insert a key-value pair into a hash table. */
void ht_set(hashtable_t *hashtable, char *key, char *value, int fd, long vmsize) {
    int bin = 0;
    entry_t *newpair = NULL;
    entry_t *last = NULL;
    entry_t *next;


    bin = ht_hash(hashtable, key);



    next = hashtable->table[ bin ];

    while (next != NULL && next->key != NULL && strcmp(key, next->key) > 0) {
        last = next;
        next = next->next;
    }

    /* There's already a pair.  Let's replace that string. */
    if (next != NULL && next->key != NULL && strcmp(key, next->key) == 0) {

        free(next->value);
        next->value = dupstr(value);
        next->fd = fd;
        next->vmsize = vmsize;


    } else {
        newpair = ht_newpair(key, value, fd, vmsize);

        if (next == hashtable->table[ bin ]) {
            newpair->next = next;
            hashtable->table[ bin ] = newpair;

        } else if (next == NULL) {

            if (last != NULL)
                last->next = newpair;

        } else {
            newpair->next = next;
            last->next = newpair;
        }
    }
}

/* Retrieve a key-value pair from a hash table. */
char *ht_get(hashtable_t *hashtable, char *key) {
    int bin = 0;
    entry_t *pair;

    bin = ht_hash(hashtable, key);

    pair = hashtable->table[ bin ];
    while (pair != NULL && pair->key != NULL && strcmp(key, pair->key) > 0) {
        pair = pair->next;
    }

    if (pair == NULL || pair->key == NULL || strcmp(key, pair->key) != 0) {
        return NULL;

    } else {
        return pair->value;
    }

}

int ht_get_fd(hashtable_t *hashtable, char *key) {
    int bin = 0;
    entry_t *pair;

    bin = ht_hash(hashtable, key);

    pair = hashtable->table[ bin ];
    while (pair != NULL && pair->key != NULL && strcmp(key, pair->key) > 0) {
        pair = pair->next;
    }

    /* Did we actually find anything? */
    if (pair == NULL || pair->key == NULL || strcmp(key, pair->key) != 0) {
        return -1;

    } else {
        return pair->fd;
    }

}

long ht_get_vmsize(hashtable_t *hashtable, char *key) {
    int bin = 0;
    entry_t *pair;

    bin = ht_hash(hashtable, key);

    pair = hashtable->table[ bin ];
    while (pair != NULL && pair->key != NULL && strcmp(key, pair->key) > 0) {
        pair = pair->next;
    }

    if (pair == NULL || pair->key == NULL || strcmp(key, pair->key) != 0) {
        return (long) NULL;

    } else {
        return pair->vmsize;
    }

}

entry_t *ht_get_entry(hashtable_t *hashtable, char *key) {
    int bin = 0;
    entry_t *pair;

    bin = ht_hash(hashtable, key);

    pair = hashtable->table[ bin ];
    while (pair != NULL && pair->key != NULL && strcmp(key, pair->key) > 0) {
        pair = pair->next;
    }

    if (pair == NULL || pair->key == NULL || strcmp(key, pair->key) != 0) {
        return NULL;

    } else {
        return (entry_t*) pair;
    }

}

void ht_del(hashtable_t *hashtable, char *key) {

    int bin = 0;
    int flag = 0;

    entry_t *temp, *pair;

    bin = ht_hash(hashtable, key);

    pair = hashtable->table[bin]; // Go to index node

    temp = pair;
    while (pair != NULL) {
        if (strcmp(key, pair->key) == 0) {
            flag = 1;
            if (pair == hashtable->table[bin]) {
                hashtable->table[bin] = pair->next;
            } else {
                temp->next = pair->next;
            }
            free(pair);
            break;
        }
        temp = pair;
        pair = pair->next;
    }
    if (flag) {
        //      printf("Data deleted successfully from Hash Table\n");
    } else {
        //    printf("Given data is not present in hash table!!!!\n");
    }
}

//Allocate the virtual memory from file descriptors using system calls
void *allocate_asm(void *addr, size_t vmsize, int prot, int flags, int fd, off_t vmoffset) {
    
    //September, 2019
    //Serious stuff, don't change unless you know what you are doing.

    //START FLUFF code, just in case we need to do more register operations in the future for further optimization.
    register int r13 __asm__("r13") = vmoffset;
    register int r14 __asm__("r14") = 0x0fffffff;
    register int r15 __asm__("r15") = 0;



    //dword sized register operations
    //eax is clobbered 

    __asm__ __volatile__(
            "   movl    %1,%%eax;"
            "   andl    %2,%%eax;"
            "   movl    %%eax,%0;"
            : "=r" (r15)
            : "r" (r13), "r" (14)
            : "%eax"
            );
    //END FLUFF CODE


    //Preparing the stack and registers
    register int64_t rax __asm__("rax") = SYSCALL_ALLOC;
    register void *rdi __asm__("rdi") = addr;
    register size_t rsi __asm__("rsi") = vmsize;
    register int rdx __asm__("rdx") = prot;
    register int r10 __asm__("r10") = flags;
    register int r9 __asm__("r9") = r15;
    //Remember 64 bit Register R8 stores the file descriptor.
    register int r8 __asm__("r8") = fd;

    //Ensure the compiler doesn't move this around
    __asm__ __volatile__(
            "syscall"
            : "+r" (rax)
            : "r" (rdi), "r" (rsi), "r" (rdx), "r" (r10), "r" (r8), "r" (r9)
            : "cc", "rcx", "r11", "memory"
            );

    
    //Voila,  pointer to a newly allocated Virtual Memory block ready to use
    return (void *) rax;
}

//Free the Allocated Virtual Memory
int free_asm(void *args, size_t vmsize) {
    
    //September, 2019
    //Serious stuff, don't change unless you know what you are doing.


     //Preparing the stack and registers
    register int64_t rax __asm__("rax") = SYSCALL_FREE;
    register void *rdi __asm__("rdi") = args;
    register size_t rsi __asm__("rsi") = vmsize;


    //Ensure the compiler doesn't move this around
    __asm__ __volatile__(
            "syscall"
            : "+r" (rax)
            : "r" (rdi), "r" (rsi)
            : "cc", "rcx", "r11", "memory"
            );
    
    return rax;

}

//Our main library function
void *bigvmalloc(long size, char* name) {


    if (hashtable == NULL) {

        hashtable = ht_create(SIZE);

    }

    long fd;
    int result;
    int pid = getpid();
    char pidstr[20];

    snprintf(pidstr, 20, "_%d.mem", pid);
    char FILEPATH[100];

    if (defaultfolder > 0) {
        if ((strcmp(defaultfolder, "placeholder") != 0) && strlen(defaultfolder) > 0) {
            snprintf(FILEPATH, 100, "%s/%s_%s", defaultfolder, name, pidstr);

        } else
            snprintf(FILEPATH, 100, "%s_%s", name, pidstr);

    } else
        snprintf(FILEPATH, 100, "%s_%s", name, pidstr);


    long FILESIZE = size;

    fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0600);

    if (fd == -1) {
        perror("Error opening file for writing");
        return (void *) - 1;
    }

    result = lseek(fd, FILESIZE - 1, SEEK_SET);

    if (result == -1) {
        close(fd);
        perror("Error Calling lseek to stretch file");
        return (void *) - 1;
    }

    result = write(fd, "", 1);

    if (result != 1) {
        close(fd);
        perror("Error Writing last byte ");
        return (void *) - 1;
    }


    ht_set(hashtable, name, FILEPATH, fd, size);

    size_t vmsize = FILESIZE;



    return allocate_asm(0, vmsize, PROTECTION_READ | PROTECTION_WRITE, VM_VISIBLE, fd, 0);

}

//Free allocated Virtual Memory, also close and delete the files
int bigvmfree(void *args, char *name) {



    long FILESIZE = 0;
    int ret = 0;
    int fd = 0;
    char *s;


    entry_t *myentry = ht_get_entry(hashtable, name);

    if (myentry == NULL) {
        perror("Error Locating VM Page");
        return -1;
    }

    FILESIZE = myentry->vmsize;
    fd = myentry->fd;
    s = myentry->value;

    ret = free_asm(args, FILESIZE);

    if (ret == -1) {
        perror("Error Freeing VM Page");
        return -1;
    }

    ret = close(fd);
    if (ret == -1) {
        perror("Error Closing File");
        return -1;
    }



    ret = remove(s);
    ht_del(hashtable, name);

    return ret;


}

//Use this to set root path of mem file created
void bigvmsetroot(char *df) {

    defaultfolder = df;

}




