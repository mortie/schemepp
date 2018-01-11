# Schemepp

Schemepp is a preprocessor, mostly designed for C, whose macros are written in
Scheme. It uses the chibi-scheme library.

## Usage

	schemepp [infile] [outfile]

## Syntax

Text between an opening `##(` and a matching `)` is interpreted as Scheme code.
If the expression returns a string, that string will be written to `[outfile]`
in place of the `##(...)`.

## Pre-defined functions and variables

* `cstr`: A function which takes a Scheme string, and returns a C string
  literal.
* `__file__`: A Scheme string which contains the absolute path of the current
  file, or `<stdin>` if input is from stdin.

## Example

	#include <stdio.h>
	int main() {
		printf("Hello from " ##(cstr __file__) "!\n");
	}

If that file is in, for example, `/home/foobar/src/something.sc`, it will
compile down to this C code:

	#include <stdio.h>
	int main() {
		printf("Hello from " "/home/foobar/src/something.sc" "!\n");
	}
