#ifndef _PARAMDEFINITION_
#define _PARAMDEFINITION_

#include <stdlib.h>

typedef struct
{
    char label[25];
    uint16_t address;
    uint16_t size;
    uint8_t type;
    uint8_t endianness;
    float scaleFactor;
} registerDefinition;

typedef struct
{
    uint8_t slaveID;
    uint8_t standard;
    char manufacturer[30];
    char model[30];
    char serialNo[30];
    uint32_t baud;
} modbusDevice_t;

enum netTypes
{
    wifi,
    gprs
};

enum standards
{
    CanadianSolar = 0,
    SunspecFronius,
    Huawei,
    Sungrow,
    SunspecEcosolys,
    Canadian,
    SAJ,
    sofar,
    Refusol,
    SunspecRefusol,

};

enum endianess
{
    bigEndian,
    littleEndian
};

enum types
{
    U16,
    U32,
    S16,
    S32,
    F32,
    str,
    sunssf,
    U64
};

#endif