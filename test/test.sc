#include <stdio.h>

int main() {
	printf("Hello from " ##(cstr __file__) "!\n");
}
