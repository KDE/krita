/*
    This file is part of krita
    Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KRITAANIMATIONDOCKER_EXPORT_H
#define KRITAANIMATIONDOCKER_EXPORT_H

#include <kdemacros.h>

#  if defined _WIN32 || defined _WIN64
#    if defined(MAKE_KRITAANIMATIONDOCKER_LIB)
#      define KRITAANIMATIONDOCKER_EXPORT KDE_EXPORT
#    else
#       define KRITAANIMATIONDOCKER_EXPORT KDE_IMPORT
#    endif
#  else /* not windows */
#    define KRITAANIMATIONDOCKER_EXPORT KDE_EXPORT
#  endif


#endif /* KRITAANIMATIONDOCKER_EXPORT_H */
