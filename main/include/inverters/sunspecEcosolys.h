#ifndef _SUNSPECECOSOLYS_
#define _SUNSPECECOSOLYS_

#include <stdlib.h>
#include "paramDefinition.h"

static registerDefinition ecosolysCommon[] = {{"Mn", 40005, 16, str, bigEndian, 1}, {"Md", 40021, 16, str, bigEndian, 1},
                {"Opt", 40037, 8, str, bigEndian, 1}, {"Vr", 40045, 8, str, bigEndian, 1}, {"sn", 40053, 16, str, bigEndian, 1}};

static registerDefinition ecosolysInverter[] = {{"atotal", 40073, 1, U16, bigEndian, 1}, {"apha", 40074, 1, U16, bigEndian, 1},
                {"aphb", 40075, 1, U16, bigEndian, 1}, {"aphc", 40076, 1, U16, bigEndian, 1}, {"asf", 40077, 1, sunssf, bigEndian, 1},
                {"vphab", 40078, 1, U16, bigEndian, 1}, {"vphbc", 40079, 1, U16, bigEndian, 1}, {"vphca", 40080, 1, U16, bigEndian, 1},
                {"vpha", 40081, 1, U16, bigEndian, 1}, {"vphb", 40082, 1, U16, bigEndian, 1}, {"vphc", 40083, 1, U16, bigEndian, 1},
                {"vsf", 40084, 1, sunssf, bigEndian, 1}, {"acw", 40085, 1, S16, bigEndian, 1}, {"acwsf", 40086, 1, sunssf, bigEndian, 1},
                {"hz", 40087, 1, U16, bigEndian, 1}, {"hzsf", 40088, 1, sunssf, bigEndian, 1}, {"VA", 40089, 1, S16, bigEndian, 1},
                {"VA_SF", 40090, 1, sunssf, bigEndian, 1}, {"acvar", 40091, 1, S16, bigEndian, 1}, {"acvarsf", 40092, 1, sunssf, bigEndian, 1},
                {"pf", 40093, 1, S16, bigEndian, 1}, {"pfsf", 40094, 1, sunssf, bigEndian, 1}, {"ent", 40095, 2, U32, bigEndian, 1},
                {"entsf", 40097, 1, sunssf, bigEndian, 1}, {"dca1", 40098, 1, U16, bigEndian, 1}, {"dcasf", 40099, 1, sunssf, bigEndian, 1},
                {"dcv1", 40100, 1, U16, bigEndian, 1}, {"dcvsf", 40101, 1, sunssf, bigEndian, 1}, {"dcw", 40102, 1, S16, bigEndian, 1},
                {"dcwsf", 40103, 1, sunssf, bigEndian, 1}, {"tmp", 40104, 1, S16, bigEndian, 1}, {"TmpSnk", 40105, 1, S16, bigEndian, 1},
                {"TmpTrns", 40106, 1, S16, bigEndian, 1}, {"TmpOt", 40107, 1, S16, bigEndian, 1}, {"tmpsf", 40108, 1, sunssf, bigEndian, 1},
                {"st1", 40109, 1, U16, bigEndian, 1}, {"st2", 40110, 1, U16, bigEndian, 1}, {"Evt1", 40111, 2, U32, bigEndian, 1},
                {"Evt2", 40113, 2, U32, bigEndian, 1}, {"alarm1", 40115, 2, U32, bigEndian, 1}, {"alarm2", 40117, 2, U32, bigEndian, 1},
                {"alarm3", 40119, 2, U32, bigEndian, 1}, {"alarm4", 40121, 2, U32, bigEndian, 1}, {"end", 40163, 1, U16, bigEndian, 1}};

static registerDefinition ecosolysMultiMPPT[] = {{"DCA_SF", 40265, 1, sunssf, bigEndian, 1}, {"DCV_SF", 40266, 1, sunssf, bigEndian, 1},
                {"DCW_SF", 40267, 1, sunssf, bigEndian, 1}, {"DCWH_SF", 40268, 1, sunssf , bigEndian, 1}, {"Evt", 40269, 2, U32, bigEndian, 1},
                {"N", 40271, 1, U16, bigEndian, 1}, {"1_ID", 40273, 1, U16, bigEndian, 1}, {"1_IDStr", 40274, 8, str, bigEndian, 1},
                {"1_DCA", 40282, 1, U16, bigEndian, 1}, {"1_DCV", 40283, 1, U16, bigEndian, 1}, {"1_DCW", 40284, 1, U16, bigEndian, 1},
                {"1_Tms", 40287, 2, U32, bigEndian, 1}, {"2_ID", 40293, 1, U16, bigEndian, 1}, {"2_IDStr", 40294, 8, str, bigEndian, 1},
                {"2_DCA", 40302, 1, U16, bigEndian, 1}, {"2_DCV", 40303, 1, U16, bigEndian, 1}, {"2_DCW", 40304, 1, U16, bigEndian, 1},
                {"2_Tms", 40307, 2, U32, bigEndian, 1}};

static unsigned int ecosolysCommonRegAmount = sizeof(ecosolysCommon)/sizeof(registerDefinition);
static unsigned int ecosolysInverterRegAmount = sizeof(ecosolysInverter)/sizeof(registerDefinition);
static unsigned int ecosolysMultiMPPTRegAmount = sizeof(ecosolysMultiMPPT)/sizeof(registerDefinition);
static unsigned int ecosolysTotalRegAmount = ecosolysCommonRegAmount+ecosolysInverterRegAmount;

#endif