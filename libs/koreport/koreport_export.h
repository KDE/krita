/*
 * Kexi Report Plugin
 * Copyright (C) 2009-2010 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KOREPORT_EXPORT_H
#define KOREPORT_EXPORT_H

// needed for KDE_EXPORT and KDE_IMPORT macros
#include <kdemacros.h>

#ifndef KOREPORT_EXPORT
# if defined(MAKE_KOREPORT_LIB)
// We are building this library
#  define KOREPORT_EXPORT KDE_EXPORT
# else
// We are using this library
#  define KOREPORT_EXPORT KDE_IMPORT
# endif
#endif

# ifndef KOREPORT_EXPORT_DEPRECATED
#  define KOREORT_EXPORT_DEPRECATED KDE_DEPRECATED KOREPORT_EXPORT
# endif

#endif

