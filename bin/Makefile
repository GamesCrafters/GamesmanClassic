
###
### Stub Makefile
###


all:
	@$(MAKE) -w -C ../src all

clean:
	@$(MAKE) -w -C ../src clean-bins
	rm -rf *~

memdebug:
	@$(MAKE) -w -C ../src memdebug

../src/%.o:
	@$(MAKE) -w -C ../src $@

m%: ../src/gamesman.o ../src/m%.o
	@$(MAKE) -w -C ../src ../bin/$@
