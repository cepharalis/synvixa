#include "plugin.h"

void load_plugin(plugin p) {
	language python;
	python.name = "Python";
	python.extensions = malloc(sizeof(char*));
	python.extensions[0] = ".py";
	python.extension_count = 1;
	token_type_vector_init(&python.token_types, 0);
	token_type_vector_push(&python.token_types, token_type_new("comment", /*language=RegExp*/ "(#.*)", COMMENT));
	token_type_vector_push(&python.token_types, token_type_new("header-name", /*language=RegExp*/ "(from|import).*", STRING));
	token_type_vector_push(&python.token_types, token_type_new("string-literal", /*language=RegExp*/ "[rfb]?(\"\"\"[\\s\\S]*?\"\"\"|'''[\\s\\S]*?'''|\".*?\"|'.*?')", STRING));
	token_type_vector_push(&python.token_types, token_type_new("keyword", /*language=RegExp*/ "\\b(False|None|True|and|as|assert|async|await|break|class|continue|def|del|elif|else|except|finally|for|from|global|if|import|in|is|lambda|nonlocal|not|or|pass|raise|return|try|while|with|yield)\\b", KEYWORD));
	token_type_vector_push(&python.token_types, token_type_new("identifier", /*language=RegExp*/ "[a-zA-Z_][a-zA-Z0-9_]*", NORMAL));
	token_type_vector_push(&python.token_types, token_type_new("constant", /*language=RegExp*/ "\\b[0-9]+(\\.[0-9]+)?\\b", CONSTANT));
	token_type_vector_push(&python.token_types, token_type_new("punctuator", /*language=RegExp*/ "[\\+\\-\\*/%&=><!~^&|\\(\\)\\[\\]\\{\\}:;,\\.]", NORMAL));
	language_vector_push(&p->languages, python);
}

void unload_plugin(const plugin p) {
	free(p->languages.data[0].extensions);
	token_type_vector_free(&p->languages.data[0].token_types);
}