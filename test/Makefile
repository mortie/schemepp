test: test.c
	gcc -o $@ $<

test.c: test.sc
	../schemepp $< $@

clean:
	rm -f test.c test
