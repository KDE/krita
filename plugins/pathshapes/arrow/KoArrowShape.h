/* This file is part of the KDE project
 * Copyright (C) 2006 Laurent Montel <montel@kde.org>
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

#ifndef KOARROWSHAPE_H
#define KOARROWSHAPE_H

#include <KoParameterShape.h>

#define KoArrowShapeId "KoArrowShape"

class KoArrowShape : public KoPathShape
{
public:
    enum KoArrowType
    {
        ArrowLeft = 0,
        ArrowRight = 1,
        ArrowBottom = 2,
        ArrowTop = 3,
        ArrowLeftDown = 4,
        ArrowLeftTop = 5,
        ArrowRightDown = 6,
        ArrowRightTop = 7
    };

    KoArrowShape();
    void createPath(const QSizeF &size);
    void setType(KoArrowType _type);
protected:
    KoArrowType m_type;
};

#endif

