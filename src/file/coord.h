//
// Created by francesco on 26/11/21.
//

#ifndef CHORA_COORD_H
#define CHORA_COORD_H

#include "../std/types.h"

typedef struct {
    u64 line;
    u64 column;
} Coordinates;

typedef struct {
    Coordinates start;
    Coordinates end;
} Span;

#endif //CHORA_COORD_H
