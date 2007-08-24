/*
    This file is part of krita
    Copyright (c) 2005 KOffice Team

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

#ifndef KROSSKRITACORE_EXPORT_H
#define KROSSKRITACORE_EXPORT_H

#include <kdemacros.h>

/* We use _WIN32/_WIN64 instead of Q_OS_WIN so that this header can be used from C files too */
#if defined(_WIN32) || defined(_WIN64)

#ifndef KROSSKRITACORE_EXPORT
# ifdef MAKE_KROSSMODULEKRITA_LIB
#  define KROSSKRITACORE_EXPORT KDE_EXPORT
# else
#  define KROSSKRITACORE_EXPORT KDE_IMPORT
# endif
#endif

#else /* not windows */

#define KROSSKRITACORE_EXPORT KDE_EXPORT

#endif /* not windows */

#endif /* KROSSKRITACORE_EXPORT_H */
