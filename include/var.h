/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
#ifndef VAR_H
#define VAR_H

typedef enum VarType {
        TYPE_INT,
} VarType;

typedef struct Var {
        VarType type;

        union {
                int i;
        } data;

        int is_const;
} Var;

#endif
