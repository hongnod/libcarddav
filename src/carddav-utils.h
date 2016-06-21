/* vim: set textwidth=80 tabstop=4: */

/* Copyright (c) 2010 Timothy Pearson (kb9vqf@pearsoncomputing.net)
 * Copyright (c) 2008 Michael Rasmussen (mir@datanom.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __CARDDAV_UTILS_H__
#define __CARDDAV_UTILS_H__

#include <glib.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "carddav.h"

/**
 * @typedef struct _CARDDAV_SETTINGS carddav_settings
 * A pointer to a struct _CARDDAV_SETTINGS
 */
typedef struct _CARDDAV_SETTINGS carddav_settings;

/**
 * @struct _CARDDAV_SETTINGS
 * A struct used to exchange all user input between various parts
 * of the library
 */
struct _CARDDAV_SETTINGS {
	gchar* username;
	gchar* password;
	gchar* url;
	gchar* file;
	gboolean usehttps;
	gboolean verify_ssl_certificate;
	gchar* custom_cacert;
	gboolean debug;
	gboolean use_locking;
	char trace_ascii;
	CARDDAV_ACTION ACTION;
	time_t start;
	time_t end;
	char use_uri;
};

/**
 * @typedef struct MemoryStruct memory_ptr
 * A pointer to a struct MemoryStruct
 */
typedef struct MemoryStruct memory_ptr;

/**
 * @struct MemoryStruct
 * Used to hold messages between the CardDAV server and the library
 */
struct MemoryStruct {
	char *memory;
	size_t size;
};

/** @struct config_data
 * Used to exchange user options to the library
 */
struct config_data {
	char trace_ascii;
};

/**
 * This function is burrowed from the libcurl documentation
 * @param text
 * @param stream
 * @param ptr
 * @param size
 * @param nohex
 */
void dump(const char* text, FILE* stream, char* ptr, size_t size, char nohex);

/**
 * This function is burrowed from the libcurl documentation
 * @param handle
 * @param type
 * @param data
 * @param size
 * @param userp
 * @return
 */
int my_trace(CURL* handle, curl_infotype type, char* data, size_t size, void* userp);

/**
 * This function is burrowed from the libcurl documentation
 * @param ptr
 * @param size
 * @return void* to memory region
 */
size_t WriteMemoryCallback(void* ptr, size_t size, size_t nmemb, void* data);

/**
 * This function is burrowed from the libcurl documentation
 * @param ptr
 * @param size
 * @param nmemb
 * @param data
 * @return number of written bytes
 */
size_t WriteHeaderCallback(void* ptr, size_t size, size_t nmemb, void* data);

/*size_t ReadMemoryCallback(void* ptr, size_t size, size_t nmemb, void* data);*/

/**
 * Initialize carddav settings structure.
 * @param settings @see carddav_settings
 */
void init_carddav_settings(carddav_settings* settings);

/**
 * Free momory assigned to carddav settings structure.
 * @param settings @see carddav_settings
 */
void free_carddav_settings(carddav_settings* settings);

/**
 * Parse URL
 * @param settings @see carddav_settings
 * @param url String containing URL to collection
 */
void parse_url(carddav_settings* settings, const char* url);

/**
 * Find a specific HTTP header from last request
 * @param header HTTP header to search for
 * @param headers String of HTTP headers from last request
 * @param lowcase Should string be returned in all lower case.
 * @return The header found or NULL
 */
gchar* get_response_header(
		const char* header, gchar* headers, gboolean lowcase);

/**
 * Parse response from CardDAV server
 * @param report Response from server
 * @param element XML element to find
 * @param type VCalendar element to find
 * @return the parsed result
 */
gchar* parse_carddav_report(char* report, const char* element, const char* type);

/**
 * Convert a time_t variable to CardDAV DateTime
 * @param time a specific date and time
 * @return the CardDAV DateTime
 */
gchar* get_carddav_datetime(time_t* time);

/**
 * Create a random text string, using MD5. @see carddav_md5_hex_digest()
 * @param text some text to randomize
 * @return MD5 hash of text
 */
gchar* random_file_name(gchar* text);

/**
 * Does the card contain a UID element or not. If not add it.
 * @param object A specific card
 * @return card, eventually added UID
 */
gchar* verify_uid(gchar* object);

/**
 * Fetch a URL from a XML element
 * @param text String
 * @return URL
 */
gchar* get_url(gchar* text);

/**
 * Fetch host from URL
 * @param url URL
 * @return host
 */
gchar* get_host(gchar* url);

/**
 * Fetch the etag element from XML
 * @param text String
 * @return etag
 */
gchar* get_etag(gchar* text);

/**
 * Fetch any element from XML
 * @param text String
 * @return element
 */
gchar* get_tag(const gchar* tag, gchar* text);



/**
 * rebuild a raw URL with https if needed from the settings
 * @param settings carddav_settings
 * @return URL
 */
gchar* rebuild_url(carddav_settings* setting, gchar* uri);

/**
 * Prepare a curl connection
 * @param settings carddav_settings
 * @return CURL
 */
CURL* get_curl(carddav_settings* setting);

#endif
