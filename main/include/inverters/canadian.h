#ifndef _CANADIAN_
#define _CANADIAN_

#include <stdlib.h>
#include "paramDefinition.h"

static registerDefinition canadianRegisters[] = {{"acw", 3004, 2, S32, bigEndian, 1},
                                                 {"dcw", 3006, 2, U32, bigEndian, 1},
                                                 {"ent", 3008, 2, U32, bigEndian, 1},
                                                 {"enm", 3010, 2, U32, bigEndian, 1},
                                                 {"Energy_Last_Month", 3012, 2, U32, bigEndian, 1},
                                                 {"end", 3014, 1, U16, bigEndian, 0.1},
                                                 {"Energy_Yesterday", 3015, 1, U16, bigEndian, 0.1},
                                                 {"Energy_Year", 3016, 2, U32, bigEndian, 1},
                                                 {"Energy_Last_Year", 3019, 2, U32, bigEndian, 1},
                                                 {"dcv1", 3021, 1, U16, bigEndian, 0.1},
                                                 {"dca1", 3022, 1, U16, bigEndian, 0.1},
                                                 {"dcv2", 3023, 1, U16, bigEndian, 0.1},
                                                 {"dca2", 3024, 1, U16, bigEndian, 0.1},
                                                 {"dcv3", 3025, 1, U16, bigEndian, 0.1},
                                                 {"dca3", 3026, 1, U16, bigEndian, 0.1},
                                                 {"dcv4", 3027, 1, U16, bigEndian, 0.1},
                                                 {"dca4", 3028, 1, U16, bigEndian, 0.1},
                                                 {"DC_busbar_voltage", 3031, 1, U16, bigEndian, 0.1},
                                                 {"DC_half-busbar_voltage", 3032, 1, U16, bigEndian, 0.1},
                                                 {"vpha", 3033, 1, U16, bigEndian, 0.1},
                                                 {"vphb", 3034, 1, U16, bigEndian, 0.1},
                                                 {"vphc", 3035, 1, U16, bigEndian, 0.1},
                                                 {"apha", 3036, 1, U16, bigEndian, 0.1},
                                                 {"aphb", 3037, 1, U16, bigEndian, 0.1},
                                                 {"aphc", 3038, 1, U16, bigEndian, 0.1},
                                                 {"Working_Mode", 3040, 1, U16, bigEndian, 1},
                                                 {"tmp", 3041, 1, S16, bigEndian, 0.1},
                                                 {"hz", 3042, 1, U16, bigEndian, 0.01},
                                                 {"Status", 3043, 1, U16, bigEndian, 1},
                                                 {"Std", 3053, 1, U16, bigEndian, 1},
                                                 {"acvar", 3055, 2, S32, bigEndian, 1},
                                                 {"S", 3057, 2, S32, bigEndian, 1},
                                                 {"pf", 3059, 1, U16, bigEndian, 0.001},
                                                 {"sn", 3060, 4, str, littleEndian, 1},
                                                 {"Set_Flag", 3065, 1, U16, bigEndian, 1},
                                                 {"alarm1", 3066, 1, U16, bigEndian, 1},
                                                 {"alarm2", 3067, 1, U16, bigEndian, 1},
                                                 {"alarm3", 3068, 1, U16, bigEndian, 1},
                                                 {"alarm4", 3069, 1, U16, bigEndian, 1},
                                                 {"alarm5", 3070, 1, U16, bigEndian, 1},
                                                 {"st1", 3071, 1, U16, bigEndian, 1}};

static unsigned int canadianRegAmount = sizeof(canadianRegisters) / sizeof(registerDefinition);

#endif