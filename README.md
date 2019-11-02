# bigvmalloc
C BIGVMALLOC library, Allocate Virtual Memory from disk space. Convenience wrapper of MMAP
This is usefull when you are feeling a bit lazy or just inherited lots of code with
malloc calls that doesn't FIT IN RAM.


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
 * char *mychar = (char *) bigvmalloc(1024*1024*1024*sizeof(char), "somename");
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
 */
