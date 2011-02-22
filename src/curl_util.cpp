/*
 *  curl_util.cpp
 *  haarspider
 *
 *  Created by Jeffrey Crouse on 2/22/11.
 *  Copyright 2011 Eyebeam. All rights reserved.
 *
 */

#include "curl_util.h"

const char* user_agent_header = "User-Agent: Mozilla/5.0";


// -------------------------------------------------------------
int writeData(char *data, size_t size, size_t nmemb, std::string *buffer)
{
	int result = 0;
	if (buffer != NULL) {
		buffer->append(data, size * nmemb);
		result = size * nmemb;
	}
	return result;
}

// -------------------------------------------------------------
// TO DO:  Maybe I only have to make the CURL object once.
string download(string url, char* dest)
{
	FILE* pFile;
	string contents;
	if(dest!=NULL) {
		pFile = fopen(dest, "w");
	}
	
	static char errorBuffer[CURL_ERROR_SIZE];
	CURL *curl = curl_easy_init();
	CURLcode result;
	if (!curl) {
		throw "Couldn't create CURL object.";
	}
	
	// Set the headers
	struct curl_slist *headers=NULL;
	headers = curl_slist_append(headers, user_agent_header);
	
	
	// TO DO:
	// put the whole thing in a try block
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 2000);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	if(dest!=NULL) {
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pFile) ;
	} else {
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &contents);
	}
	result = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	
	
	// Did we succeed?
	if (result != CURLE_OK)
	{
		cerr << "Bad result from CURL: " << errorBuffer << endl;
		return "";						
	}
	
	long http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	char status_msg[255];
	sprintf(status_msg, "HTTP status code: %ld", http_code);
	cerr << status_msg << endl;
	if (http_code != 200 || result == CURLE_ABORTED_BY_CALLBACK)
	{
		cerr << "HTTP error" << endl;
		return "";
	}
	
	if(dest!=NULL) {
		fclose (pFile);	
	}
	
	return contents;
}