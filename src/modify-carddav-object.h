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

#ifndef __MODIFY_CARDDAV_OBJECT_H__
#define __MODIFY_CARDDAV_OBJECT_H__

#include "carddav-utils.h"
#include "carddav.h"

/**
 * Function for modifying a card.
 * @param settings A pointer to carddav_settings. @see carddav_settings
 * @param error A pointer to carddav_error. @see carddav_error
 * @return TRUE in case of error, FALSE otherwise.
 */
gboolean carddav_modify(carddav_settings* settings, carddav_error* error);

/**
 * Function for modifying a card by URI.
 * @param settings A pointer to carddav_settings. @see carddav_settings
 * @param error A pointer to carddav_error. @see carddav_error
 * @return TRUE in case of error, FALSE otherwise.
 */
gboolean carddav_modify_by_uri(carddav_settings* settings, carddav_error* error);

#endif
