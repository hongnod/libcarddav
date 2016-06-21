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

#ifndef __LOCK_CARDDAV_OBJECT_H__
#define __LOCK_CARDDAV_OBJECT_H__

#include "carddav-utils.h"
#include "carddav.h"

/**
 * Function which requests a lock on an addressbook resource
 * @param URI The resource to request lock on.
 * @param settings @see carddav_settings
 * @param error A pointer to carddav_error. @see carddav_error
 * @return The Lock-Token or NULL in case of error
 */
gchar* carddav_lock_object(
		gchar* URI, carddav_settings* settings, carddav_error* error);

/**
 * Function which requests to have a lock removed from a addressbook resource
 * @param lock_token A previous acquired Lock-Token
 * @param URI The resource to request unlock on.
 * @param settings @see carddav_settings
 * @param error A pointer to carddav_error. @see carddav_error
 * @return False in case the lock could not be removed. True otherwise.
 */
gboolean carddav_unlock_object(gchar* lock_token, gchar* URI, 
		carddav_settings* settings, carddav_error* error);

/**
 * Function to test whether the server supports locking or not
 * @param settings @see carddav_settings
 * @param error A pointer to carddav_error. @see carddav_error
 * @return True if locking is supported by the server. False otherwise
 */
gboolean carddav_lock_support(carddav_settings* settings, carddav_error* error);

#endif
