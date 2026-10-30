#pragma once

#include "analysis/language.h"
#include "util/dynamic_loader.h"
#include "util/vector.h"

typedef struct plugin {
	dynamic_loader_handle handle;
	language_vector languages;
} plugin;

DEFINE_VECTOR(plugin);

int plugins_load();

plugin_vector* plugins_get();

void plugins_unload();