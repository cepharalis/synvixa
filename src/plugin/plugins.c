#include "plugins.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "util/string_util.h"

static plugin_vector plugins;

int plugins_load() {
	DIR* dir = opendir("plugins");
	if (!dir) {
		return 1;
	}
	struct dirent* entry;
	const char* extension = dynamic_loader_file_extension();
	while ((entry = readdir(dir))) {
		if (entry->d_type != DT_REG) {
			continue;
		}
		if (string_ends_with(entry->d_name, extension)) {
			char path[264] = "plugins/";
			memcpy(path + 8, entry->d_name, strlen(entry->d_name));
			// ReSharper disable once CppLocalVariableMayBeConst
			dynamic_loader_handle handle = dynamic_loader_load(path);
			if (!handle) {
				printf("Failed to open plugin: %s\n", entry->d_name);
				continue;
			}
			printf("Loading plugin: %s\n", entry->d_name);
			plugin p;
			p.handle = handle;
			plugin_vector_push(&plugins, p);
			void* load = dynamic_loader_get_symbol(handle, "load_plugin");
			if (load) {
				((void(*)())load)();
			}
		}
	}
	return 0;
}

void plugins_unload() {
	for (size_t i = 0; i < plugins.size; i++) {
		// ReSharper disable once CppLocalVariableMayBeConst
		dynamic_loader_handle handle = plugins.data[i].handle;
		void* unload = dynamic_loader_get_symbol(handle, "unload_plugin");
		if (unload) {
			((void(*)())unload)();
		}
		dynamic_loader_unload(handle);
	}
}
