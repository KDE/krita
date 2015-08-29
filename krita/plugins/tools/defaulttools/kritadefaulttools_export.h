/*
    This file is part of krita
    Copyright (c) 2015 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef KRITADEFAULTTOOLS_EXPORT_H
#define KRITADEFAULTTOOLS_EXPORT_H

#include <kdemacros.h>

#ifdef COMPILING_TESTS
#  if defined _WIN32 || defined _WIN64
#    if defined(kritadefaulttools_EXPORTS)
#      define KRITADEFAULTTOOLS_TEST_EXPORT KDE_EXPORT
#    else
#       define KRITADEFAULTTOOLS_TEST_EXPORT KDE_IMPORT
#    endif
#  else /* not windows */
#    define KRITADEFAULTTOOLS_TEST_EXPORT KDE_EXPORT
#  endif
#else /* not compiling tests */
#  define KRITADEFAULTTOOLS_TEST_EXPORT
#endif

#endif /* KRITADEFAULTTOOLS_EXPORT_H */
