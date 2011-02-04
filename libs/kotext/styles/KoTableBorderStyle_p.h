/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008,2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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
#ifndef KOTABLEBORDERSTYLE_P_H
#define KOTABLEBORDERSTYLE_P_H

#include "KoTableBorderStyle.h"

class KoTableBorderStylePrivate
{
public:
    struct Edge {
        Edge() : innerPen(), outerPen(), spacing(0.0) { }

        QPen innerPen;
        QPen outerPen;
        qreal spacing;
    };

    KoTableBorderStylePrivate();
    virtual ~KoTableBorderStylePrivate();

    Edge edges[6];
    KoTableBorderStyle::BorderStyle borderstyle[6];
};

#endif // KOTABLEBORDERSTYLE_P_H
