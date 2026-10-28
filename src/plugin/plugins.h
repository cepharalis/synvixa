#pragma once

#include "util/dynamic_loader.h"
#include "util/vector.h"

typedef struct plugin {
	dynamic_loader_handle handle;
} plugin;

DEFINE_VECTOR(plugin);

int plugins_load();

void plugins_unload();