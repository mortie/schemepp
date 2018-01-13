PREFIX ?= /usr/local

schemepp: src/main.c scheme-std.c
	$(CC) -lchibi-scheme -o $@ $<

scheme-std.c: scheme-std.scm
	schemepp <<< "##(import (chibi io))##(cstr (file->string \"$<\"))" > $@

install: schemepp
	mv schemepp "$(PREFIX)/bin"

clean:
	rm -f schemepp
