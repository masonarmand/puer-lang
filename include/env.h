/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
#ifndef ENV_H
#define ENV_H

#include "var.h"

void env_push(void);
void env_pop(void);
Var* env_get(const char* name);
void env_set(const char* name, Var val);
void env_clear();

#endif
