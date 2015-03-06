T = 10
N = 4
S = q

IPs = 11 12 14 16 17 19 20

G_ARGS = mpic++  -fPIC -c -g  -Wall 

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

bother: othersort.cpp
	$(G_ARGS) othersort.cpp 

bsort: sort.cpp
	$(G_ARGS) sort.cpp

ball: bquick bmerge bradix bother bsort bpquick bhelper

blib: ball
	mpic++ -shared -o libpsort.so quicksort.o mergesort.o radixsort.o othersort.o sort.o pquick.o helper.o

unpublish:
	for ip in $(IPs) ; do \
		ssh 172.16.7.$$ip "rm -rf ~/Desktop/*"; \
	done


publish: 
	for ip in $(IPs) ; do \
		scp -prq 172.16.7.13:~/Desktop/mpi-sort/. 172.16.7.$$ip:~/Desktop/mpi-sort ; \
	done
test: blib test.cpp
	mpic++ -o test test.cpp -B . -lpsort
	LD_LIBRARY_PATH=. mpirun -n $(N) ./test $(T) $(S)

clean:
	rm -f *.o
	rm -f *.so
	rm -f test
	rm -f *.out
	clear


