/*
    This file is part of krita
    Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
    Copyright (c) 2008 Thomas Zander <zander@kde.org>

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

#ifndef KRITAIMAGE_EXPORT_H
#define KRITAIMAGE_EXPORT_H

#include <kdemacros.h>

/* We use _WIN32/_WIN64 instead of Q_OS_WIN so that this header can be used from C files too */
#if defined(_WIN32) || defined(_WIN64)

#ifndef KRITAIMAGE_EXPORT
# ifdef kritaimage_EXPORTS
#  define KRITAIMAGE_EXPORT KDE_EXPORT
# else
#  define KRITAIMAGE_EXPORT KDE_IMPORT
# endif
#endif

#else // not windows

#define KRITAIMAGE_EXPORT KDE_EXPORT

#endif /* not windows */

/* Now the same for Krita*_TEST_EXPORT, if compiling with unit tests enabled */
#ifdef COMPILING_TESTS
#  if defined _WIN32 || defined _WIN64
#    if defined(kritaimage_EXPORTS)
#      define KRITAIMAGE_TEST_EXPORT KDE_EXPORT
#    else
#       define KRITAIMAGE_TEST_EXPORT KDE_IMPORT
#    endif
#  else /* not windows */
#    define KRITAIMAGE_TEST_EXPORT KDE_EXPORT
#  endif
#else /* not compiling tests */
#  define KRITAIMAGE_TEST_EXPORT
#endif

#endif /* KRITAIMAGE_EXPORT_H */
