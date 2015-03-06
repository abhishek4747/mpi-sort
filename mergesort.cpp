#include "mergesort.h"
#include "helper.h"

using namespace std;

inline void  merge  (dataType *datal , int sizel , dataType *datar , int sizer , dataType *data2){
    int i1 = 0, i2 = 0, i3 = 0;
       
    while (i1 < sizel && i2 < sizer) {
        if ( getkey(datal,i1) < getkey(datar,i2)) 
            data2[i3++] = datal[i1++];
        else 
            data2[i3++] = datar[i2++];
    }
	while(i1<sizel)
		data2[i3++] = datal[i1++];
	while(i2<sizel)
		data2[i3++] = datar[i2++];
}

void msort(dataType *data, int size, dataType *data2){
	if (size>1){
		int siz = size>>1;
		int siz1 = (size+1)>>1;
			msort(data2,siz,data);
			msort(data2+siz, siz1, data+siz);

		merge(data2,siz,data2+siz, siz1,data);
	}
}

void mergesort(dataType *data, int size){
	dataType* data2=(dataType*)malloc(size*sizeof(dataType));
	if (data2==0){
		cout<<"OUT OF MEMORY\n";
		exit(0);
		for (int i=0; i<size; i++){
			data2[i] = data[i];
		}

			msort(data, size, data2);	
	}
	free(data2);
}


