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



#ifndef KRITA_EXPORT_H
#define KRITA_EXPORT_H

#include <kdemacros.h>

#ifdef Q_WS_WIN

/* TODO #ifdef MAKE_KRITAUI_LIB etc.  */

#define KRITA_EXPORT KDE_EXPORT
#define KRITAUI_EXPORT KDE_EXPORT
#define KRITACOLOR_EXPORT KDE_EXPORT
#define KRITATOOL_EXPORT KDE_EXPORT
#define KRITAPAINT_EXPORT KDE_EXPORT
#define KRITAIMAGE_EXPORT KDE_EXPORT
#define KRITASCRIPTING_EXPORT KDE_EXPORT

#else // not windows

#define KRITA_EXPORT KDE_EXPORT
#define KRITACOLOR_EXPORT KDE_EXPORT
#define KRITAUI_EXPORT KDE_EXPORT
#define KRITATOOL_EXPORT KDE_EXPORT
#define KRITAPAINT_EXPORT KDE_EXPORT
#define KRITAIMAGE_EXPORT KDE_EXPORT
#define KRITASCRIPTING_EXPORT KDE_EXPORT

#endif /* not windows */

#endif /* KRITA_EXPORT_H */
