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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "modify-carddav-object.h"
#include "lock-carddav-object.h"
#include <glib.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * A static literal string containing the first part of the calendar query.
 * The actual UID to use for the query is added at runtime.
 */
static char* search_head =
"<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
"<C:addressbook-query xmlns:D=\"DAV:\""
"                 xmlns:C=\"urn:ietf:params:xml:ns:carddav\">"
" <D:prop>"
"   <D:getetag/>"
"   <C:address-data>"
"      <C:allprop/>"
"   </C:address-data>"
" </D:prop>"
" <C:filter test=\"anyof\">"
"    <C:prop-filter name=\"UID\">";

/**
 * A static literal string containing the last part of the calendar query
 */
static char* search_tail =
"    </C:prop-filter>"
" </C:filter>"
"</C:addressbook-query>\r\n";

/**
 * Function for modifying a card.
 * @param settings A pointer to carddav_settings. @see carddav_settings
 * @param error A pointer to carddav_error. @see carddav_error
 * @return TRUE in case of error, FALSE otherwise.
 */
gboolean carddav_modify(carddav_settings* settings, carddav_error* error) {
	CURL* curl;
	CURLcode res = 0;
	char error_buf[CURL_ERROR_SIZE];
	struct config_data data;
	struct MemoryStruct chunk;
	struct MemoryStruct headers;
	struct curl_slist *http_header = NULL;
	gchar* search;
	gchar* uid;
	gboolean result = FALSE;
	gboolean LOCKSUPPORT = FALSE;
	gchar* lock_token = NULL;

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
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION,	WriteHeaderCallback);
	/* we pass our 'headers' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *)&headers);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, (char *) &error_buf);
	if (settings->debug) {
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
		curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &data);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	}
	gchar* file = g_strdup(settings->file);
	if ((uid = get_response_header("uid", file, FALSE)) == NULL) {
		g_free(file);
		error->code = 1;
		error->str = g_strdup("Error: Missing required UID for object");
		return TRUE;
	}
	g_free(file);
	/*
	 * collation is not supported by ICalendar.
	 * <C:text-match collation=\"i;ascii-casemap\">%s</C:text-match>
	 */
	search = g_strdup_printf(
		"%s<C:text-match collation=\"i;unicode-casemap\" negate-condition=\"no\" match-type=\"exact\">%s</C:text-match>%s",
		search_head, uid, search_tail);
	g_free(uid);
	/* enable uploading */
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, search);
	curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, strlen(search));
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "REPORT");
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_UNRESTRICTED_AUTH, 1);
	curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
	res = curl_easy_perform(curl);
	curl_slist_free_all(http_header);
	http_header = NULL;
	g_free(search);
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
			error->str = g_strdup(chunk.memory);
			result = TRUE;
		}
		else {
			/* enable uploading */
			gchar* url = NULL;
			gchar* etag = NULL;
			url = get_url(chunk.memory);
			if (url) {
				etag = get_etag(chunk.memory);
				if (etag) {
					gchar* host = get_host(settings->url);
					if (host) {
						file = g_strdup(url);
						g_free(url);
						url = g_strdup_printf("%s%s", host, file);
						g_free(file);
						g_free(host);
					}
					else {
						g_free(etag);
						g_free(url);
						url = NULL;
					}
				}
				else {
					g_free(url);
					url = NULL;
				}
				if (url) {
					int lock = 0;
					carddav_error lock_error;

					file = g_strdup(etag);
					g_free(etag);
					etag = g_strdup_printf("If-Match: %s", file);
					g_free(file);
					http_header = curl_slist_append(http_header, etag);
					g_free(etag);
					http_header = curl_slist_append(http_header,
						"Content-Type: text/directory; charset=\"utf-8\"");
					http_header = curl_slist_append(http_header, "Connection: close");
					http_header = curl_slist_append(http_header, "Expect:");
					http_header = curl_slist_append(
									http_header, "Transfer-Encoding:");
					if (settings->use_locking)
						LOCKSUPPORT = carddav_lock_support(settings, &lock_error);
					else
						LOCKSUPPORT = FALSE;
					if (LOCKSUPPORT) {
						lock_token = carddav_lock_object(url, settings, &lock_error);
						if (lock_token) {
							http_header = curl_slist_append(
								http_header, g_strdup_printf(
										"If: (%s)", lock_token));
						}
						/*
						 * If error code is 423 (Resource is LOCKED) bail out
						 */
						else if (lock_error.code == 423) {
							lock = -1;
						}
						/*
						 * If error code is 501 (Not implemented) we continue
						 * hoping for the best.
						 */
						else if (lock_error.code == 501) {
							lock_token = g_strdup("");
						}
						else {
							lock = -1;
						}
					}
					if (! LOCKSUPPORT || (LOCKSUPPORT && lock_token && lock_error.code != 423)) {
						curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);
						curl_easy_setopt(curl, CURLOPT_URL, rebuild_url(settings, url));
						curl_easy_setopt(curl, CURLOPT_POSTFIELDS, settings->file);
						curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE,
									strlen(settings->file));
						curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
						curl_easy_setopt(curl, CURLOPT_UNRESTRICTED_AUTH, 1);
						curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
						curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
						res = curl_easy_perform(curl);
						if (LOCKSUPPORT && lock_token) {
							carddav_unlock_object(
									lock_token, url, settings, &lock_error);
						}
					}
					g_free(url);
					g_free(lock_token);
					if (res != 0 || lock < 0) {
						/* Is this a lock_error don't change error*/
						if (lock == 0 || lock_error.code == 423) {
							error->code = code;
							error->str = g_strdup(chunk.memory);
						}
						else {
							error->code = lock_error.code;
							error->str = g_strdup(lock_error.str);
						}
						result = TRUE;
						g_free(settings->file);
						settings->file = NULL;
					}
					else {
						long code;
						res = curl_easy_getinfo(
									curl, CURLINFO_RESPONSE_CODE, &code);
						if (code != 204) {
							error->code = code;
							error->str = g_strdup(chunk.memory);
							result = TRUE;
						}
					}
					curl_slist_free_all(http_header);
				}
				else {
					error->code = code;
					if (chunk.memory)
						error->str = g_strdup(chunk.memory);
					else
						error->str = g_strdup("No object found");
					result = TRUE;
				}
			}
			else {
				/*
				 * No object found on server. Posible synchronization
				 * problem or a server side race condition
				 */
				error->code = 409;
				error->str = g_strdup("No object found");
				result = TRUE;
			}
		}
	}
	if (chunk.memory)
		free(chunk.memory);
	if (headers.memory)
		free(headers.memory);
	curl_easy_cleanup(curl);
	return result;
}

/**
 * Function for modifying a card by URI.
 * @param settings A pointer to carddav_settings. @see carddav_settings
 * @param error A pointer to carddav_error. @see carddav_error
 * @return TRUE in case of error, FALSE otherwise.
 */
gboolean carddav_modify_by_uri(carddav_settings* settings, carddav_error* error) {
	CURL* curl;
	CURLcode res = 0;
	char error_buf[CURL_ERROR_SIZE];
	struct config_data data;
	struct MemoryStruct chunk;
	struct MemoryStruct headers;
	struct curl_slist *http_header = NULL;
	gchar* search;
	gchar* uid;
	gboolean result = FALSE;
	gboolean LOCKSUPPORT = FALSE;
	gchar* lock_token = NULL;

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
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION,	WriteHeaderCallback);
	/* we pass our 'headers' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *)&headers);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, (char *) &error_buf);
	if (settings->debug) {
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
		curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &data);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	}
	gchar* file = g_strdup(settings->file);
	if ((uid = get_response_header("uri", file, FALSE)) == NULL) {
		g_free(file);
		error->code = 1;
		error->str = g_strdup("Error: Missing required URI for object\nThe requested contact may not exist on the server");
		return TRUE;
	}
	g_free(file);

	/* enable uploading */
	long code;
	gchar* url = NULL;
	gchar* etag = NULL;
	url = uid;
	if (url) {
		gchar* host = get_host(settings->url);
		if (host) {
			file = g_strdup(url);
			g_free(url);
			url = g_strdup_printf("%s%s", host, file);
			g_free(file);
			g_free(host);
		}
		else {
			g_free(url);
			url = NULL;
		}
		if (url) {
			int lock = 0;
			carddav_error lock_error;

			file = g_strdup(etag);
			g_free(etag);
			etag = g_strdup_printf("If-Match: %s", file);
			g_free(file);
			http_header = curl_slist_append(http_header, etag);
			g_free(etag);
			http_header = curl_slist_append(http_header,
				"Content-Type: text/directory; charset=\"utf-8\"");
			http_header = curl_slist_append(http_header, "Expect:");
			http_header = curl_slist_append(
							http_header, "Transfer-Encoding:");
			http_header = curl_slist_append(http_header, "Connection: close");
			if (settings->use_locking)
				LOCKSUPPORT = carddav_lock_support(settings, &lock_error);
			else
				LOCKSUPPORT = FALSE;
			if (LOCKSUPPORT) {
				lock_token = carddav_lock_object(url, settings, &lock_error);
				if (lock_token) {
					http_header = curl_slist_append(
						http_header, g_strdup_printf(
								"If: (%s)", lock_token));
				}
				/*
					* If error code is 423 (Resource is LOCKED) bail out
					*/
				else if (lock_error.code == 423) {
					lock = -1;
				}
				/*
					* If error code is 501 (Not implemented) we continue
					* hoping for the best.
					*/
				else if (lock_error.code == 501) {
					lock_token = g_strdup("");
				}
				else {
					lock = -1;
				}
			}
			if (! LOCKSUPPORT || (LOCKSUPPORT && lock_token && lock_error.code != 423)) {
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);
				curl_easy_setopt(curl, CURLOPT_URL, rebuild_url(settings, url));
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, settings->file);
				curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE,
							strlen(settings->file));
				curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
				curl_easy_setopt(curl, CURLOPT_UNRESTRICTED_AUTH, 1);
				curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
				res = curl_easy_perform(curl);
				if (LOCKSUPPORT && lock_token) {
					carddav_unlock_object(
							lock_token, url, settings, &lock_error);
				}
			}
			g_free(url);
			g_free(lock_token);
			if (res != 0 || lock < 0) {
				/* Is this a lock_error don't change error*/
				if (lock == 0 || lock_error.code == 423) {
					error->code = code;
					error->str = g_strdup(chunk.memory);
				}
				else {
					error->code = lock_error.code;
					error->str = g_strdup(lock_error.str);
				}
				result = TRUE;
				g_free(settings->file);
				settings->file = NULL;
			}
			else {
				long code;
				res = curl_easy_getinfo(
							curl, CURLINFO_RESPONSE_CODE, &code);
				if (code != 204) {
					error->code = code;
					error->str = g_strdup(chunk.memory);
					result = TRUE;
				}
			}
			curl_slist_free_all(http_header);
		}
		else {
			error->code = code;
			if (chunk.memory)
				error->str = g_strdup(chunk.memory);
			else
				error->str = g_strdup("No object found");
			result = TRUE;
		}
	}
	else {
		/*
			* No object found on server. Posible synchronization
			* problem or a server side race condition
			*/
		error->code = 409;
		error->str = g_strdup("No object found");
		result = TRUE;
	}
	if (chunk.memory)
		free(chunk.memory);
	if (headers.memory)
		free(headers.memory);
	curl_easy_cleanup(curl);
	return result;
}
