#include <bitset>
#include <set>
#include <stdio.h>
#include "sort.h"
#include "helper.h"

using namespace std;

SortType sorttype = QUICK;
int NUM_OF_ELEMENTS = 1<<20;
int MIN_NUM = 1<<20;


void arrayPrint(dataType *data, int n = NUM_OF_ELEMENTS){
    for (int i = 0; i < n; ++i){
        if (sorttype!=RADIX)
            cout<<getkey(data,i)<<"\t";
        else
            cout<</*(bitset<64>)*/ getkey(data, i)<<"\n";

    }
    cout<<"\n"<<endl;
}

int isSorted(dataType* data, int n = NUM_OF_ELEMENTS){
    for (int i = 1; i < n; ++i){
        if (getkey(data,i-1) > getkey(data,i)){
            return 0;
        }
    }
    return 1;
}



int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    //printf("Hello world from processor %s, rank %d"
    //       " out of %d processors\n",
    //       processor_name, world_rank, world_size);

	if (argc>1){
		NUM_OF_ELEMENTS = 1 << atoi(argv[1]);
	}
	if (argc>2){
		switch (argv[2][0]){
            case 'q':
                sorttype = QUICK;
                if (world_rank==0) cout<<"\n\t\tQUICKSORT"<<endl;
                break;
            case 'm':
                sorttype = MERGE;
                if (world_rank==0) cout<<"\n\t\tMERGESORT"<<endl;
                break;
            case 'r':
                sorttype = RADIX;
                if (world_rank==0) cout<<"\n\t\tRADIXSORT"<<endl;
                break;
            default:
                sorttype = BEST;
                if (world_rank==0) cout<<"\n\t\tBESTSORT"<<endl;
        }   	
	}

	double begin, end;
	double time_spent;

	/* Create a random array and distribute it */
	dataType *data;
	if (world_rank==0)
		data=(dataType*)malloc(NUM_OF_ELEMENTS*sizeof(dataType));
		if (data==0){
			cout<<"OUT OF MEMORY.\n";
			exit(0);
		}
	set <long long> s1;
	if (world_rank==0){
		srand(time(NULL));
		/* fill array with random numbers */
		/* -------------------------- */
		srand(rand());srand(rand());
		begin = MPI_Wtime();
		unsigned seed;
		seed = rand()*world_size;
		for (int i = 0; i < NUM_OF_ELEMENTS; ++i){
			int r = rand_r(&seed);
			data[i].key = (long long *)(((((long long)(rand_r(&seed))<<31)|(long long)r)<<2)|(long long)(r>>29));
			 //data[i].key = (long long *)(((long long)(rand_r(&seed))<<31)|r);
			// data[i].key = (long long *)((long long)r>>20);
		}
		if (NUM_OF_ELEMENTS < MIN_NUM){
			for (int i = 0; i < NUM_OF_ELEMENTS; ++i){
				s1.insert(getkey(data,i));
			}
		}
		end = MPI_Wtime();
		time_spent = (double)(end - begin);
		cout<<"Time: "<<time_spent<<" seconds to fill the array with "<<NUM_OF_ELEMENTS<<" numbers."<<endl;
		if (NUM_OF_ELEMENTS< 1<<4)
        arrayPrint(data);

	}



	/* sort array */
    /* -------------------------- */
    begin = MPI_Wtime();
        pSort(data,NUM_OF_ELEMENTS,sorttype);
    end = MPI_Wtime();
    time_spent = (double)(end - begin);
	if (world_rank==0){
		cout<<endl;
		cout<<"Time: "<<time_spent<<" seconds to sort the array of "<<NUM_OF_ELEMENTS<<" numbers."<<endl;
	}
    /* -------------------------- */



	/* check array */
    /* -------------------------- */
	if (world_rank==0){
		begin = MPI_Wtime();
		set <long long> s2;
		if (NUM_OF_ELEMENTS < MIN_NUM){
			for (int i = 0; i < NUM_OF_ELEMENTS; ++i){
				s2.insert(getkey(data,i));
			}
		}
		
		if (!isSorted(data)){
			cout<<"ARRAY NOT SORTED!!\n";
		}
		if (s1.size()!=s2.size()){
			cout<<"ARRAY CORRUPTED!!\n";
		}
		end = MPI_Wtime();
		time_spent = (double)(end - begin);
		cout<<"Time: "<<time_spent<<" seconds to check if array is sorted."<<endl;
		if (NUM_OF_ELEMENTS< 1<<4)
			arrayPrint(data);
	}
	/* -------------------------- */

	if (world_rank==0)
		free(data);



    // Finalize the MPI environment.
    MPI_Finalize();
	return 0;
}
