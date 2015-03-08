T = 10
N = 4
S = q
O = 0

IPs = 11 12 16 17 19 20
Hosts = 172.16.7.13,172.16.7.11,172.16.7.12,172.16.7.16,172.16.7.17,172.16.7.19,172.16.7.20 

#H =  -hostfile host_file 
H = -H $(Hosts)

G_ARGS = mpic++ -fopenmp -fPIC -c -g  -Wall

all:
	@echo "try 'make test'"

bhelper: helper.cpp
	$(G_ARGS) helper.cpp

bpquick: pquick.cpp
	$(G_ARGS) pquick.cpp

bquick: quicksort.cpp
	$(G_ARGS) quicksort.cpp 

bmerge: mergesort.cpp
	$(G_ARGS) mergesort.cpp 

bradix: radixsort.cpp
	$(G_ARGS) radixsort.cpp 

bsort: sort.cpp
	$(G_ARGS) sort.cpp

ball: bquick bmerge bradix bsort bpquick bhelper

blib: ball
	mpic++ -shared -o libpsort.so quicksort.o mergesort.o radixsort.o sort.o pquick.o helper.o

unpublish:
	for ip in $(IPs) ; do \
		ssh 172.16.7.$$ip "rm -rf ~/Desktop/*"; \
	done


publish: unpublish 
	for ip in $(IPs) ; do \
		scp -prq 172.16.7.13:~/Desktop/mpi-sort/. 172.16.7.$$ip:~/Desktop/mpi-sort ; \
	done

localbuild: blib test.cpp
	mpic++ -fopenmp -Wall -o test test.cpp -B . -lpsort

build: localbuild publish
	
run: test
	mpirun -x LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:~/Desktop/mpi-sort -x PATH=$$PATH -x OMPE=$(O) -np $(N) $(H) ~/Desktop/mpi-sort/test $(T) $(S)

local: test
	mpirun -x LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:~/Desktop/mpi-sort -x OMPE=$(O) -np $(N) ./test $(T) $(S)
clean: unpublish
	rm -f *.o
	rm -f *.so
	rm -f test
	rm -f *.out
	clear


