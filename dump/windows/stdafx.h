/*
 * Page table dumper for Windows
 *
 * (C) Copyright 2016 Jethro G. Beekman
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */

#pragma once

#include "targetver.h"

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
using namespace std;

#include "ptdump-common.h"
#include "main.h"
