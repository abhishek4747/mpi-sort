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
	while(i2<sizer)
		data2[i3++] = datar[i2++];
}

void msort(dataType *data, int size, dataType *data2){
	if (size>1){
		int siz = size>>1;
		int siz1 = size-siz;//(size+1)>>1;
		#pragma omp task if (size > 1<<20)
		{
			msort(data2,siz,data);
		}
		#pragma omp task if (size > 1<<20)
		{
			msort(data2+siz, siz1, data+siz);
		}

		#pragma omp taskwait
		merge(data2,siz,data2+siz, siz1,data);
	}
}

void mergesort(dataType *data, int size){
    // Setup Global and Environment variables
    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Status status;
    int OMPE = atoi(getenv("OMPE"))==1;
	
	// for MPI buffer
	dataType* data2=(dataType*)malloc(size*sizeof(dataType));
	if (data2==0){
		cout<<"OUT OF MEMORY\n";
		exit(0);
	}

	// backup for mergesort
	dataType* data3=(dataType*)malloc(size*sizeof(dataType));
	if (data3==0){
		cout<<"OUT OF MEMORY\n";
		exit(0);
	}
	if (world_size>1){
		int data_size = size;
		long long chunk = ceil(((double)size/world_size));
		MPI_Scatter(data,chunk,MPI_LONG_LONG,data2,chunk, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
		data_size = chunk;
		if ((world_rank==world_size-1)&& (size%world_size!=0)){
				data_size = size%data_size;
		}
		MPI_Barrier(MPI_COMM_WORLD);
		printf("I am %d and I have %d elements.\n", world_rank, data_size);
		/*
		for(int i=0;i<data_size;i++)
			cout<<getkey(data2,i)<<", ";
		cout<<endl;
		*/
		#pragma omp parallel if (OMPE)
		{
			#pragma omp for
			for (int i=0; i<data_size; i++){
				data[i] = data2[i];
			}

			#pragma omp master
			{
				msort(data, data_size, data2);	
			}	
		}
		for (int i=0; i<data_size-1;i++)
			if (getkey(data,i)>getkey(data,i+1))
				cout<<"WRONG WRONG WRONG "<<world_rank<<"\n\n\n"<<endl;
		MPI_Barrier(MPI_COMM_WORLD);
		int done = 0;
		for (int step=1; step< world_size;step*=2){
			if (!done){
				if (world_rank%(2*step)==0){
					if (world_rank + step < world_size){
						int m;
						MPI_Recv(&m,1,MPI_INT, world_rank+step,0,MPI_COMM_WORLD,&status);
					
						MPI_Recv(data3,m,MPI_LONG_LONG,world_rank+step,0,MPI_COMM_WORLD, &status);

						//for(int i=0; i<data_size;i++)
						//	cout<<"data3 "<<getkey(data3,i)<<" "<<world_rank<<"\t";
						//cout<<endl;
						#pragma omp parallel if (OMPE)
						{
							#pragma omp for
							for(int i=0; i<data_size; i++){
								data2[i] = data[i];
							}
						}
						printf("Step: %d I am %d and I am recieved %d elements from %d. I have %d elements: something\n",step, world_rank, m, world_rank+step,data_size);		
						
						merge(data3,m, data2,data_size, data);

						data_size += m;

					}
				}else{
					int dest = world_rank - step;
					MPI_Send(&data_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
					MPI_Send(data,data_size,MPI_LONG_LONG,dest,0,MPI_COMM_WORLD);
					//for(int i=0; i<data_size;i++)
					//	cout<<"sent "<<getkey(data,i)<<" "<<world_rank<<"\t";
					//cout<<endl;
					done = 1;
					//printf("Step: %d I am %d and I am sent %d elements to %d\n",step, world_rank, data_size, dest);
					data_size = 0;
				}	
			}
			MPI_Barrier(MPI_COMM_WORLD);
		}
	}
	else{
		#pragma omp parallel if (OMPE)   
		{
			#pragma omp for 
			for (int i=0; i<size; i++){
				data2[i] = data[i];
			}

			#pragma omp master
			{
				msort(data, size, data2);	
			}
		}
	}
	free(data2);
	free(data3);
}


