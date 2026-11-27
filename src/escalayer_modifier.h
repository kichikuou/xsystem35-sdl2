#ifndef ESCALAYER_MODIFIER_H
#define ESCALAYER_MODIFIER_H

#include <stdbool.h>
#include "xsystem35_main.h"

void escalayer_modifier_tick(void);
void escalayer_modifier_reset(bool active);
bool escalayer_modifier_available(void);
const char *xsystem35_modifier_current_game(void);

#endif
