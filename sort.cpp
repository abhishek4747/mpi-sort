#include "sort.h"
#include "quicksort.h"					// For quicksort
#include "pquick.h"
#include "mergesort.h"					// For mergesort
#include "radixsort.h"					// For radixsort

using namespace std;

void pSort(dataType *data, int size, SortType sorter){
	switch (sorter){
		case QUICK:
			quicksort(data,size);
			break;
		case MERGE:		
			mergesort(data,size);
			break;
		case RADIX:
			radixsort(data,size);
			break;
		default:
			pquicksort(data,size);
	}
}

