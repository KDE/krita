/*
    This file is part of krita
    Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>

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

#ifndef KRITA_EXPORT_H
#define KRITA_EXPORT_H

#include <kdemacros.h>

/* We use _WIN32/_WIN64 instead of Q_OS_WIN so that this header can be used from C files too */
#if defined(_WIN32) || defined(_WIN64)

#ifndef KRITAUI_EXPORT
# ifdef MAKE_KRITAUI_LIB
#  define KRITAUI_EXPORT KDE_EXPORT
# else
#  define KRITAUI_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KRITAIMAGE_EXPORT
# ifdef MAKE_KRITAIMAGE_LIB
#  define KRITAIMAGE_EXPORT KDE_EXPORT
# else
#  define KRITAIMAGE_EXPORT KDE_IMPORT
# endif
#endif

#else // not windows

#define KRITAUI_EXPORT KDE_EXPORT
#define KRITAIMAGE_EXPORT KDE_EXPORT

#endif /* not windows */

#endif /* KRITA_EXPORT_H */
