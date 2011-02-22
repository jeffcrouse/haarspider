/*
 *  string_utils.c
 *  haarspider
 *
 *  Created by Jeffrey Crouse on 2/22/11.
 *  Copyright 2011 Eyebeam. All rights reserved.
 *
 */

#include "string_utils.h"

// -----------------------------------------
bool starts_with(string test, string prefix)
{
	if(prefix.length() > test.length()) return false;
	return test.compare(0, prefix.length(), prefix)==0;
}


// -----------------------------------------
bool ends_with(string test, string suffix)
{
	if(suffix.length() > test.length()) return false;
	return test.compare(test.length()-suffix.length(), suffix.length(), suffix)==0;
}

// -----------------------------------------
bool ends_with(string test, vector<string> suffix)
{
	for(int i=0; i<suffix.size(); i++) {
		if(ends_with(test, suffix[i])) return true;
	}
	return false;
}

// -----------------------------------------
template <class T> string tostr(const T& t)
{
    std::ostringstream oss;
    oss << t;
    return oss.str();
}
