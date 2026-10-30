#include "plugin.h"

void load_plugin(plugin* p) {
	language c;
	c.name = "C";
	c.extensions = malloc(sizeof(char*));
	c.extensions[0] = "c";
	c.extension_count = 1;
	token_type_vector_init(&c.token_types, 0);
	token_type_vector_push(&c.token_types, token_type_new("comment", /*language=RegExp*/ "//.*"));
	token_type_vector_push(&c.token_types, token_type_new("include", /*language=RegExp*/ "#include"));
	token_type_vector_push(&c.token_types, token_type_new("header-name", /*language=RegExp*/ "<.*[^-]>"));
	token_type_vector_push(&c.token_types, token_type_new("string-literal", /*language=RegExp*/ "\".*\""));
	token_type_vector_push(&c.token_types, token_type_new("keyword", /*language=RegExp*/ "(auto|break|case|char|const|continue|default|do|double|else|enum|extern|float|for|goto|if|inline|int|long|register|restrict|return|short|signed|sizeof|static|struct|switch|typedef|union|unsigned|void|volatile|while|_Alignas|_Alignof|_Atomic|_Bool|_Complex|_Generic|_Imaginary|_Noreturn|_Static_assert|_Thread_local)"));
	token_type_vector_push(&c.token_types, token_type_new("identifier", /*language=RegExp*/ "[a-zA-Z_$][a-zA-Z_0-9$]*"));
	token_type_vector_push(&c.token_types, token_type_new("constant", /*language=RegExp*/ "(([1-9][0-9]*([uU][lL]?|[uU](ll|LL)|[lL][uU]?|(ll|LL)[uU]?)?|0[0-7]*([uU][lL]?|[uU](ll|LL)|[lL][uU]?|(ll|LL)[uU]?)?|0[xX][0-9a-fA-F]+([uU][lL]?|[uU](ll|LL)|[lL][uU]?|(ll|LL)[uU]?)?)|((([0-9]*\\.[0-9]+|[0-9]+\\.)([eE][+-]?[0-9]+)?[fFlL]?|[0-9]+[eE][+-]?[0-9]+[fFlL]?)|0[xX](([0-9a-fA-F]*\\.[0-9a-fA-F]+|[0-9a-fA-F]+\\.)|[0-9a-fA-F]+)[pP][+-]?[0-9]+[fFlL]?)|[LuU]?'([^'\\\\]|(\\\\['\"?\\\\abfnrtv]|\\\\[0-7]{1,3}+)|\\\\x[0-9a-fA-F]+|\\\\u([0-9a-fA-F]{4}|[0-9a-fA-F]{8}))+')"));
	token_type_vector_push(&c.token_types, token_type_new("punctuator", /*language=RegExp*/ "->|\\+\\+|--|<<=|>>=|<<|>>|<=|>=|==|!=|&&|\\|\\||\\.\\.\\.|\\*=|/=|%=|\\+=|-=|&=|\\^=|\\|=|##|<:|:>|<%|%>|%:%:|%:|[]\\[(){}.&*+~!/%?:;=,#^|<>\\-]"));
	language_vector_push(&p->languages, c);
}

void unload_plugin(const plugin* p) {
	free(p->languages.data[0].extensions);
	token_type_vector_free(&p->languages.data[0].token_types);
}
