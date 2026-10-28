#include "plugin.h"

void load_plugin(plugin* p) {
	language c;
	c.name = "C";
	c.extensions = malloc(sizeof(char*));
	c.extension_count = 1;
	token_type_vector_init(&c.token_types, 0);
	language_vector_push(&p->languages, c);
}

void unload_plugin(const plugin* p) {
	free(p->languages.data[0].extensions);
	token_type_vector_free(&p->languages.data[0].token_types);
}
