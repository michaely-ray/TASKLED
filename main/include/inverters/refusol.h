#ifndef _REFUSOL_
#define _REFUSOL_

#include <stdlib.h>
#include "paramDefinition.h"

static registerDefinition refusolRegisters[] = {
    {"acw", 12, 1, U16, bigEndian, 0.01},
    {"sn", 0, 1, str, bigEndian, 1},

    {"dcw1", 10, 1, U16, bigEndian, 0.01},
    {"dcw2", 11, 1, U16, bigEndian, 0.01},
    {"ent", 21, 2, U16, bigEndian, 1},
    {"end", 25, 1, U16, bigEndian, 0.01},
    {"dcv1", 6, 1, U16, bigEndian, 0.1},
    {"dca1", 7, 1, S16, bigEndian, 0.01},
    {"dcv2", 8, 1, U16, bigEndian, 0.1},
    {"dca2", 9, 1, S16, bigEndian, 0.01},
    {"DC_busbar_voltage", 29, 1, U16, bigEndian, 0.1},
    {"vpha", 15, 1, U16, bigEndian, 0.1},
    {"vphb", 17, 1, U16, bigEndian, 0.1},
    {"vphc", 19, 1, U16, bigEndian, 0.1},
    {"apha", 16, 1, U16, bigEndian, 0.01},
    {"aphb", 18, 1, U16, bigEndian, 0.01},
    {"aphc", 20, 1, U16, bigEndian, 0.01},
    {"Working_Mode", 0, 1, U16, bigEndian, 1},
    {"tmp", 28, 1, S16, bigEndian, 1},
    {"hz", 14, 1, S16, bigEndian, 0.01},
    {"pf", 13, 1, S16, bigEndian, 0.01},
    {"acvar", 13, 1, S16, bigEndian, 0.01},
    {"alarm1", 1, 1, U16, bigEndian, 1},
    {"alarm2", 2, 1, U16, bigEndian, 1},
    {"alarm3", 3, 1, U16, bigEndian, 1},
    {"alarm4", 4, 1, U16, bigEndian, 1},
    {"alarm5", 5, 1, U16, bigEndian, 1},
    {"st1", 0, 1, U16, bigEndian, 1},
};

static unsigned int refusolRegAmount = sizeof(refusolRegisters) / sizeof(registerDefinition);

#endif