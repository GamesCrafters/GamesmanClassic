
all: bittest
clean:
	rm -rf bittest *.o *~

bittest: bittest.o bits.o
	g++ -o bittest bittest.o bits.o

bits.o: bits.cc bits.h
	g++ -g -o bits.o -c bits.cc -O3 -Wall

bittest.o: bittest.cc bits.h
	g++ -g -o bittest.o -c bittest.cc -O3 -Wall

