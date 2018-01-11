test: test.c
	gcc -o $@ $<

test.c: test.sc foo.txt
	../schemepp $< $@

clean:
	rm -f test.c test
