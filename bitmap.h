// based on cs3650 starter code

#ifndef BITMAP_H
#define BITMAP_H

/*
 * Represents a bitmap to indicate free chunks.
 */

// Returns the element at the location
int bitmap_get(void* bm, int ii);
// Sets the element at the given location
void bitmap_put(void* bm, int ii, int vv);

#endif
