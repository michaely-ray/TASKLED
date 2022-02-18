#ifndef _SOFAR_
#define _SOFAR_

#include <stdlib.h>
#include "paramDefinition.h"

static registerDefinition sofarRegisters[] = {
    {"acw", 12, 1, U16, bigEndian, 0.01},//
    {"sn", 0, 1, str, bigEndian, 1},//

    {"dcw1", 10, 1, U16, bigEndian, 0.01},//ok
    {"dcw2", 11, 1, U16, bigEndian, 0.01},//ok
    {"ent", 21, 2, U16, bigEndian, 1},//
    {"end", 25, 1, U16, bigEndian, 0.01},//
    {"dcv1", 6, 1, U16, bigEndian, 0.1}, //ok
    {"dca1", 7, 1, S16, bigEndian, 0.01}, //ok
    {"dcv2", 8, 1, U16, bigEndian, 0.1}, //ok
    {"dca2", 9, 1, S16, bigEndian, 0.01}, //ok
    {"DC_busbar_voltage", 29, 1, U16, bigEndian, 0.1}, //ok
    {"vpha", 15, 1, U16, bigEndian, 0.1}, //ok
    {"vphb", 17, 1, U16, bigEndian, 0.1}, //ok
    {"vphc", 19, 1, U16, bigEndian, 0.1}, //ok
    {"apha", 16, 1, U16, bigEndian, 0.01}, //ok
    {"aphb", 18, 1, U16, bigEndian, 0.01}, //ok
    {"aphc", 20, 1, U16, bigEndian, 0.01}, //ok
    {"Working_Mode", 0, 1, U16, bigEndian, 1}, //ok
    {"tmp", 28, 1, S16, bigEndian, 1}, //ok
    {"hz", 14, 1, S16, bigEndian, 0.01}, //ok
    {"pf", 13, 1, S16, bigEndian, 0},//13
    {"acvar", 13, 1, S16, bigEndian, 0.01},//13
    {"alarm1", 1, 1, U16, bigEndian, 1},//ok
    {"alarm2", 2, 1, U16, bigEndian, 1},//ok
    {"alarm3", 3, 1, U16, bigEndian, 1},//ok
    {"alarm4", 4, 1, U16, bigEndian, 1},//ok
    {"alarm5", 5, 1, U16, bigEndian, 1},//ok
    {"st1", 0, 1, U16, bigEndian, 1},
};

static unsigned int sofarRegAmount = sizeof(sofarRegisters) / sizeof(registerDefinition);

#endif