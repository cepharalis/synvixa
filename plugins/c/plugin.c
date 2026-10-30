#include "plugin.h"

void load_plugin(plugin* p) {
	language c;
	c.name = "C";
	c.extensions = malloc(sizeof(char*));
	c.extensions[0] = ".c";
	c.extension_count = 1;
	token_type_vector_init(&c.token_types, 0);
	token_type_vector_push(&c.token_types, token_type_new("comment", /*language=RegExp*/ "(//.*|/\\*.*\\*/)", COMMENT));
	token_type_vector_push(&c.token_types, token_type_new("preprocessor", /*language=RegExp*/ "#(define|elif|elifdef|elifndef|else|embed|endif|error|if|ifdef|ifndef|include|line|pragma|undef|warning)?\\>", PREPROCESSOR));
	token_type_vector_push(&c.token_types, token_type_new("header-name", /*language=RegExp*/ "<.*[^-]>", STRING));
	token_type_vector_push(&c.token_types, token_type_new("string-literal", /*language=RegExp*/ "([LuU]|u8)?\".*\"", STRING));
	token_type_vector_push(&c.token_types, token_type_new("keyword", /*language=RegExp*/ "(auto|bool|break|case|char|const|continue|default|do|double|else|enum|extern|false|float|for|goto|if|inline|int|long|nullptr|register|restrict|return|short|signed|sizeof|static|struct|switch|true|typedef|union|unsigned|void|volatile|while|_Alignas|_Alignof|_Atomic|_Bool|_Complex|_Generic|_Imaginary|_Noreturn|_Static_assert|_Thread_local)\\>", KEYWORD));
	token_type_vector_push(&c.token_types, token_type_new("identifier", /*language=RegExp*/ "[a-zA-Z_$][a-zA-Z_0-9$]*", NORMAL));
	token_type_vector_push(&c.token_types, token_type_new("constant", /*language=RegExp*/ "(([1-9][0-9]*([uU][lL]?|[uU](ll|LL)|[lL][uU]?|(ll|LL)[uU]?)?|0[0-7]*([uU][lL]?|[uU](ll|LL)|[lL][uU]?|(ll|LL)[uU]?)?|0[xX][0-9a-fA-F]+([uU][lL]?|[uU](ll|LL)|[lL][uU]?|(ll|LL)[uU]?)?)|((([0-9]*\\.[0-9]+|[0-9]+\\.)([eE][+-]?[0-9]+)?[fFlL]?|[0-9]+[eE][+-]?[0-9]+[fFlL]?)|0[xX](([0-9a-fA-F]*\\.[0-9a-fA-F]+|[0-9a-fA-F]+\\.)|[0-9a-fA-F]+)[pP][+-]?[0-9]+[fFlL]?)|[LuU]?'([^'\\\\]|(\\\\['\"?\\\\abfnrtv]|\\\\[0-7]{1,3}+)|\\\\x[0-9a-fA-F]+|\\\\u([0-9a-fA-F]{4}|[0-9a-fA-F]{8}))+')", CONSTANT));
	token_type_vector_push(&c.token_types, token_type_new("punctuator", /*language=RegExp*/ "->|\\+\\+|--|<<=|>>=|<<|>>|<=|>=|==|!=|&&|\\|\\||\\.\\.\\.|\\*=|/=|%=|\\+=|-=|&=|\\^=|\\|=|##|<:|:>|<%|%>|%:%:|%:|[]\\[(){}.&*+~!/%?:;=,#^|<>\\-]", NORMAL));
	language_vector_push(&p->languages, c);

	language cpp;
	cpp.name = "C++";
	cpp.extensions = malloc(sizeof(char*) * 9);
	cpp.extensions[0] = ".cpp";
	cpp.extensions[1] = ".cxx";
	cpp.extensions[2] = ".c++";
	cpp.extensions[3] = ".cc";
	cpp.extensions[4] = ".hpp";
	cpp.extensions[5] = ".hxx";
	cpp.extensions[6] = ".h++";
	cpp.extensions[7] = ".hh";
	cpp.extensions[8] = ".h";
	cpp.extension_count = 9;
	token_type_vector_init(&cpp.token_types, 0);
	token_type_vector_init(&cpp.token_types, 0);
	token_type_vector_push(&cpp.token_types, token_type_new("comment", /*language=RegExp*/ "(//.*|/\\*.*\\*/)", COMMENT));
	token_type_vector_push(&cpp.token_types, token_type_new("preprocessor", /*language=RegExp*/ "#(define|elif|elifdef|elifndef|else|embed|endif|error|if|ifdef|ifndef|include|line|pragma|undef|warning)?\\>", PREPROCESSOR));
	token_type_vector_push(&cpp.token_types, token_type_new("header-name", /*language=RegExp*/ "<.*[^-]>", STRING));
	token_type_vector_push(&cpp.token_types, token_type_new("string-literal", /*language=RegExp*/ "([LuUR]|u8)??\".*\"", STRING));
	token_type_vector_push(&cpp.token_types, token_type_new("keyword", /*language=RegExp*/ "(alignas|alignof|and|and_eq|asm|atomic_cancel|atomic_commit|atomic_noexcept|auto|bitand|bitor|bool|break|case|catch|char|char8_t|char16_t|char32_t|class|compl|concept|const|consteval|constexpr|constinit|const_cast|continue|contract_assert|co_await|co_return|co_yield|decltype|default|delete|do|double|dynamic_cast|else|enum|explicit|export|extern|false|float|for|friend|goto|if|inline|int|long|mutable|namespace|new|noexcept|not|not_eq|nullptr|operator|or|or_eq|private|protected|public|register|reinterpret_cast|requires|return|short|signed|sizeof|static|static_assert|static_cast|struct|switch|synchronized|template|this|thread_local|throw|true|try|typedef|typeid|typename|union|unsigned|using|virtual|void|volatile|wchar_t|while|xor|xor_eq)\\>", KEYWORD));
	token_type_vector_push(&cpp.token_types, token_type_new("identifier", /*language=RegExp*/ "[a-zA-Z_$][a-zA-Z_0-9$]*", NORMAL));
	token_type_vector_push(&cpp.token_types, token_type_new("constant", /*language=RegExp*/ "(([1-9][0-9]*([uU][lL]?|[uU](ll|LL)|[lL][uU]?|(ll|LL)[uU]?)?|0[0-7]*([uU][lL]?|[uU](ll|LL)|[lL][uU]?|(ll|LL)[uU]?)?|0[xX][0-9a-fA-F]+([uU][lL]?|[uU](ll|LL)|[lL][uU]?|(ll|LL)[uU]?)?)|((([0-9]*\\.[0-9]+|[0-9]+\\.)([eE][+-]?[0-9]+)?[fFlL]?|[0-9]+[eE][+-]?[0-9]+[fFlL]?)|0[xX](([0-9a-fA-F]*\\.[0-9a-fA-F]+|[0-9a-fA-F]+\\.)|[0-9a-fA-F]+)[pP][+-]?[0-9]+[fFlL]?)|[LuU]?'([^'\\\\]|(\\\\['\"?\\\\abfnrtv]|\\\\[0-7]{1,3}+)|\\\\x[0-9a-fA-F]+|\\\\u([0-9a-fA-F]{4}|[0-9a-fA-F]{8}))+')", CONSTANT));
	token_type_vector_push(&cpp.token_types, token_type_new("punctuator", /*language=RegExp*/ "::|\\.\\*|->\\*|->|\\+\\+|--|<<=|>>=|<<|>>|<=|>=|==|!=|&&|\\|\\||\\.\\.\\.|\\*=|/=|%=|\\+=|-=|&=|\\^=|\\|=|##|<:|:>|<%|%>|%:%:|%:|[]\\[(){}.&*+~!/%?:;=,#^|<>\\-]", NORMAL));
	language_vector_push(&p->languages, cpp);
}

void unload_plugin(const plugin* p) {
	free(p->languages.data[0].extensions);
	token_type_vector_free(&p->languages.data[0].token_types);
	free(p->languages.data[1].extensions);
	token_type_vector_free(&p->languages.data[1].token_types);
}
