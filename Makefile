schemepp: src/main.c std.scm
	$(CC) -lchibi-scheme -o $@ $<

clean:
	rm -f schemepp
