
all: sigdispatch

signames.h:
	./gen_signames.sh

sigdispatch: signames.h
	gcc $(CCFLAGS) $@.c -o $@
