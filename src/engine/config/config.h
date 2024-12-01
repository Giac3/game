#pragma once

#include "../input/input.h"
#include "../types.h"

typedef struct Config {
    u8 keybinds[6];
} Config_State;

void config_init(void);
void config_key_bind(Input_Key key, const char *key_name);