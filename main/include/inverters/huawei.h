#ifndef _HUAWEI_
#define _HUAWEI_

#include <stdlib.h>
#include "paramDefinition.h"

static registerDefinition huaweiRegisters[] = {{"Md", 30000, 15, str, bigEndian, 1},
                                               {"sn", 30015, 10, str, bigEndian, 1},
                                               {"PN", 30025, 10, str, bigEndian, 1},
                                               {"Md_ID", 30070, 1, U16, bigEndian, 1},
                                               {"No_Strings", 30071, 1, U16, bigEndian, 1},
                                               {"No_MPPT", 30072, 1, U16, bigEndian, 1},
                                               {"Rated_Power", 30073, 2, U32, bigEndian, 0.001},
                                               {"PMax", 30075, 2, U32, bigEndian, 0.001},
                                               {"SMax", 30077, 2, U32, bigEndian, 0.001},
                                               {"QMax_fed", 30079, 2, S32, bigEndian, 0.001},
                                               {"QMax_abs", 30081, 2, S32, bigEndian, 0.001},
                                               {"st1", 32000, 1, U16, bigEndian, 1},
                                               {"st2", 32002, 1, U16, bigEndian, 1},
                                               {"st3", 32003, 2, U32, bigEndian, 1},
                                               {"alarm1", 32008, 1, U16, bigEndian, 1},
                                               {"alarm2", 32009, 1, U16, bigEndian, 1},
                                               {"alarm3", 32010, 1, U16, bigEndian, 1},
                                               {"dcv1", 32016, 1, S16, bigEndian, 0.1},
                                               {"dca1", 32017, 1, S16, bigEndian, 0.01},
                                               {"dcv2", 32018, 1, S16, bigEndian, 0.1},
                                               {"dca2", 32019, 1, S16, bigEndian, 0.01},
                                               {"dcv3", 32020, 1, S16, bigEndian, 0.1},
                                               {"dca3", 32021, 1, S16, bigEndian, 0.01},
                                               {"dcv4", 32022, 1, S16, bigEndian, 0.1},
                                               {"dca4", 32023, 1, S16, bigEndian, 0.01},
                                               {"dcw", 32064, 2, S32, bigEndian, 0.001},
                                               {"vphab", 32066, 1, U16, bigEndian, 0.1},
                                               {"vphbc", 32067, 1, U16, bigEndian, 0.1},
                                               {"vphca", 32068, 1, U16, bigEndian, 0.1},
                                               {"vpha", 32069, 1, U16, bigEndian, 0.1},
                                               {"vphb", 32070, 1, U16, bigEndian, 0.1},
                                               {"vphc", 32071, 1, U16, bigEndian, 0.1},
                                               {"apha", 32072, 2, S32, bigEndian, 0.001},
                                               {"aphb", 32074, 2, S32, bigEndian, 0.001},
                                               {"aphc", 32076, 2, S32, bigEndian, 0.001},
                                               {"Peak_W_Day", 32078, 2, S32, bigEndian, 0.001},
                                               {"acw", 32080, 2, S32, bigEndian, 0.001},
                                               {"acvar", 32082, 2, S32, bigEndian, 0.001},
                                               {"pf", 32084, 1, S16, bigEndian, 0.001},
                                               {"hz", 32085, 1, U16, bigEndian, 0.01},
                                               {"Efficiency", 32086, 1, U16, bigEndian, 0.01},
                                               {"tmp", 32087, 1, S16, bigEndian, 0.1},
                                               {"Ins_Resistance", 32088, 1, U16, bigEndian, 0.001},
                                               {"st4", 32089, 1, U16, bigEndian, 1},
                                               {"Fault_Code", 32090, 1, U16, bigEndian, 1},
                                               {"ent", 32106, 2, U32, bigEndian, 0.01},
                                               {"end", 32114, 2, U32, bigEndian, 0.01}};

static unsigned int huaweiRegAmount = sizeof(huaweiRegisters) / sizeof(registerDefinition);

#endif