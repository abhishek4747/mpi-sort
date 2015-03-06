#include "quicksort.h"
#include "helper.h"


using namespace std;

int splitq ( dataType *a, int upper ){
	int  p, q;
	p =  0 ;
	q = upper - 1 ;

	while ( q >= p ){
		while (q>=p &&  getkey(a,p) <=  getkey(a,0) )
			p++ ;

		while (q>=p &&  getkey(a,q) >  getkey(a,0) )
			q-- ;

		if ( q > p )
			swap(a,p,q);
	}
	swap(a,0,q);
	return q ;
}

void qsort(dataType *data, int start, int end){
	if (start+1>=end){
		return;
	} else if (start+2==end){
		if ( getkey(data,start)>  getkey(data,start+1)){
			swap(data, start, start+1);
		}
	}
	else if (start+1<end){
		int size = end - start;
		int store = start + splitq(data+start, size);
		
		qsort(data, start, store);
		qsort(data, store+1, end);
	}
}

void quicksort(dataType *data, int size){
	int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Status status;

	dataType* data2 =(dataType*)malloc(size*sizeof(dataType));
	if (data2==0){
		cout<<"OUT OF MEMORY\n";
		exit(0);
	}
	if (world_rank==0)
		for (int i=0; i<size; i++){
			data2[i] = data[i];
		}

	int pivot;
	int data_size = size;
	for (int s=world_size; s> 1; s=s>>1){
		if (world_rank % s ==0){
			pivot = splitq(data2, data_size);
			printf("%d is sending %d to %d.\n",world_rank,data_size-pivot,world_rank+ s/2);
			MPI_Send(data2+pivot, data_size - pivot, MPI_LONG_LONG, world_rank + s/2, 0, MPI_COMM_WORLD);

			data_size = pivot;
		}else if (world_rank % s == s>>1){
			MPI_Recv(data2, size, MPI_LONG_LONG, world_rank- s/2, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_LONG_LONG, &data_size);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	int *gather_arr;
	int *disp_arr;
	if (world_rank==0){
		disp_arr = (int*) malloc((world_size+1)*sizeof(int));
		disp_arr[0] = 0;
		gather_arr = (int*) malloc((world_size)*sizeof(int));;
	}
	MPI_Gather(&data_size, 1, MPI_INT, gather_arr, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	if (world_rank==0){
		for (int i =0; i< world_size; i++){
			printf("I am no. %d of %d and I have %d elements.\n",i, world_size,gather_arr[i]);
			disp_arr[i+1] = disp_arr[i]+gather_arr[i];
		}
	}
	/*
	MPI_Barrier(MPI_COMM_WORLD);
	for (int i=0; i< data_size; i++){
		cout<<world_rank<<","<<getkey(data2,i)<<"\t";
	}
	cout<<world_rank<<","<<endl;
	*/
	qsort(data2, 0, data_size);
	/*
	MPI_Barrier(MPI_COMM_WORLD);
	printf("I am no. %d of %d and I have sorted %d elements.\n",world_rank, world_size, data_size);
	MPI_Barrier(MPI_COMM_WORLD);
	for (int i=0; i< data_size; i++){
		cout<<world_rank<<","<<getkey(data2,i)<<"\t";
	}
	cout<<world_rank<<","<<endl;
	MPI_Barrier(MPI_COMM_WORLD);
	*/
	MPI_Gatherv(data2, data_size, MPI_LONG_LONG, data, gather_arr,disp_arr, MPI_LONG,0,MPI_COMM_WORLD);
	if (world_rank==0){
		free(gather_arr);
		free(disp_arr);
	}
		
	free(data2);
}
