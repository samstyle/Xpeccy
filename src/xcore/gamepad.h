#pragma once

#include <string>

#include "xcore.h"

void padLoadConfig(std::string);
void padSaveConfig(std::string);

int padExists(std::string);
int padCreate(std::string);
void padDelete(std::string);
