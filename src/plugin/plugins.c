#include "plugins.h"

#include <stdio.h>
#include <string.h>

#if defined(__linux__) || defined(__APPLE__)
#include <dirent.h>
#elifdef __WIN32
#include <windows.h>
#else
#error Unsupported platform
#endif

#include "util/string_util.h"

static plugin_vector plugins;

static void load_plugin(char* name) {
	char path[264] = "plugins/";
	memcpy(path + 8, name, strlen(name));
	// ReSharper disable once CppLocalVariableMayBeConst
	dynamic_loader_handle handle = dynamic_loader_load(path);
	if (!handle) {
		printf("Failed to open plugin: %s\n", name);
		return;
	}
	printf("Loading plugin: %s\n", name);
	plugin p;
	p.handle = handle;
	language_vector_init(&p.languages, 0);
	void* load = dynamic_loader_get_symbol(handle, "load_plugin");
	if (load) {
		((void(*)(plugin*))load)(&p);
	}
	plugin_vector_push(&plugins, p);
}

int plugins_load() {
	plugin_vector_init(&plugins, 0);
	const char* extension = dynamic_loader_file_extension();
#if defined(__linux__) || defined(__APPLE__)
	DIR* dir = opendir("plugins");
	if (!dir) {
		return 1;
	}
	struct dirent* entry;
	while ((entry = readdir(dir))) {
		if (entry->d_type != DT_REG) {
			continue;
		}
		char* name = entry->d_name;
		if (string_ends_with(name, extension)) {
			load_plugin(name);
		}
	}
#elifdef __WIN32
	WIN32_FIND_DATAW find_data;
	// ReSharper disable once CppLocalVariableMayBeConst
	HANDLE h = FindFirstFileW(L"plugins\\*", &find_data);
	if (h == INVALID_HANDLE_VALUE) {
		return 1;
	}
	do {
		const int size = WideCharToMultiByte(CP_UTF8, 0, find_data.cFileName, -1, nullptr, 0, nullptr, nullptr);
		char* name = malloc(size);
		WideCharToMultiByte(CP_UTF8, 0, find_data.cFileName, -1, name, size, nullptr, nullptr);
		if (string_ends_with(name, extension)) {
			load_plugin(name);
		}
		free(name);
	} while (FindNextFileW(h, &find_data));
	FindClose(h);
#else
#error Unsupported platform
#endif
	return 0;
}

plugin_vector* plugins_get() {
	return &plugins;
}

void plugins_unload() {
	for (size_t i = 0; i < plugins.size; i++) {
		// ReSharper disable once CppLocalVariableMayBeConst
		dynamic_loader_handle handle = plugins.data[i].handle;
		void* unload = dynamic_loader_get_symbol(handle, "unload_plugin");
		if (unload) {
			((void(*)(plugin*))unload)(&plugins.data[i]);
		}
		dynamic_loader_unload(handle);
	}
}
