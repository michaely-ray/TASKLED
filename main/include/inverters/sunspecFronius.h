#ifndef _SUNSPECFRONIUS_
#define _SUNSPECFRONIUS_

#include <stdlib.h>
#include "paramDefinition.h"

static registerDefinition froniusCommon[] = {{"Mn", 40004, 16, str, bigEndian, 1},
                                             {"Md", 40020, 16, str, bigEndian, 1},
                                             {"Opt", 40036, 8, str, bigEndian, 1},
                                             {"Vr", 40044, 8, str, bigEndian, 1},
                                             {"sn", 40052, 16, str, bigEndian, 1},
                                             {"DA", 40068, 1, U16, bigEndian, 1}};

static registerDefinition froniusInverter[] = {{"atotal", 40071, 2, F32, bigEndian, 1},
                                               {"apha", 40073, 2, F32, bigEndian, 1},
                                               {"aphb", 40075, 2, F32, bigEndian, 1},
                                               {"aphc", 40077, 2, F32, bigEndian, 1},
                                               {"vphab", 40079, 2, F32, bigEndian, 1},
                                               {"vphbc", 40081, 2, F32, bigEndian, 1},
                                               {"vphca", 40083, 2, F32, bigEndian, 1},
                                               {"vpha", 40085, 2, F32, bigEndian, 1},
                                               {"vphb", 40087, 2, F32, bigEndian, 1},
                                               {"vphc", 40089, 2, F32, bigEndian, 1},
                                               {"acw", 40091, 2, F32, bigEndian, 1},
                                               {"hz", 40093, 2, F32, bigEndian, 1},
                                               {"VA", 40095, 2, F32, bigEndian, 1},
                                               {"acvar", 40097, 2, F32, bigEndian, 1},
                                               {"pf", 40099, 2, F32, bigEndian, 0.01},
                                               {"ent", 40101, 2, F32, bigEndian, 1},
                                               {"DCA", 40103, 2, F32, bigEndian, 1},
                                               {"DCV", 40105, 2, F32, bigEndian, 1},
                                               {"dcw", 40107, 2, F32, bigEndian, 1},
                                               {"st1", 40117, 1, U16, bigEndian, 1},
                                               {"st2", 40118, 1, U16, bigEndian, 1},
                                               {"Evt1", 40119, 2, U32, bigEndian, 1},
                                               {"Evt2", 40121, 2, U32, bigEndian, 1},
                                               {"alarm1", 40123, 2, U32, bigEndian, 1},
                                               {"alarm2", 40125, 2, U32, bigEndian, 1},
                                               {"alarm3", 40127, 2, U32, bigEndian, 1},
                                               {"alarm4", 40129, 2, U32, bigEndian, 1},
                                               {"end", 501, 4, U64, bigEndian, 0.001}};

static registerDefinition froniusMultiMPPT[] = {{"dcasf", 40265, 1, sunssf, bigEndian, 1},
                                                {"dcvsf", 40266, 1, sunssf, bigEndian, 1},
                                                {"dcwsf", 40267, 1, sunssf, bigEndian, 1},
                                                {"dcwhsf", 40268, 1, sunssf, bigEndian, 1},
                                                {"Evt", 40269, 2, U32, bigEndian, 1},
                                                {"N", 40271, 1, U16, bigEndian, 1},
                                                {"1_ID", 40273, 1, U16, bigEndian, 1},
                                                {"1_IDStr", 40274, 8, str, bigEndian, 1},
                                                {"dca1", 40282, 1, U16, bigEndian, 1},
                                                {"dcv1", 40283, 1, U16, bigEndian, 1},
                                                {"dcw1", 40284, 1, U16, bigEndian, 1},
                                                {"1_Tms", 40287, 2, U32, bigEndian, 1},
                                                {"2_ID", 40293, 1, U16, bigEndian, 1},
                                                {"2_IDStr", 40294, 8, str, bigEndian, 1},
                                                {"dca2", 40302, 1, U16, bigEndian, 1},
                                                {"dcv2", 40303, 1, U16, bigEndian, 1},
                                                {"dcw2", 40304, 1, U16, bigEndian, 1},
                                                {"2_Tms", 40307, 2, U32, bigEndian, 1}};

static unsigned int froniusCommonRegAmount = sizeof(froniusCommon) / sizeof(registerDefinition);
static unsigned int froniusInverterRegAmount = sizeof(froniusInverter) / sizeof(registerDefinition);
static unsigned int froniusMultiMPPTRegAmount = sizeof(froniusMultiMPPT) / sizeof(registerDefinition);
static unsigned int froniusTotalRegAmount = froniusCommonRegAmount + froniusInverterRegAmount + froniusMultiMPPTRegAmount;

#endif