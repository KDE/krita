/*
    visibility management for some symbols

    Copyright 2008 Brad Hards <bradh@frogmouth.net>
    Copyright 2009 Inge Wallin <inge@lysator.liu.se>

    This library is free software; you can redistribute it and/or modify it under
    the terms of the GNU Library General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option) any
    later version.

    This library is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.  See the GNU Library General Public License for more
    details.

    You should have received a copy of the GNU Library General Public License along
    with this library; see the file COPYING.LIB.  If not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef VECTORIMAGE_EXPORT_H
#define VECTORIMAGE_EXPORT_H

#include <kdemacros.h>

/* We use _WIN32/_WIN64 instead of Q_OS_WIN so that this header can be used from C files too */
#if defined _WIN32 || defined _WIN64

# ifndef VECTORIMAGE_EXPORT
#  if defined(KDELIBS_STATIC_LIBS)
   /* No export/import for static libraries */
#   define VECTORIMAGE_EXPORT
#  elif defined(MAKE_VECTORIMAGE_LIB)
#   define VECTORIMAGE_EXPORT KDE_EXPORT
#  else
#   define VECTORIMAGE_EXPORT KDE_IMPORT
#  endif
# endif
#else /* UNIX */

#define VECTORIMAGE_EXPORT KDE_EXPORT

#endif

#endif
