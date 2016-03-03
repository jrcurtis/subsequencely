
#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "app.h"

extern const uint16_t APP_ID;
extern const uint16_t DATA_VERSION;

void serialize_app();
void deserialize_app();
void serialize_clear();

#endif
