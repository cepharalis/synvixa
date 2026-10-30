#include "plugin.h"

void load_plugin(plugin* p) {
	language cmake;
	cmake.name = "CMake";
	cmake.extensions = malloc(sizeof(char*) * 2);
	cmake.extensions[0] = "CMakeLists.txt";
	cmake.extensions[1] = ".cmake";
	cmake.extension_count = 2;
	token_type_vector_init(&cmake.token_types, 0);
	token_type_vector_push(&cmake.token_types, token_type_new("comment", /*language=RegExp*/ "#(\\[\\[.*]])", COMMENT));
	token_type_vector_push(&cmake.token_types, token_type_new("line_comment", /*language=RegExp*/ "#.*", COMMENT));
	token_type_vector_push(&cmake.token_types, token_type_new("command", /*language=RegExp*/ "[a-z_]+\\(", NORMAL));
	token_type_vector_push(&cmake.token_types, token_type_new("command_end", /*language=RegExp*/ "\\)", NORMAL));
	token_type_vector_push(&cmake.token_types, token_type_new("string", /*language=RegExp*/ "\".*\"", STRING));
	token_type_vector_push(&cmake.token_types, token_type_new("identifier", /*language=RegExp*/ "[^ )]+", KEYWORD));
	language_vector_push(&p->languages, cmake);
}

void unload_plugin(const plugin* p) {
	free(p->languages.data[0].extensions);
	token_type_vector_free(&p->languages.data[0].token_types);
}
