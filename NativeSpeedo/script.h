#pragma once


#include "TCPManager.h"

#include "..\..\inc\natives.h"
#include "..\..\inc\types.h"
#include "..\..\inc\enums.h"

#include "..\..\inc\main.h"

#include <iostream>

static bool script_running_bool = true;

void ScriptMain();
void ScriptExit();
void update();
int main();