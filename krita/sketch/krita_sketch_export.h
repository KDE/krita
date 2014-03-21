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

#ifndef KRITA_SKETCH_EXPORT_H
#define KRITA_SKETCH_EXPORT_H

#include <kdemacros.h>

#ifndef KRITA_SKETCH_EXPORT
# ifdef MAKE_KRITA_SKETCH_LIB
#  define KRITA_SKETCH_EXPORT KDE_EXPORT
# else
#  define KRITA_SKETCH_EXPORT KDE_IMPORT
# endif
#endif

#endif /* KRITA_SKETCH_EXPORT_H */
