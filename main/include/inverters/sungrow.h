#ifndef _SUNGROW_
#define _SUNGROW_

#include <stdlib.h>
#include "paramDefinition.h"

static registerDefinition sungrowRegisters[] = {{"sn", 4989, 10, str, bigEndian, 1},
                                                {"Device_Type_Code", 4999, 1, U16, bigEndian, 1},
                                                {"Nominal_Output_P", 5000, 1, U16, bigEndian, 0.1},
                                                {"Output_Type", 5001, 1, U16, bigEndian, 1},
                                                {"end", 5002, 1, U16, bigEndian, 0.1},
                                                {"ent", 5003, 2, U16, bigEndian, 1},
                                                {"Total_Running_Time", 5005, 2, U16, bigEndian, 1},
                                                {"tmp", 5007, 1, S16, bigEndian, 0.1},
                                                {"dcv1", 5010, 1, U16, bigEndian, 0.1},
                                                {"dca1", 5011, 1, U16, bigEndian, 0.1},
                                                {"dcv2", 5012, 1, U16, bigEndian, 0.1},
                                                {"dca2", 5013, 1, U16, bigEndian, 0.1},
                                                {"dcv3", 5014, 1, U16, bigEndian, 0.1},
                                                {"dca3", 5015, 1, U16, bigEndian, 0.1},
                                                {"dcw", 5016, 2, U16, bigEndian, 1},
                                                {"vpha", 5018, 1, U16, bigEndian, 0.1},
                                                {"vphb", 5019, 1, U16, bigEndian, 0.1},
                                                {"vphc", 5020, 1, U16, bigEndian, 0.1},
                                                {"apha", 5021, 1, U16, bigEndian, 0.1},
                                                {"aphb", 5022, 1, U16, bigEndian, 0.1},
                                                {"aphc", 5023, 1, U16, bigEndian, 0.1},
                                                {"acw", 5030, 2, U16, bigEndian, 1},
                                                {"acvar", 5032, 2, S16, bigEndian, 1},
                                                {"pf", 5034, 1, S16, bigEndian, 0.001},
                                                {"hz", 5035, 1, U16, bigEndian, 0.1},
                                                {"st1", 5037, 1, U16, bigEndian, 1},
                                                {"Fault_Year", 5038, 1, U16, bigEndian, 1},
                                                {"Fault_Month", 5039, 1, U16, bigEndian, 1},
                                                {"Fault_Day", 5040, 1, U16, bigEndian, 1},
                                                {"Fault_Hour", 5041, 1, U16, bigEndian, 1},
                                                {"Fault_Min", 5042, 1, U16, bigEndian, 1},
                                                {"Fault_Sec", 5043, 1, U16, bigEndian, 1},
                                                {"alarm1", 5044, 1, U16, bigEndian, 1},
                                                {"Nominal_Output_Q", 5048, 1, U16, bigEndian, 0.1},
                                                {"Impedance_GND", 5070, 1, U16, bigEndian, 1},
                                                {"st2", 5080, 2, U16, bigEndian, 1},
                                                {"Daily_Running_Time", 5112, 1, U16, bigEndian, 1},
                                                {"Country", 5113, 1, U16, bigEndian, 1},
                                                {"dcv4", 5114, 1, U16, bigEndian, 0.1},
                                                {"dca4", 5115, 1, U16, bigEndian, 0.1},
                                                {"Monthly_P_Yields", 5127, 2, U16, bigEndian, 0.1},
                                                {"Negative_V_to_GND", 5145, 1, S16, bigEndian, 0.1},
                                                {"Bus_V", 5146, 1, U16, bigEndian, 0.1},
                                                {"hz2", 5147, 1, U16, bigEndian, 0.01}};

static unsigned int sungrowRegAmount = sizeof(sungrowRegisters) / sizeof(registerDefinition);

#endif