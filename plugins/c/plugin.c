#include "plugin.h"

#include <stdio.h>

void load_plugin() {
	printf("Hello, world!\n");
}

void unload_plugin() {
	printf("Unloading\n");
}
