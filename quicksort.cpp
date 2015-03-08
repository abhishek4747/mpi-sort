#include "quicksort.h"
#include "helper.h"


using namespace std;

int splitq ( dataType *a, int upper ){
	if (upper==0)
		return  0;
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
		#pragma omp task if (size > 1 <<20)
		{
			qsort(data, start, store);
		}
		#pragma omp task if (size > 1 <<20)
		{
			qsort(data, store+1, end);
		}		
	}
}

void quicksort(dataType *data, int size){
	// Setup Global and Environment variables
	int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Status status;
	int OMPE = atoi(getenv("OMPE"))==1;

	if (world_size>1){
		// Allocate space on every node for sorting
		dataType* data2 =(dataType*)malloc(size*sizeof(dataType));
		if (data2==0){
			cout<<"OUT OF MEMORY\n";
			exit(0);
		}
		if (world_rank==0){
			#pragma omp parallel if(OMPE)
			{
				#pragma omp for
				for (int i=0; i<size; i++){
					data2[i] = data[i];
				}
			}
		}

		// Split and distribute data across nodes
		int pivot;
		int data_size = size;
		int *gather_arr;
		int *disp_arr;
		double begin, end, time_spent;

		MPI_Barrier(MPI_COMM_WORLD);
		if (world_rank==0)
			begin  = MPI_Wtime();
		int lp = 0;
		for (int s=world_size; s> 1; ){
			if (world_rank ==lp){
				pivot = splitq(data2, data_size) + 1;
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
				qsort(data2, 0, data_size);
			}
		}

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
	else{
		#pragma omp parallel if (OMPE)
		{
			#pragma omp single nowait
			{
				qsort(data, 0, size);
			}
		}
	}
}
