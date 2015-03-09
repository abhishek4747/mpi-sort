#include "helper.h"

MPI_Datatype dataTypeMPI;
bool isDeclared = false;

MPI_Datatype getDataTypeMPI()    {   
    if(isDeclared)
        return dataTypeMPI;
    isDeclared = true;

    int blocklens[] = {1, LOADSIZE}; 
    MPI_Datatype types[] = {MPI_LONG_LONG, MPI_CHAR};
    MPI_Aint indices[] = {0, sizeof(long long)};
    MPI_Type_create_struct(2, blocklens, indices, types, &dataTypeMPI);
    MPI_Type_commit(&dataTypeMPI);
    return dataTypeMPI;
}


