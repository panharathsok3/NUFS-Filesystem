#include "bitmap.h"
#include <stdio.h>
#include <stdint.h>

// This was given starter code.

/*
 * Puts the given element at the given index.
 * Takes in the bitmap, the index, and number to put.
 */
void bitmap_put(void* bm, int ii, int vv){
	int byte = ii / 8;
	int bit = ii % 8;
	if (vv == 0) {
		((uint8_t*)bm) [byte] &= ~(1 << (ii & 7));
	} else {
		((uint8_t*)bm) [byte] |= 1 << (7 - (bit));
	}
}

/*
 * Returns the element at the given index.
 * Takes in the bitmap and an index
 */
int bitmap_get(void* bm, int ii) {
	int byte = ii / 8;
	int bit = ii % 8;
	uint8_t* bmu = bm;
	return ((bmu[byte] >> 7 - bit) & 0x01);
}


