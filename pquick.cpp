#include "pquick.h"
#include "helper.h"
#include "quicksort.h"

using namespace std;

int procs, osize;

// must be called from one thread only
void psum(int *data, int size, int *data2){
	if (size<2)
		return;

	// #1: Pairwise sum
	int s = size>>1;
	#pragma omp parallel for
	for (int i=0; i< s; i++){
		data2[i] = data[2*i] + data[2*i+1];
	}
	// #2: Recurse
	psum(data2, s, data2+s);

	// #3: Finalize
	#pragma omp parallel for
	for (int i=0; i<s; i++){
		data[2*i] = data2[i] -data[2*i+1];
		data[2*i + 1] = data2[i];
	}

	if (size&1){
		data[size-1] = data[size-1] + data[size -2];
	}
}



int splitp ( dataType *a, int upper , long long pivot){
	int  p, q;
	p =  0 ;
	q = upper - 1 ;

	while ( q >= p ){
		while (q>=p && getkey(a,p)<= pivot )
			p++ ;

		while (q>=p && getkey(a,q)> pivot )
			q-- ;

		if ( q > p )
			swap(a,p,q);
	}
	return q ;
}

	
int partition(dataType *data, int size, dataType *data2){
	if (size<2)
		return 0;
	else if (/*size <osize/procs ||*/ size < 1<<20){
		return splitq(data, size);
	} else{
		int p = procs;
		int *lessp;
		lessp = (int*) malloc(p*sizeof(int));
		if( lessp == 0 ){
			cout<<"OUT OF MEMORY."<<endl;
			exit(0);
		}
		int *lessp2;
		lessp2 = (int*) malloc(p*sizeof(int));
		if( lessp2 == 0 ){
			cout<<"OUT OF MEMORY."<<endl;
			exit(0);
		}
		int chunk = ceil(((double)size)/p);
		p = ceil(((double)size)/chunk);

		#pragma omp parallel num_threads(p) 
		{
			int i = omp_get_thread_num();
			int start = chunk*i;
			int end = min((int)chunk*(i+1),size);

			lessp[i] = splitp(data+start, end-start, getkey(data,0))+1;	
		}
		
		psum(lessp,p,lessp2);
		free(lessp2);

		#pragma omp parallel num_threads(p)
		{
			int i = omp_get_thread_num();
			int start = chunk*i;
			int end = min((int)chunk*(i+1),size);
			int lp = (i>0? lessp[i-1]:0);
			int dp = lessp[i];
			
			for (int j = 0; j<dp-lp; j++){
				data2[lp+j] = data[start+j];
			}
			int gp = (i>0?( start - lp):0);
			int gd = end - dp;
			for (int j =0; j <gd-gp; j++){
				data2[lessp[p-1]+gp+j] = data[start+dp-lp+j];
			}
		}
		
		swap(data2,0,lessp[p-1]-1);	
		
		#pragma omp parallel for num_threads(p)
		for (int i=0; i<size; i++){
			data[i] = data2[i];
		}
		return (lessp[p-1]>0?lessp[p-1]-1:0);
	}
}

void pqsort(dataType *data, int start, int end, dataType *data2){
	if (start+1>=end){
		return;
	}else if (start+2==end){
		if ( getkey(data,start)>  getkey(data,start+1)){
			swap(data, start, start+1);
		}
	}else if (start+1<end){
		int size = end - start;
		int store = start + partition(data+start, size, data2+start);
		//int store = start + splitq(data+start, size);
		#pragma omp task if (size>1<<20)
		{
			pqsort(data, start, store, data2);
		}
		#pragma omp task if (size>1<<20)
		{
			pqsort(data, store+1, end, data2);
		}
	}
}

void pquicksort(dataType *data, int size){
	// Setup Global and Environment variables
	int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Status status;
	int OMPE = atoi(getenv("OMPE"))==1;

	// Allocate space on every node for sorting
	dataType* data2 =(dataType*)malloc(size*sizeof(dataType));
	if (data2==0){
		cout<<"OUT OF MEMORY\n";
		exit(0);
	}
	if (world_rank==0){
		for (int i=0; i<size; i++){
			data2[i] = data[i];
		}
	}
	
	osize = size;
	procs = omp_get_num_procs();
	dataType *data3;
	data3 = (dataType*) malloc(size*sizeof(dataType));
	if (data3==0){
		cout<<"OUT OF MEMORY"<<endl;
		exit(0);
	}
	omp_set_nested(1);
	
	// Split and distribute data across nodes
	int pivot;
	int data_size = size;
	double begin, end, time_spent;

	MPI_Barrier(MPI_COMM_WORLD);
	if (world_rank==0)
		begin  = MPI_Wtime();
	int lp = 0;
	for (int s=world_size; s> 1; ){
		if (world_rank ==lp){
			#pragma omp parallel if (OMPE)
			{
				#pragma omp single nowait
				{
					if (OMPE)
						pivot = partition(data2, data_size, data3) + 1;
					else
						pivot = splitq(data2, data_size) + 1;
				}
			}
			printf("%d is sending %d to %d.\n",world_rank,data_size-pivot,world_rank+ s/2);
			MPI_Send(data2+pivot, data_size - pivot, MPI_LONG_LONG, world_rank + s/2, 0, MPI_COMM_WORLD);
			data_size = pivot;
		}else if (world_rank  ==lp +  (s>>1)){
			MPI_Recv(data2, size, MPI_LONG_LONG, lp, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_LONG_LONG, &data_size);
		}
		
		int d = s>>1;
		if (world_rank>=d + lp){
			s = s - d;
			lp = d + lp;
		}else{
			s = d;
		}
	}

	int *gather_arr;
	int *disp_arr;
	if (world_rank==0){
		disp_arr = (int*) malloc((world_size+1)*sizeof(int));
		disp_arr[0] = 0;
		gather_arr = (int*) malloc((world_size)*sizeof(int));;
	}
	MPI_Gather(&data_size, 1, MPI_INT, gather_arr, 1, MPI_INT, 0, MPI_COMM_WORLD);
	if (world_rank==0){
		for (int i =0; i< world_size; i++){
			printf("I am no. %d of %d and I have %d elements.\n",i, world_size,gather_arr[i]);
			disp_arr[i+1] = disp_arr[i]+gather_arr[i];
		}
		end = MPI_Wtime();
		time_spent = (double)(end - begin);
		cout<<"Time: "<<time_spent<<" seconds to distribute the array with "<<size<<" numbers in "<<world_size<<" nodes."<<endl;
	}
	
	#pragma omp parallel if (OMPE)
	{
		#pragma omp single nowait
		{
			pqsort(data2, 0, data_size, data3);
		}
	}
	free(data3);
	MPI_Barrier(MPI_COMM_WORLD);
	if (world_rank==0)
		begin = MPI_Wtime();	
	MPI_Gatherv(data2, data_size, MPI_LONG_LONG, data, gather_arr,disp_arr, MPI_LONG,0,MPI_COMM_WORLD);
	if (world_rank==0){
		end = MPI_Wtime();
		time_spent = (double)(end - begin);
		cout<<"Time: "<<time_spent<<" seconds to gather the array with "<<size<<" numbers from "<<world_size<<" nodes."<<endl;
		free(gather_arr);
		free(disp_arr);
	}
		
	free(data2);
}

