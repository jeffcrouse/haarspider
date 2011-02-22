/*
 *  curl_util.h
 *  haarspider
 *
 *  Created by Jeffrey Crouse on 2/22/11.
 *  Copyright 2011 Eyebeam. All rights reserved.
 *
 */

#pragma once
#include <iostream>
#include <string.h>
#include <curl/curl.h>

using namespace std;

string download(string url, char* dst=NULL);