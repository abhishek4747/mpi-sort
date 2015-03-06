T = 10
N = 4
S = q

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
	ssh 172.16.7.11 "rm -rf ~/Desktop/*"
	ssh 172.16.7.12 "rm -rf ~/Desktop/*"
	ssh 172.16.7.14 "rm -rf ~/Desktop/*"
	#ssh 172.16.7.15 "rm -rf ~/Desktop/*"
	ssh 172.16.7.16 "rm -rf ~/Desktop/*"
	ssh 172.16.7.17 "rm -rf ~/Desktop/*"
	ssh 172.16.7.19 "rm -rf ~/Desktop/*"
	ssh 172.16.7.20 "rm -rf ~/Desktop/*"


publish: 
	scp -prq 172.16.7.13:~/Desktop/mpi-sort/. 172.16.7.11:~/Desktop/mpi-sort
	scp -prq 172.16.7.13:~/Desktop/mpi-sort/. 172.16.7.12:~/Desktop/mpi-sort
	scp -prq 172.16.7.13:~/Desktop/mpi-sort/. 172.16.7.14:~/Desktop/mpi-sort
	#scp -prq 172.16.7.13:~/Desktop/mpi-sort/. 172.16.7.15:~/Desktop/mpi-sort
	scp -prq 172.16.7.13:~/Desktop/mpi-sort/. 172.16.7.16:~/Desktop/mpi-sort
	scp -prq 172.16.7.13:~/Desktop/mpi-sort/. 172.16.7.17:~/Desktop/mpi-sort
	scp -prq 172.16.7.13:~/Desktop/mpi-sort/. 172.16.7.19:~/Desktop/mpi-sort
	scp -prq 172.16.7.13:~/Desktop/mpi-sort/. 172.16.7.20:~/Desktop/mpi-sort

test: blib test.cpp
	mpic++ -o test test.cpp -B . -lpsort
	LD_LIBRARY_PATH=. mpirun -n $(N) ./test $(T) $(S)

clean:
	rm -f *.o
	rm -f *.so
	rm -f test
	rm -f *.out
	clear


