#pragma once
#include <iostream>

#include "enums.h"
#include "moves.h"

struct PerftMovesCount
{
    unsigned long long count = 0;
    unsigned long long captures = 0;
    unsigned long long en_pasants = 0;
    unsigned long long checks = 0;
    unsigned long long checkmates = 0;
    unsigned long long castles = 0;
    unsigned long long promotion = 0;
};

void printPerftObject(PerftMovesCount obj);

PerftMovesCount perf(int depth, Board &board);