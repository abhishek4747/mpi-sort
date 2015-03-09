#include "helper.h"

/*

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

MPI_Datatype getDataTypeMPI()    {   
	MPI_Datatype dataTypeMPI;
    int blocklens[] = {1, LOADSIZE}; 
    MPI_Datatype types[] = {MPI_LONG_LONG, MPI_CHAR};
    //MPI_Aint indices[] = {0, sizeof(long long)};
	MPI_Aint indices[2];
	indices[0] = (MPI_Aint)OFFSETOF(dataType, key);
	indices[1] = (MPI_Aint)OFFSETOF(dataType, payload);
    MPI_Type_create_struct(2, blocklens, indices, types, &dataTypeMPI);
    MPI_Type_commit(&dataTypeMPI);
    return dataTypeMPI;
}

*/

MPI_Datatype getDataTypeMPI()    {   
	MPI_Datatype dataTypeMPI;
    int blockLengths[] = {1, LOADSIZE, 1}; 
    MPI_Aint indices[] = {0, sizeof(long long), sizeof(dataType)};
    MPI_Datatype types[] = {MPI_LONG_LONG, MPI_CHAR, MPI_UB};
    MPI_Type_create_struct(3, blockLengths, indices, types, &dataTypeMPI);
    MPI_Type_commit(&dataTypeMPI);
    return dataTypeMPI;
}
