
MODULES=misc.o db/db.o db/memdb.o db/bits.o solver/solver.o test.o
INCLUDES=include/db.h include/gamesman.h include/misc.h include/types.h

all: test

clean:
	rm -rf $(MODULES) test *.o *~ */*.o */*~

test: $(MODULES)
	gcc -O3 -o $@ $(MODULES) -Wall

%.o: %.c
	gcc -O3 -c -o $@ $< -Wall -Iinclude
