
###
### Stub Makefile
###


all:
	@$(MAKE) -w -C ../src all

clean:
	@$(MAKE) -w -C ../src clean-bins
	rm -rf *~

../src/%.o:
	@$(MAKE) -w -C ../src $@

m%: ../src/gamesman.o ../src/m%.o
	@$(MAKE) -w -C ../src ../bin/$@
