/* vim: set textwidth=80 tabstop=4 smarttab: */

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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "get-carddav-report.h"
#include <glib.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * A static literal string containing the webdav query for fetching
 * the names of all vcf files in the directory
 */
static const char* dirlist_request =
"<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
"<a:propfind xmlns:a=\"DAV:\">"
"  <a:prop><a:resourcetype/></a:prop>"
"</a:propfind>\r\n";

/**
 * A static literal string containing the card query for fetching
 * all cards from collection.
 */
static const char* getall_request_header =
"<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
"<C:addressbook-multiget xmlns:D=\"DAV:\""
"                 xmlns:C=\"urn:ietf:params:xml:ns:carddav\">"
" <D:prop>"
"   <D:getetag/>"
"   <C:address-data>"
"      <C:allprop/>"
"   </C:address-data>"
" </D:prop>";

static const char* getall_request_footer =
"</C:addressbook-multiget>\r\n";

#define ELEM_HREF "href"

/**
 * Function for listing a directory.
 * @param settings A pointer to carddav_settings. @see carddav_settings
 * @param error A pointer to carddav_error. @see carddav_error
 * @return TRUE in case of error, FALSE otherwise.
 */
static gchar* carddav_dirlist(carddav_settings* settings, carddav_error* error) {
	CURL* curl;
	CURLcode res = 0;
	char error_buf[CURL_ERROR_SIZE];
	struct config_data data;
	struct MemoryStruct chunk;
	struct MemoryStruct headers;
	struct curl_slist *http_header = NULL;
	gchar* all_href = NULL;

	chunk.memory = NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */
	headers.memory = NULL;
	headers.size = 0;

	curl = get_curl(settings);
	if (!curl) {
		error->code = -1;
		error->str = g_strdup("Could not initialize libcurl");
		g_free(settings->file);
		settings->file = NULL;
		return NULL;
	}

	http_header = curl_slist_append(http_header,
			"Content-Type: application/xml; charset=\"utf-8\"");
	http_header = curl_slist_append(http_header, "Depth: 1");
	http_header = curl_slist_append(http_header, "Expect:");
	http_header = curl_slist_append(http_header, "Transfer-Encoding:");
	http_header = curl_slist_append(http_header, "Connection: close");
	data.trace_ascii = settings->trace_ascii;
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteHeaderCallback);
	/* we pass our 'headers' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *)&headers);
	/* enable uploading */
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dirlist_request);
	curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, strlen(dirlist_request));
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, (char *) &error_buf);
	if (settings->debug) {
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
		curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &data);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	}
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PROPFIND");
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_UNRESTRICTED_AUTH, 1);
	curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
	res = curl_easy_perform(curl);
	if (res != 0) {
		error->code = -1;
		error->str = g_strdup_printf("%s", error_buf);
		g_free(settings->file);
		settings->file = NULL;
	}
	else {
		long code;
		res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
		if (code != 207) {
			error->code = code;
			error->str = g_strdup(headers.memory);
		}
		else {
			gchar* tmp_report = NULL;
			gchar* href = NULL;
			gchar* tmp = NULL;
			char* pos;
			tmp_report = g_strdup(chunk.memory);
			href = get_tag(ELEM_HREF, tmp_report);
			/* Maybe namespace prefixed */
			if (!href) {
				href = get_tag("D:href", tmp_report);
			}
			if (!href) {
				href = get_tag("d:href", tmp_report);
			}
			pos = strstr(tmp_report, href);
			all_href = g_strdup_printf("");
			while ((href != NULL) && (pos != NULL)) {
				href = get_tag(ELEM_HREF, pos);
				/* Maybe namespace prefixed */
				if (!href) {
					href = get_tag("D:href", pos);
				}
				if (!href) {
					href = get_tag("d:href", pos);
				}
				if (!href)
					break;
				pos = strstr(pos, href);
				tmp = g_strdup(all_href);
				g_free(all_href);
				all_href = g_strdup_printf("%s <D:href>%s</D:href>\r\n", tmp, href);
				g_free(tmp);
			}
			g_free(tmp_report);
		}
	}
	if (chunk.memory)
		free(chunk.memory);
	if (headers.memory)
		free(headers.memory);
	curl_slist_free_all(http_header);
	curl_easy_cleanup(curl);
	return all_href;
}

/**
 * Function for getting all cards from collection.
 * @param settings A pointer to carddav_settings. @see carddav_settings
 * @param error A pointer to carddav_error. @see carddav_error
 * @return TRUE in case of error, FALSE otherwise.
 */
gboolean carddav_getall(carddav_settings* settings, carddav_error* error) {
	gchar * dav_file_listing;
	dav_file_listing = carddav_dirlist(settings, error);
	if (dav_file_listing == NULL)
		return TRUE;

	CURL* curl;
	CURLcode res = 0;
	char error_buf[CURL_ERROR_SIZE];
	struct config_data data;
	struct MemoryStruct chunk;
	struct MemoryStruct headers;
	struct curl_slist *http_header = NULL;
	gboolean result = FALSE;

	chunk.memory = NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */
	headers.memory = NULL;
	headers.size = 0;

	curl = get_curl(settings);
	if (!curl) {
		error->code = -1;
		error->str = g_strdup("Could not initialize libcurl");
		g_free(settings->file);
		settings->file = NULL;
		return TRUE;
	}

	http_header = curl_slist_append(http_header,
			"Content-Type: application/xml; charset=\"utf-8\"");
	http_header = curl_slist_append(http_header, "Depth: 1");
	http_header = curl_slist_append(http_header, "Expect:");
	http_header = curl_slist_append(http_header, "Transfer-Encoding:");
	http_header = curl_slist_append(http_header, "Connection: close");
	data.trace_ascii = settings->trace_ascii;
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteHeaderCallback);
	/* we pass our 'headers' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *)&headers);
	/* assemble request */
	gchar * get_request;
	get_request = g_strdup_printf("%s%s%s\r\n", getall_request_header, dav_file_listing, getall_request_footer);
	/* enable uploading */
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, get_request);
	curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, strlen(get_request));
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, (char *) &error_buf);
	if (settings->debug) {
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
		curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &data);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	}
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "REPORT");
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_UNRESTRICTED_AUTH, 1);
	curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
	res = curl_easy_perform(curl);
	if (res != 0) {
		error->code = -1;
		error->str = g_strdup_printf("%s", error_buf);
		g_free(settings->file);
		settings->file = NULL;
		result = TRUE;
	}
	else {
		long code;
		res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
		if (code != 207) {
			error->code = code;
			error->str = g_strdup(headers.memory);
			result = TRUE;
		}
		else {
			gchar* report;
			report = parse_carddav_report(
						chunk.memory, "address-data", "VCARD");
			settings->file = g_strdup(report);
			g_free(report);
		}
	}
	if (chunk.memory)
		free(chunk.memory);
	if (headers.memory)
		free(headers.memory);
	curl_slist_free_all(http_header);
	curl_easy_cleanup(curl);
	g_free(dav_file_listing);
	return result;
}

/**
 * Function for getting all cards from collection.
 * This version stores each object's URI in a VCARD parameter.
 * @param settings A pointer to carddav_settings. @see carddav_settings
 * @param error A pointer to carddav_error. @see carddav_error
 * @return TRUE in case of error, FALSE otherwise.
 */
gboolean carddav_getall_by_uri(carddav_settings* settings, carddav_error* error) {
	gchar * dav_file_listing;
	dav_file_listing = carddav_dirlist(settings, error);
	if (dav_file_listing == NULL)
		return TRUE;

	CURL* curl;
	CURLcode res = 0;
	char error_buf[CURL_ERROR_SIZE];
	struct config_data data;
	struct MemoryStruct chunk;
	struct MemoryStruct headers;
	struct curl_slist *http_header = NULL;
	gboolean result = FALSE;

	chunk.memory = NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */
	headers.memory = NULL;
	headers.size = 0;

	curl = get_curl(settings);
	if (!curl) {
		error->code = -1;
		error->str = g_strdup("Could not initialize libcurl");
		g_free(settings->file);
		settings->file = NULL;
		return TRUE;
	}

	http_header = curl_slist_append(http_header,
			"Content-Type: application/xml; charset=\"utf-8\"");
	http_header = curl_slist_append(http_header, "Depth: 1");
	http_header = curl_slist_append(http_header, "Expect:");
	http_header = curl_slist_append(http_header, "Transfer-Encoding:");
	http_header = curl_slist_append(http_header, "Connection: close");
	data.trace_ascii = settings->trace_ascii;
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteHeaderCallback);
	/* we pass our 'headers' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *)&headers);
	/* assemble request */
	gchar * get_request;
	get_request = g_strdup_printf("%s%s%s\r\n", getall_request_header, dav_file_listing, getall_request_footer);
	/* enable uploading */
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, get_request);
	curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, strlen(get_request));
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, (char *) &error_buf);
	if (settings->debug) {
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
		curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &data);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	}
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "REPORT");
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_UNRESTRICTED_AUTH, 1);
	curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
	res = curl_easy_perform(curl);
	if (res != 0) {
		error->code = -1;
		error->str = g_strdup_printf("%s", error_buf);
		g_free(settings->file);
		settings->file = NULL;
		result = TRUE;
	}
	else {
		long code;
		res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
		if (code != 207) {
			error->code = code;
			error->str = g_strdup(headers.memory);
			result = TRUE;
		}
		else {
			gchar* report;
			report = parse_carddav_report(
						chunk.memory, "address-data", "VCARD");
			settings->file = g_strdup(report);
			g_free(report);
		}
	}
	if (chunk.memory)
		free(chunk.memory);
	if (headers.memory)
		free(headers.memory);
	curl_slist_free_all(http_header);
	curl_easy_cleanup(curl);
	g_free(dav_file_listing);
	return result;
}