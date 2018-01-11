#include <stdio.h>

##(import (chibi io))
char *foo = ##(cstr (file->string "foo.txt"));

int main() {
	printf("Hello from " ##(cstr __file__) "!\n");
	printf("The content of foo.txt: %s\n", foo);
}
