#ifndef COMPONENTEXAMPLE_H
#define COMPONENTEXAMPLE_H
#include "../Engine.h"

void ComponentExampleConstructor(void** data);
void ComponentExampleDestructor(void** data);
void* ComponentExampleCopy(void* data);
cJSON* ComponentExampleEncode(void** data, cJSON* currentData);
void* ComponentExampleDecode(cJSON **data);

#endif