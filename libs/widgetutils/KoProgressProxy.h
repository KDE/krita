/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#ifndef KO_PROGRESS_PROXY
#define KO_PROGRESS_PROXY

#include "kowidgetutils_export.h"

class QString;

/**
 * A proxy interface for a real progress status reporting thing, either
 * a widget such as a KoProgressProxy childclass that also inherits this
 * interface, or something that prints progress to stdout.
 */
class KOWIDGETUTILS_EXPORT KoProgressProxy
{

public:

    virtual ~KoProgressProxy() { }

    virtual int maximum() const = 0;
    virtual void setValue(int value) = 0;
    virtual void setRange(int minimum, int maximum) = 0;
    virtual void setFormat(const QString &format) = 0;
};


#endif
