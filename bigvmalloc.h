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
 * bigvmsetroot("/mount/ssd/tmp");
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

#ifdef BIGVMALLOC_H
#define BIGVMALLOC_H

#define _XOPEN_SOURCE 500  /* Enable certain library functions (strdup) on linux.  See feature_test_macros(7) */

// SIZE represent the maximum number of times a  bigvmalloc call you can use on one process instance
// WARNING IF YOU CHANGE THIS YOU NEED TO CHANGE THE HASHING ALGORITHM AS WELL
// Some hashing algorithm implementation requires a minimal hashtable size to function correctly
#define SIZE 100 



// Architecture and OS specific settings
// Compatible with  x64 CPU AND LINUX
#define PROTECTION_READ  0x01
#define PROTECTION_WRITE  0x02
#define VM_VISIBLE  0x01
#define SYSCALL_ALLOC  0x09
#define SYSCALL_FREE  0x0b




#ifdef __cplusplus
extern "C" {
#endif

    
extern void *bigvmalloc (long size, char* name);
extern int bigvmfree (void *pointer, char* name);
extern void bigvmsetroot (char* name);
    



#ifdef __cplusplus
}
#endif

#endif /* BIGVMALLOC */

