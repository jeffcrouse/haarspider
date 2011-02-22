/*
 *  string_utils.h
 *  haarspider
 *
 *  Created by Jeffrey Crouse on 2/22/11.
 *  Copyright 2011 Eyebeam. All rights reserved.
 *
 */

#pragma once
#include <iostream>
#include <string.h>
#include <vector>
#include <sstream>

using namespace std;

string truncate(string str, int n=60);

bool starts_with(string test, string prefix);
bool ends_with(string test, string suffix);
bool ends_with(string test, vector<string> suffix);