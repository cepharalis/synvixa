#include "dynamic_loader.h"

#ifdef __linux__
#include <dlfcn.h>

char* dynamic_loader_file_extension() {
	return ".so";
}

dynamic_loader_handle dynamic_loader_load(const char* path) {
	return dlopen(path, RTLD_NOW);
}

// ReSharper disable once CppParameterMayBeConst
void* dynamic_loader_get_symbol(dynamic_loader_handle library, const char* symbol) {
	return dlsym(library, symbol);
}

// ReSharper disable once CppParameterMayBeConst
void dynamic_loader_unload(dynamic_loader_handle library) {
	dlclose(library);
}
#elifdef __APPLE__
#include <dlfcn.h>

char* dynamic_loader_file_extension() {
	return ".dylib";
}

dynamic_loader_handle dynamic_loader_load(const char* path) {
	return dlopen(path, RTLD_NOW);
}

// ReSharper disable once CppParameterMayBeConst
void* dynamic_loader_get_symbol(dynamic_loader_handle library, const char* symbol) {
	return dlsym(library, symbol);
}

// ReSharper disable once CppParameterMayBeConst
void dynamic_loader_unload(dynamic_loader_handle library) {
	dlclose(library);
}
#elifdef _WIN32

char* dynamic_loader_file_extension() {
	return ".dll";
}

dynamic_loader_handle dynamic_loader_load(const char* path) {
	return LoadLibrary(TEXT(path));
}

void* dynamic_loader_get_symbol(dynamic_loader_handle library, const char* symbol) {
	return GetProcAddress(library, symbol);
}

void dynamic_loader_unload(dynamic_loader_handle library) {
	FreeLibrary(library);
}
#else
#error Unsupported platform
#endif
