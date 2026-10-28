#pragma once

#if defined(__linux__) || defined(__APPLE__)
typedef void* dynamic_loader_handle;
#elifdef _WIN32
#include <windows.h>
typedef HMODULE dynamic_loader_handle;
#else
#error Unsupported platform
#endif

char* dynamic_loader_file_extension();

dynamic_loader_handle dynamic_loader_load(const char* path);

void* dynamic_loader_get_symbol(dynamic_loader_handle library, const char* symbol);

void dynamic_loader_unload(dynamic_loader_handle library);