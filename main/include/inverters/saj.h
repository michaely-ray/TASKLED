#ifndef _SAJ_
#define _SAJ_

#include <stdlib.h>
#include "paramDefinition.h"

static registerDefinition sajRegisters[] = {

    {"acw", 275, 1, U16, bigEndian, 1},                 //113
    {"sn", 36611, 10, str, bigEndian, 1},               //8F03
    {"dcw1", 265, 1, U16, bigEndian, 1},                //109
    {"dcw2", 268, 1, U16, bigEndian, 1},                //10c
    {"dcw3", 271, 1, U16, bigEndian, 1},                //10f
    {"ent", 305, 2, U32, bigEndian, 0.01},              //131-0.01-u32
    {"end", 300, 1, U32, bigEndian, 0.01},              //12c-0.01-u32
    {"dcv1", 263, 1, U16, bigEndian, 0.1},              //107-0.1
    {"dca1", 264, 1, S16, bigEndian, 0.01},             //108-0.01
    {"dcv2", 266, 1, U16, bigEndian, 0.1},              //10a-0.1
    {"dca2", 267, 1, S16, bigEndian, 0.01},             //10b-0.01
    {"dcv3", 269, 1, U16, bigEndian, 0.1},              //10d-0.1
    {"dca3", 270, 1, S16, bigEndian, 0.01},             //10e-0.01
    {"DC_busbar_voltage", 272, 1, U16, bigEndian, 0.1}, //110 -0.1
    //All phases voltage and currents refer to the grid
    {"vpha", 278, 1, U16, bigEndian, 0.1},  //116-0.1
    {"vphb", 284, 1, U16, bigEndian, 0.1},  //11c-0.1
    {"vphc", 290, 1, U16, bigEndian, 0.1},  //122-0.1
    {"apha", 279, 1, S16, bigEndian, 0.01}, //117-0.01
    {"aphb", 285, 1, S16, bigEndian, 0.01}, //11d-0.01
    {"aphc", 291, 1, S16, bigEndian, 0.01}, //123-0.01
    // {"Working_Mode", 8453, 1, U16, bigEndian, 1}, //2105
    {"tmp", 273, 1, S16, bigEndian, 0.1}, //111-0.1-S16
    {"hz", 280, 1, U16, bigEndian, 0.01}, //118-0.01
    {"pf", 277, 1, S16, bigEndian, 1},    //115-0.001-s16
    {"acvar", 276, 1, S16, bigEndian, 1}, //114-s16
    {"st1", 256, 1, U16, bigEndian, 1},   //100

};

static unsigned int sajRegAmount = sizeof(sajRegisters) / sizeof(registerDefinition);

#endif