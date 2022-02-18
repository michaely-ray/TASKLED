#ifndef _CANADIANSOLAR_
#define _CANADIANSOLAR_

#include <stdlib.h>
#include "paramDefinition.h"

static registerDefinition canadianSolarRegisters[] = {{"acw1", 3074, 1, U16, bigEndian, 1},
                                                      {"acw2", 3075, 1, U16, bigEndian, 1},
                                                      {"acw3", 3076, 1, U16, bigEndian, 1},
                                                      {"dcw1", 3054, 1, U16, bigEndian, 1},
                                                      {"dcw2", 3057, 1, U16, bigEndian, 1},
                                                      {"enth", 3047, 2, U32, bigEndian, 0.1},
                                                      {"entl", 3048, 2, U32, bigEndian, 0.1},

                                                      {"endh", 3045, 1, U16, bigEndian, 0.1},
                                                      {"endl", 3046, 1, U16, bigEndian, 0.1},

                                                      {"dcv1", 3052, 1, U16, bigEndian, 0.1},
                                                      {"dca1", 3053, 1, U16, bigEndian, 0.01},
                                                      {"dcv2", 3055, 1, U16, bigEndian, 0.1},
                                                      {"dca2", 3056, 1, U16, bigEndian, 0.01},

                                                      {"DC_busbar_voltage", 3160, 1, U16, bigEndian, 0.1},

                                                      {"vpha", 3067, 1, U16, bigEndian, 0.1},
                                                      {"vphb", 3068, 1, U16, bigEndian, 0.1},
                                                      {"vphc", 3069, 1, U16, bigEndian, 0.1},
                                                      {"apha", 3070, 1, U16, bigEndian, 0.01},
                                                      {"aphb", 3071, 1, U16, bigEndian, 0.01},
                                                      {"aphc", 3072, 1, U16, bigEndian, 0.01},
                                                      //{"Working_Mode", 3040, 1, U16, bigEndian, 1},
                                                      {"tmp", 3087, 1, S16, bigEndian, 0.1},
                                                      {"hz", 3084, 1, U16, bigEndian, 0.01},
                                                      // {"Status", 3043, 1, U16, bigEndian, 1},
                                                      // {"Std", 3053, 1, U16, bigEndian, 1},
                                                      {"acvar", 3079, 2, S16, bigEndian, 1},
                                                      {"S", 3081, 2, S16, bigEndian, 1},
                                                      {"pf", 3083, 1, U16, bigEndian, 0.001},
                                                      {"sn", 3010, 8, str, littleEndian, 1},
                                                      // {"Set_Flag", 3065, 1, U16, bigEndian, 1},
                                                      {"alarm1", 3099, 1, U16, bigEndian, 1},
                                                      // {"alarm2", 3067, 1, U16, bigEndian, 1},
                                                      // {"alarm3", 3068, 1, U16, bigEndian, 1},
                                                      // {"alarm4", 3069, 1, U16, bigEndian, 1},
                                                      // {"alarm5", 3070, 1, U16, bigEndian, 1},
                                                      {"st1", 3090, 1, U16, bigEndian, 1}};

static unsigned int canadianSolarRegAmount = sizeof(canadianSolarRegisters) / sizeof(registerDefinition);

#endif