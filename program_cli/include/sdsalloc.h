/* SDSLib 2.0 -- A C dynamic strings library
 *
 */

/* SDS allocator selection.
 *
 * This file is used in order to change the SDS allocator at compile time.
 * Just define the following defines to what you want to use. Also add
 * the include of your alternate allocator if needed (not needed in order
 * to use the default libc allocator). */

#define s_malloc malloc
#define s_realloc realloc
#define s_free free
