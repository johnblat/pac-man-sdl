#ifndef ARR_UTIL_H
#define ARR_UTIL_H


#define acc2DArray(arr, row, col, numCols ) ( arr[ row*numCols + col ] )

#define set2DArrayElm(arr, row, col, numCols, val ) ( arr[ row*numCols + col ] = val ) 

#endif