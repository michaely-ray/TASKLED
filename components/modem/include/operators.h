#ifndef _OPERATORS_
#define _OPERATORS_

typedef struct
{
    char name[15];
    char apn[25];
} operatorDefinition_t;

typedef enum
{
    claro,
    vivo,
    oi,
    tim,
    nextel,
    sercomtel,
    ctbc
} operatorName_t;
// static operatorDefinition_t operators[] = {{"Claro", "up.claro.com.br"}, {"Vivo", "up.vivo.com.br"}, {"Oi", "up.oi.com.br"}, {"Tim", "timbrasil.br"}, {"Nextel", "up.nextel.com.br"}, {"Sercomtel", "up.sercomtel.com.br"}, {"CTBC", "up.ctbc.com.br"}, {"Algar", "upchip.br"}};
static operatorDefinition_t operators[] = {{"Claro", "upchip.br"}, {"Vivo", "upchip.br"}, {"Oi", "upchip.br"}, {"Tim", "upchip.br"}, {"Nextel", "upchip.br"}, {"Sercomtel", "upchip.br"}, {"CTBC", "upchip.br"}, {"Algar", "upchip.br"}};

#endif