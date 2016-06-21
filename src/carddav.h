/* vim: set textwidth=80 tabstop=4: */

/**
 * @file carddav.h
 * @brief interface to the carddav library.
 * The library conforms to IETF carddav draft specification daboo-10. For further information follow this
 * link http://tools.ietf.org/html/draft-ietf-vcarddav-carddav-10
 */

/** 
 * @mainpage
 * This document is the documentation for the public interface to libcarddav.
 * If you want to study the implementation look for the developers API.
 *
 * The libray and documentation is Copyright (c) 2010 Timothy Pearson
 * (kb9vqf@pearsoncomputing.net) and the original caldav implementation is
 * copyright (c) 2008 Michael Rasmussen (mir@datanom.net)
 *
 * License for the source code.
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
 *
 * License for the documentation.
 *
 * Permission is granted to copy, distribute and/or modify this document
 * under the terms of the GNU Free Documentation License, Version 1.2
 * or any later version published by the Free Software Foundation;
 * with no Invariant Sections, no Front-Cover Texts, and no Back-Cover
 * Texts.
 */

#ifndef __CARDDAV_H__
#define __CARDDAV_H__

#include <time.h>

/* For debug purposes */
/**
 * @typedef struct debug_curl
 * A struct used to set internal options in the library
 */
typedef struct {
  int		trace_ascii; /** @var int trace_ascii
					 	  * 0 or 1
					 	  */
  int		debug;       /** @var int debug
					 	  * 0 or 1
					 	  */
  int		verify_ssl_certificate;
  int		use_locking;
  char*		custom_cacert; 
} debug_curl;

/**
 * @typedef struct _carddav_error carddav_error
 * Pointer to a carddav_error structure
 */
typedef struct _carddav_error carddav_error;

/**
 * @struct _carddav_error
 * A struct for storing error codes and messages
 */
struct _carddav_error {
	long code; /**
				* @var long code
				* if < 0 internal error > 0 CardDAV protocol error.
				*/
	char* str; /** @var char* str
				* For storing human readable error message
				*/
};

/**
 * @typedef struct runtime_info
 * Pointer to a runtime structure holding debug and error information
 */
typedef struct {
    carddav_error*   error;
    debug_curl*	    options;
} runtime_info;

/* CardDAV is defined in RFC4791 */

/* Buffer to hold response */
/**
 * @typedef struct _response response
 * Pointer to a _response structure
 */
typedef struct _response response;

/**
 * @struct _response
 * A struct used for returning messages from the library to users
 */
struct _response {
	char* msg; /** @var char* msg
				* String for storing response
				*/
};

/**
 * @enum CARDDAV_ACTION specifies supported CardDAV actions.
 * UNKNOWN. An unknown action.
 * ADD. Add a CardDAV calendar object.
 * DELETE. Delete a CardDAV calendar object.
 * MODIFY. Modify a CardDAV calendar object.
 * GET. Get one or more CardDAV calendar object(s).
 * GETALL. Get all CardDAV calendar objects.
 */
typedef enum {
	UNKNOWN,
	ADD,
	DELETE,
	FREEBUSY,
	MODIFY,
	GET,
	GETALL,
	GETCALNAME,
	ISCARDDAV,
	OPTIONS
} CARDDAV_ACTION;

/**
 * @enum CARDDAV_RESPONSE specifies CardDAV error states.
 * OK (HTTP 200). Request was satisfied.
 * FORBIDDEN (HTTP 403). Access not allowed. Dont repeat request.
 * CONFLICT (HTTP 409). Conflict between current state of CardDAV collection
 * and request. Client must solve the conflict and then resend request.
 * LOCKED (HTTP 423). Locking failed.
 */
typedef enum {
	OK,
	FORBIDDEN,
	CONFLICT,
	LOCKED,
	NOTIMPLEMENTED
} CARDDAV_RESPONSE;


#ifndef __CARDDAV_USERAGENT
#define __CARDDAV_USERAGENT "libcurl-agent/0.1"
#endif


/**
 * Function for adding a new card.
 * @param object Appointment following ICal format (RFC2445). Receiver is
 * responsible for freeing the memory.
 * @param URL Defines CardDAV resource. Receiver is responsible for freeing
 * the memory. [http://][username[:password]@]host[:port]/url-path.
 * See (RFC1738).
 * @param info Pointer to a runtime_info structure. @see runtime_info
 * @return Ok, FORBIDDEN, or CONFLICT. @see CARDDAV_RESPONSE
 */
CARDDAV_RESPONSE carddav_add_object(const char* object,
				  const char* URL,
				  runtime_info* info);

/**
 * Function for deleting a card.
 * @param object Appointment following ICal format (RFC2445). Receiver is
 * responsible for freeing the memory.
 * @param URL Defines CardDAV resource. Receiver is responsible for freeing
 * the memory. [http://][username[:password]@]host[:port]/url-path.
 * See (RFC1738).
 * @param info Pointer to a runtime_info structure. @see runtime_info
 * @return Ok, FORBIDDEN, or CONFLICT. @see CARDDAV_RESPONSE
 */
CARDDAV_RESPONSE carddav_delete_object(const char* object,
				     const char* URL,
				     runtime_info* info);

/**
 * Function for deleting a card by URI.
 * @param object Appointment following ICal format (RFC2445). Receiver is
 * responsible for freeing the memory.
 * @param URL Defines CardDAV resource. Receiver is responsible for freeing
 * the memory. [http://][username[:password]@]host[:port]/url-path.
 * See (RFC1738).
 * @param info Pointer to a runtime_info structure. @see runtime_info
 * @return Ok, FORBIDDEN, or CONFLICT. @see CARDDAV_RESPONSE
 */
CARDDAV_RESPONSE carddav_delete_object_by_uri(const char* object,
				     const char* URL,
				     runtime_info* info);

/**
 * Function for modifying a card.
 * @param object Appointment following ICal format (RFC2445). Receiver is
 * responsible for freeing the memory.
 * @param URL Defines CardDAV resource. Receiver is responsible for freeing
 * the memory. [http://][username[:password]@]host[:port]/url-path.
 * See (RFC1738).
 * @param info Pointer to a runtime_info structure. @see runtime_info
 * @return Ok, FORBIDDEN, or CONFLICT. @see CARDDAV_RESPONSE
 */
CARDDAV_RESPONSE carddav_modify_object(const char* object,
				     const char* URL,
				     runtime_info* info);

/**
 * Function for modifying a card by URI.
 * @param object Appointment following ICal format (RFC2445). Receiver is
 * responsible for freeing the memory.
 * @param URL Defines CardDAV resource. Receiver is responsible for freeing
 * the memory. [http://][username[:password]@]host[:port]/url-path.
 * See (RFC1738).
 * @param info Pointer to a runtime_info structure. @see runtime_info
 * @return Ok, FORBIDDEN, or CONFLICT. @see CARDDAV_RESPONSE
 */
CARDDAV_RESPONSE carddav_modify_object_by_uri(const char* object,
				     const char* URL,
				     runtime_info* info);

/**
 * Function for getting a collection of cards determined by time range.
 * @param result A pointer to struct _response where the result is to stored.
 * @see response. Caller is responsible for freeing the memory.
 * @param start time_t variable specifying start for range. Included in search.
 * @param end time_t variable specifying end for range. Included in search.
 * @param URL Defines CardDAV resource. Receiver is responsible for freeing
 * the memory. [http://][username[:password]@]host[:port]/url-path.
 * See (RFC1738).
 * @param info Pointer to a runtime_info structure. @see runtime_info
 * @return Ok, FORBIDDEN, or CONFLICT. @see CARDDAV_RESPONSE
 */
CARDDAV_RESPONSE carddav_get_object(response* result,
				  time_t start,
				  time_t end,
				  const char* URL,
				  runtime_info* info);

/**
 * Function for getting all cards from the collection.
 * @param result A pointer to struct _response where the result is to stored.
 * @see response. Caller is responsible for freeing the memory.
 * @param URL Defines CardDAV resource. Receiver is responsible for freeing
 * the memory. [http://][username[:password]@]host[:port]/url-path.
 * See (RFC1738).
 * @param info Pointer to a runtime_info structure. @see runtime_info
 * @return Ok, FORBIDDEN, or CONFLICT. @see CARDDAV_RESPONSE
 */
CARDDAV_RESPONSE carddav_getall_object(response* result,
				     const char* URL,
				     runtime_info* info);

/**
 * Function for getting all cards from the collection.
 * This version stores the URI as a VCARD parameter.
 * @param result A pointer to struct _response where the result is to stored.
 * @see response. Caller is responsible for freeing the memory.
 * @param URL Defines CardDAV resource. Receiver is responsible for freeing
 * the memory. [http://][username[:password]@]host[:port]/url-path.
 * See (RFC1738).
 * @param info Pointer to a runtime_info structure. @see runtime_info
 * @return Ok, FORBIDDEN, or CONFLICT. @see CARDDAV_RESPONSE
 */
CARDDAV_RESPONSE carddav_getall_object_by_uri(response* result,
				     const char* URL,
				     runtime_info* info);

/**
 * Function for getting the stored display name for the collection.
 * @param result A pointer to struct _response where the result is to stored.
 * @see response. Caller is responsible for freeing the memory.
 * @param URL Defines CardDAV resource. Receiver is responsible for freeing
 * the memory. [http://][username[:password]@]host[:port]/url-path.
 * See (RFC1738).
 * @param info Pointer to a runtime_info structure. @see runtime_info
 * @return Ok, FORBIDDEN, or CONFLICT. @see CARDDAV_RESPONSE
 */
CARDDAV_RESPONSE carddav_get_displayname(response* result,
				       const char* URL,
				       runtime_info* info);

/**
 * Function to test wether a calendar resource is CardDAV enabled or not.
 * @param URL Defines CardDAV resource. Receiver is responsible for
 * freeing the memory. [http://][username[:password]@]host[:port]/url-path.
 * See (RFC1738).
 * @param info Pointer to a runtime_info structure. @see runtime_info
 * @result 0 (zero) means no CardDAV support, otherwise CardDAV support
 * detechted.
 */
int carddav_enabled_resource(const char* URL, runtime_info* info);

/**
 * Function for getting free/busy information.
 * @param result A pointer to struct _response where the result is to stored.
 * @see response. Caller is responsible for freeing the memory.
 * @param start time_t variable specifying start and end for range. Both
 * are included in range.
 * @param end time_t variable specifying start and end for range. Both
 * are included in range.
 * @param URL Defines CardDAV resource. Receiver is responsible for freeing
 * the memory. [http://][username[:password]@]host[:port]/url-path.
 * See (RFC1738).
 * @return Ok, FORBIDDEN, or CONFLICT. @see CARDDAV_RESPONSE
 */
CARDDAV_RESPONSE carddav_get_freebusy(response *result,
				  					time_t start,
				  					time_t end,
				  					const char* URL,
				  					runtime_info* info);

/** 
 * @deprecated Always returns an initialized empty carddav_error
 * Function to call in case of errors.
 * Caller provides a pointer to a local carddav_error structure.
 * Carddav_get_error will initialize pointer if NULL.
 * Caller is responsible for freeing returned memory.
 * After the first call the internal error buffer is reset.
 * @param lib_error A pointer to a struct _carddav_error. @see _carddav_error
 * @return An initialized carddav_error pointer to memory where error
 * messages can be found from the last call to the library.
 */
carddav_error* carddav_get_error(carddav_error* lib_error);

/** 
 * Function for freeing memory for a previous initialization of a
 * carddav_error. @see carddav_get_error()
 * Caller provides a pointer to a local carddav_error structure.
 * @param lib_error A pointer to a struct _carddav_error. @see _carddav_error
 */
void carddav_free_error(carddav_error* lib_error);

/* Setting various options in library */

/**
 * @deprecated Does nothing
 * Function which supports sending various options inside the library.
 * @param curl_options A struct debug_curl. See debug_curl.
 */
void carddav_set_options(debug_curl curl_options);

/**
 * Function to call to get a list of supported CardDAV options for a server
 * @param URL Defines CardDAV resource. Receiver is responsible for
 * freeing the memory. [http://][username[:password]@]host[:port]/url-path.
 * See (RFC1738).
 * @param info Pointer to a runtime_info structure. @see runtime_info
 * @result A list of available options or NULL in case of any error.
 */
char** carddav_get_server_options(const char* URL, runtime_info* info);

/**
 * Function for getting an initialized runtime_info structure
 * @return runtime_info. @see runtime_info
 */
runtime_info* carddav_get_runtime_info();

/**
 * Function for freeing memory for a previous initialization of an info
 * structure
 * @param info Address to a pointer to a runtime_info structure. @see 
 * runtime_info
 */
void carddav_free_runtime_info(runtime_info** info);

/**
 * Function for getting an initialized response structure
 * @return response. @see _response
 */
response* carddav_get_response();

/**
 * Function for freeing memory for a previous initialization of an response
 * structure
 * @param info Address to a pointer to a response structure. @see 
 * _response
 */
void carddav_free_response(response** info);

#endif
