
all: sigdispatch

.PHONY: all

signames.h:
	./gen_signames.sh > $@

sigdispatch: sigdispatch.c signames.h
	gcc $(CCFLAGS) $@.c -o $@

install: /usr/tool/sigdispatch

.PHONY: install

/usr/tool/sigdispatch: sigdispatch
	install --compare $< $@
