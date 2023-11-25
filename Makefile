
all: sigdispatch

.PHONY: all

signames.h:
	./gen_signames.sh > $@

sigdispatch: sigdispatch.c signames.h
	gcc $(CCFLAGS) $@.c -o $@

install: /usr/tool/sigdispatch /usr/share/man/man1/sigdispatch.1.gz

.PHONY: install

/usr/tool/sigdispatch: sigdispatch
	install --compare $< $@

/usr/share/man/man1/sigdispatch.1.gz: sigdispatch.pod
	pod2man --name=sigdispatch --section=1 --utf8 "$<" | gzip > "$@~"
	mv "$@~" "$@"
	/etc/cron.daily/man-db
