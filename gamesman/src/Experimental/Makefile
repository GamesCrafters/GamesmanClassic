
MODULES=misc.o db/db.o db/memdb.o db/bits.o solver/solver.o test.o games.o
INCLUDES=include/db.h include/gamesman.h include/misc.h include/types.h include/games.h

all: test

clean:
	rm -rf $(MODULES) test *.o *~ */*.o */*~

test: $(MODULES)
	gcc -O3 -g -o $@ $(MODULES) -Wall

%.o: %.c
	gcc -O3 -g -c -o $@ $< -Wall -Iinclude
