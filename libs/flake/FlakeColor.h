/* This file is part of the KDE project
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef FLAKE_COLOR_H
#define FLAKE_COLOR_H

// XXX: or do the #ifdef everywhere, as in Qt?
#ifdef NO_PIGMENT
#include <QColor>
class KoColor : public QColor {
public:
    QColor toQColor() { return *this; }
    QColor toQColor() const { return *this; }

};
Q_DECLARE_METATYPE(KoColor)
#else
#   include <KoColor.h>
#endif

#endif
