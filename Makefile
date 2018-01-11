schemepp: src/main.c
	$(CC) -lchibi-scheme -o $@ $<

clean:
	rm -f schemepp
